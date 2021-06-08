/** \file
 * Unitary matrix (decomposition) implementation.
 */

#include "ql/com/unitary.h"

#include "ql/utils/exception.h"
#include "ql/utils/logger.h"

#ifndef WITHOUT_UNITARY_DECOMPOSITION
#include <Eigen/MatrixFunctions>
#include <complex.h>
#define lapack_complex_float    std::complex<float>
#define lapack_complex_double   std::complex<double>
#include <src/misc/lapacke.h>
#endif

#include <chrono>

namespace ql {
namespace com {

using namespace utils;

/**
 * Creates a unitary gate with the given name and row-major unitary matrix.
 */
Unitary::Unitary(
    const Str &name,
    const Vec<Complex> &array
) :
    decomposed(false),
    name(name),
    array(array)
{
}

/**
 * Returns the number of elements in the incoming matrix.
 */
UInt Unitary::size() const {
    return array.size();
}

#ifdef WITHOUT_UNITARY_DECOMPOSITION

/**
 * Explicitly runs the matrix decomposition algorithm. Used to be required,
 * nowadays is called implicitly by get_decomposition() if not done explicitly.
 */
void Unitary::decompose() {
    throw Exception("unitary decomposition was explicitly disabled in this build!");
}

/**
 * Returns whether unitary decomposition support was enabled in this build
 * of OpenQL.
 */
Bool Unitary::is_decompose_support_enabled() {
    return false;
}

#else

// JvS: this was originally the class "unitary" itself, but compile times of
// Eigen are so excessive that I moved it into its own compile unit and
// provided a wrapper instead. It doesn't actually NEED to be wrapped like
// this, because the Eigen::Matrix<...> member is actually only used within
// the scope of a single method (calling other methods), but I'm not touching
// this code.
class UnitaryDecomposer {
private:
    Eigen::Matrix<Complex, Eigen::Dynamic, Eigen::Dynamic> _matrix;

public:
    Str name;
    Vec<Complex> array;
    Vec<Complex> SU;
    Real delta;
    Real alpha;
    Real beta;
    Real gamma;
    Bool decomposed;
    Vec<Real> instruction_list;

    typedef Eigen::Matrix<Complex, Eigen::Dynamic, Eigen::Dynamic> complex_matrix;

    UnitaryDecomposer() : name(""), decomposed(false) {}

    UnitaryDecomposer(
        const Str &name,
        const Vec<Complex> &array
    ) :
        name(name),
        array(array),
        decomposed(false)
    {
        QL_DOUT("constructing unitary: " << name
                  << ", containing: " << array.size() << " elements");
    }

    Real size() const {
        if (!array.empty()) {
            return (Real) array.size();
        } else {
            return (Real) _matrix.size();
        }
    }

    complex_matrix getMatrix() {
        if (!array.empty()) {
            Int matrix_size = (Int)sqrt(array.size());

            Eigen::Map<complex_matrix> matrix(array.data(), matrix_size, matrix_size);
            _matrix = matrix.transpose();
        }
        return _matrix;
    }

    void decompose() {
        QL_DOUT("decomposing Unitary: " << name);

        getMatrix();
        Int matrix_size = _matrix.rows();

        // compute the number of qubits: length of array is collumns*rows, so log2(sqrt(array.size))
        Int numberofbits = uint64_log2(matrix_size);

        Eigen::MatrixXcd identity = Eigen::MatrixXcd::Identity(matrix_size, matrix_size);
        Eigen::MatrixXcd matmatadjoint = (_matrix.adjoint()*_matrix);
        // very little accuracy because of tests using printed-from-matlab code that does not have many digits after the comma
        if (!matmatadjoint.isApprox(identity, 0.001)) {
            //Throw an error
            QL_EOUT("Unitary " << name <<" is not a unitary matrix!");

            throw utils::Exception("Error: Unitary '"+ name+"' is not a unitary matrix. Cannot be decomposed!" + to_string(matmatadjoint), false);
        }
        // initialize the general M^k lookuptable
        genMk();

        decomp_function(_matrix, numberofbits); //needed because the matrix is read in columnmajor

        QL_DOUT("Done decomposing");
        decomposed = true;
    }

    Str to_string(
        const complex_matrix &m,
        const Str &vector_prefix = "",
        const Str &elem_sep = ", "
    ) {
        StrStrm ss;
        ss << m << "\n";
        return ss.str();
    }

    // std::chrono::duration<Real> CSD_time;
    // std::chrono::duration<Real> CSD_time2;
    // std::chrono::duration<Real> CSD_time3;
    // std::chrono::duration<Real> zyz_time;
    // std::chrono::duration<Real> multiplexing_time;
    // std::chrono::duration<Real> demultiplexing_time;

    void decomp_function(const Eigen::Ref<const complex_matrix>& matrix, Int numberofbits) {
        QL_DOUT("decomp_function: \n" << to_string(matrix));
        if(numberofbits == 1) {
            zyz_decomp(matrix);
        } else {
            Int n = matrix.rows()/2;

            complex_matrix V(n,n);
            complex_matrix W(n,n);
            Eigen::VectorXcd D(n);
            // if q2 is zero, the whole thing is a demultiplexing problem instead of full CSD
            if (matrix.bottomLeftCorner(n,n).isZero(10e-14) && matrix.topRightCorner(n,n).isZero(10e-14)) {
                QL_DOUT("Optimization: q2 is zero, only demultiplexing will be performed.");
                instruction_list.push_back(200.0);
                if (matrix.topLeftCorner(n, n).isApprox(matrix.bottomRightCorner(n,n),10e-4)) {
                    QL_DOUT("Optimization: Unitaries are equal, skip one step in the recursion for unitaries of size: " << n << " They are both: " << matrix.topLeftCorner(n, n));
                    instruction_list.push_back(300.0);
                    decomp_function(matrix.topLeftCorner(n, n), numberofbits-1);
                } else {
                    demultiplexing(matrix.topLeftCorner(n, n), matrix.bottomRightCorner(n,n), V, D, W, numberofbits-1);

                    decomp_function(W, numberofbits-1);
                    multicontrolledZ(D, D.rows());
                    decomp_function(V, numberofbits-1);
                }
            } else if (
                // Check to see if it the kronecker product of a bigger matrix and the identity matrix.
                // By checking if the first row is equal to the second row one over, and if thelast two rows are equal
                // Which means the last qubit is not affected by this gate
                matrix(Eigen::seqN(0, n, 2), Eigen::seqN(1, n, 2)).isZero()
                && matrix(Eigen::seqN(1, n, 2), Eigen::seqN(0, n, 2)).isZero()
                && matrix.block(0,0,1,2*n-1) == matrix.block(1,1,1,2*n-1)
                && matrix.block(2*n-2,0,1,2*n-1) == matrix.block(2*n-1,1,1,2*n-1)
            ) {
                QL_DOUT("Optimization: last qubit is not affected, skip one step in the recursion.");
                // Code for last qubit not affected
                instruction_list.push_back(100.0);
                decomp_function(matrix(Eigen::seqN(0, n, 2), Eigen::seqN(0, n, 2)), numberofbits-1);
            } else {
                complex_matrix ss(n,n);
                complex_matrix L0(n,n);
                complex_matrix L1(n,n);
                complex_matrix R0(n,n);
                complex_matrix R1(n,n);
                // auto start = std::chrono::steady_clock::now();
                CSD(matrix, L0, L1, R0, R1, ss);
                // CSD_time += (std::chrono::steady_clock::now() - start);
                demultiplexing(R0, R1, V, D, W, numberofbits-1);
                decomp_function(W, numberofbits-1);
                multicontrolledZ(D, D.rows());
                decomp_function(V, numberofbits-1);

                multicontrolledY(ss.diagonal(), n);

                demultiplexing(L0, L1, V, D, W, numberofbits-1);
                decomp_function(W, numberofbits-1);
                multicontrolledZ(D, D.rows());
                decomp_function(V, numberofbits-1);
            }
        }
    }

    void CSD(
        const Eigen::Ref<const complex_matrix> &U,
        Eigen::Ref<complex_matrix> u1,
        Eigen::Ref<complex_matrix> u2,
        Eigen::Ref<complex_matrix> v1,
        Eigen::Ref<complex_matrix> v2,
        Eigen::Ref<complex_matrix> s
    ) {
        // auto start = std::chrono::steady_clock::now();
        //Cosine sine decomposition
        // U = [q1, U01] = [u1    ][c  s][v1  ]
        //     [q2, U11] = [    u2][-s c][   v2]
        Int n = U.rows();
        // complex_matrix c(n,n); // c matrix is not needed for the higher level
        // complex_matrix q1 = U.topLeftCorner(n/2,m/2);

        Eigen::BDCSVD<complex_matrix> svd(n/2,n/2);
        svd.compute(U.topLeftCorner(n/2,n/2), Eigen::ComputeThinU | Eigen::ComputeThinV); // possible because it's square anyway


        // thinCSD: q1 = u1*c*v1.adjoint()
        //          q2 = u2*s*v1.adjoint()
        Int p = n/2;
        // complex_matrix z = Eigen::MatrixXd::Identity(p, p).colwise().reverse();
        complex_matrix c(svd.singularValues().reverse().asDiagonal());
        u1.noalias() = svd.matrixU().rowwise().reverse();
        v1.noalias() = svd.matrixV().rowwise().reverse(); // Same v as in matlab: u*s*v.adjoint() = q1

        complex_matrix q2 = U.bottomLeftCorner(p,p)*v1;

        Int k = 0;
        for (Int j = 1; j < p; j++) {
            if (c(j,j).real() <= 0.70710678119) {
                k = j;
            }
        }
        //complex_matrix b = q2.block( 0,0, p, k+1);

        Eigen::HouseholderQR<complex_matrix> qr(p,k+1);
        qr.compute(q2.block( 0,0, p, k+1));
        u2 = qr.householderQ();
        s.noalias() = u2.adjoint()*q2;
        if (k < p-1) {
            QL_DOUT("k is smaller than size of q1 = "<< p << ", adjustments will be made, k = " << k);
            k = k+1;
            Eigen::BDCSVD<complex_matrix> svd2(p-k, p-k);
            svd2.compute(s.block(k, k, p-k, p-k), Eigen::ComputeThinU | Eigen::ComputeThinV);
            s.block(k, k, p-k, p-k) = svd2.singularValues().asDiagonal();
            c.block(0,k, p,p-k) = c.block(0,k, p,p-k)*svd2.matrixV();
            u2.block(0,k, p,p-k) = u2.block(0,k, p,p-k)*svd2.matrixU();
            v1.block(0,k, p,p-k) = v1.block(0,k, p,p-k)*svd2.matrixV();

            Eigen::HouseholderQR<complex_matrix> qr2(p-k, p-k);

            qr2.compute(c.block(k,k, p-k,p-k));
            c.block(k,k,p-k,p-k) = qr2.matrixQR().triangularView<Eigen::Upper>();
            u1.block(0,k, p,p-k) = u1.block(0,k, p,p-k)*qr2.householderQ();
        }
        // CSD_time2 += (std::chrono::steady_clock::now() - start);

        // auto start2 = std::chrono::steady_clock::now();



        Vec<Int> c_ind;
        Vec<Int> s_ind;
        for (Int j = 0; j < p; j++) {
            if (c(j,j).real() < 0) {
                c_ind.push_back(j);
            }
            if (s(j,j).real() < 0) {
                s_ind.push_back(j);
            }
        }

        c(c_ind,c_ind) = -c(c_ind,c_ind);
        u1(Eigen::all, c_ind) = -u1(Eigen::all, c_ind);

        //s.diagonal()(s_ind) = -s.diagonal()(s_ind);
        s(s_ind,s_ind) = -s(s_ind,s_ind);
        u2(Eigen::all, s_ind) = -u2(Eigen::all, s_ind);

        if (!U.topLeftCorner(p,p).isApprox(u1*c*v1.adjoint(), 10e-8) || !U.bottomLeftCorner(p,p).isApprox(u2*s*v1.adjoint(), 10e-8)) {
            if (U.topLeftCorner(p,p).isApprox(u1*c*v1.adjoint(), 10e-8)) {
                QL_DOUT("q1 is correct");
            } else {
                QL_DOUT("q1 is not correct! (is not usually an issue");
                QL_DOUT("q1: \n" << U.topLeftCorner(p,p));
                QL_DOUT("reconstructed q1: \n" << u1*c*v1.adjoint());
            }
            if (U.bottomLeftCorner(p,p).isApprox(u2*s*v1.adjoint(), 10e-8)) {
                QL_DOUT("q2 is correct");
            } else {
                QL_DOUT("q2 is not correct! (is not usually an issue)");
                QL_DOUT("q2: " << U.bottomLeftCorner(p,p));
                QL_DOUT("reconstructed q2: " << u2*s*v1.adjoint());
            }
        }
        v1.adjointInPlace(); // Use this instead of = v1.adjoint (to avoid aliasing issues)
        s = -s;

        complex_matrix tmp_s = u1.adjoint()*U.topRightCorner(p,p);
        complex_matrix tmp_c = u2.adjoint()*U.bottomRightCorner(p,p);

        // Vec<Int> c_ind_row;
        // Vec<Int> s_ind_row;
        for (Int i = 0; i < p; i++) {
            if (abs(s(i,i)) > abs(c(i,i))) {
                // Vec<Int> s_ind_row;
                v2.row(i).noalias() = tmp_s.row(i)/s(i,i);
            } else {
                // c_ind_row.push_back(i);
                v2.row(i).noalias() = tmp_c.row(i)/c(i,i);
            }
        }


        // v2(s_ind_row, Eigen::all) = tmp_s(s_ind_row, Eigen::all).rowwise() / s(s_ind_row, s_ind_row).array();
        // v2(c_ind_row, Eigen::all) = tmp_c(c_ind_row, Eigen::all).rowwise() / c(c_ind_row, c_ind_row).array();
        // U = [q1, U01] = [u1    ][c  s][v1  ]
        //     [q2, U11] = [    u2][-s c][   v2]

        complex_matrix tmp(n,n);
        tmp.topLeftCorner(p,p) = u1*c*v1;
        tmp.bottomLeftCorner(p,p) = -u2*s*v1;
        tmp.topRightCorner(p,p) = u1*s*v2;
        tmp.bottomRightCorner(p,p) = u2*c*v2;
        // Just to see if it kinda matches
        if (!tmp.isApprox(U, 10e-2)) {
            throw utils::Exception("CSD of unitary '"+ name+"' is wrong! Failed at matrix: \n"+to_string(tmp) + "\nwhich should be: \n" + to_string(U), false);
        }
        // CSD_time3 += (std::chrono::steady_clock::now() - start2);

    }

    void zyz_decomp(const Eigen::Ref<const complex_matrix> &matrix) {
        // auto start = std::chrono::steady_clock::now();

        Complex det = matrix.determinant();// matrix(0,0)*matrix(1,1)-matrix(1,0)*matrix(0,1);

        Real delta = atan2(det.imag(), det.real())/matrix.rows();
        Complex A = exp(Complex(0,-1)*delta)*matrix(0,0);
        Complex B = exp(Complex(0,-1)*delta)*matrix(0,1); //to comply with the other y-gate definition

        Real sw = sqrt(pow((Real) B.imag(),2) + pow((Real) B.real(),2) + pow((Real) A.imag(),2));
        Real wx = 0;
        Real wy = 0;
        Real wz = 0;

        if (sw > 0) {
            wx = B.imag()/sw;
            wy = B.real()/sw;
            wz = A.imag()/sw;
        }

        Real t1 = atan2(A.imag(),A.real());
        Real t2 = atan2(B.imag(), B.real());
        alpha = t1+t2;
        gamma = t1-t2;
        beta = 2*atan2(sw*sqrt(pow((Real) wx,2)+pow((Real) wy,2)),sqrt(pow((Real) A.real(),2)+pow((wz*sw),2)));
        instruction_list.push_back(-gamma);
        instruction_list.push_back(-beta);
        instruction_list.push_back(-alpha);
        // zyz_time += (std::chrono::steady_clock::now() - start);
    }

    void demultiplexing(
        const Eigen::Ref<const complex_matrix> &U1,
        const Eigen::Ref<const complex_matrix> &U2,
        Eigen::Ref<complex_matrix> V,
        Eigen::Ref<Eigen::VectorXcd> D,
        Eigen::Ref<complex_matrix> W,
        Int numberofcontrolbits
    ) {
        // [U1 0 ]  = [V 0][D 0 ][W 0]
        // [0  U2]    [0 V][0 D*][0 W]
        // auto start = std::chrono::steady_clock::now();
        complex_matrix check = U1*U2.adjoint();
        // complex_matrix D;
        // complex_matrix V;
        // complex_matrix W;
        if (check == check.adjoint()) {
            QL_IOUT("Demultiplexing matrix is self-adjoint()");
            Eigen::SelfAdjointEigenSolver<Eigen::MatrixXcd> eigslv(check);
            D.noalias() = ((complex_matrix) eigslv.eigenvalues()).cwiseSqrt();
            V.noalias() = eigslv.eigenvectors();
            W.noalias() = D.asDiagonal()*V.adjoint()*U2;
        } else {
            if (numberofcontrolbits < 5) {//schur is faster for small matrices
                Eigen::ComplexSchur<complex_matrix> decomposition(check);
                D.noalias() = decomposition.matrixT().diagonal().cwiseSqrt();
                V.noalias() = decomposition.matrixU();
                W.noalias() = D.asDiagonal() * V.adjoint() * U2;
            } else {
                Eigen::ComplexEigenSolver<complex_matrix> decomposition(check);
                D.noalias() = decomposition.eigenvalues().cwiseSqrt();
                V.noalias() = decomposition.eigenvectors();
                W.noalias() = D.asDiagonal() * V.adjoint() * U2;
            }
        }

        // demultiplexing_time += (std::chrono::steady_clock::now() - start);
        if (!(V*V.adjoint()).isApprox(Eigen::MatrixXd::Identity(V.rows(), V.rows()), 10e-3)) {
            QL_DOUT("Eigenvalue decomposition incorrect: V is not unitary, adjustments will be made");
            Eigen::BDCSVD<complex_matrix> svd3(V.block(0,0,V.rows(),2), Eigen::ComputeFullU);
            V.block(0,0,V.rows(),2) = svd3.matrixU();
            svd3.compute(V(Eigen::all,Eigen::seq(Eigen::last-1,Eigen::last)), Eigen::ComputeFullU);
            V(Eigen::all,Eigen::seq(Eigen::last-1,Eigen::last)) = svd3.matrixU();
        }

        complex_matrix Dtemp = D.asDiagonal();
        if (!U1.isApprox(V*Dtemp*W, 10e-2) || !U2.isApprox(V*Dtemp.adjoint()*W, 10e-2)) {
            QL_EOUT("Demultiplexing not correct!");
            throw utils::Exception("Demultiplexing of unitary '"+ name+"' not correct! Failed at matrix U1: \n"+to_string(U1)+ "and matrix U2: \n" +to_string(U2) + "\nwhile they are: \n" + to_string(V*D.asDiagonal()*W) + "\nand \n" + to_string(V*D.conjugate().asDiagonal()*W), false);
        }

    }

    Vec<Eigen::MatrixXd> genMk_lookuptable;

    // returns M^k = (-1)^(b_(i-1)*g_(i-1)), where * is bitwise inner product, g = binary gray code, b = binary code.
    void genMk() {
        Int numberqubits = uint64_log2(_matrix.rows());
        for (Int n = 1; n <= numberqubits; n++) {
            Int size=1<<n;
            Eigen::MatrixXd Mk(size,size);
            for (Int i = 0; i < size; i++) {
                for (Int j = 0; j < size ;j++) {
                    Mk(i,j) = pow(-1, bitParity(i&(j^(j>>1))));
                }
            }
            genMk_lookuptable.push_back(Mk);
        }

        // return genMk_lookuptable[numberqubits-1];
    }

    // source: https://stackoverflow.com/questions/994593/how-to-do-an-integer-log2-in-c user Todd Lehman
    Int uint64_log2(uint64_t n) {
#define S(k) if (n >= (UINT64_C(1) << k)) { i += k; n >>= k; }
        Int i = -(n == 0); S(32); S(16); S(8); S(4); S(2); S(1); return i;
#undef S
    }

    Int bitParity(Int i) {
        if (i < 2 << 16) {
            i = (i >> 16) ^ i;
            i = (i >> 8) ^ i;
            i = (i >> 4) ^ i;
            i = (i >> 2) ^ i;
            i = (i >> 1) ^ i;
            return i % 2;
        } else {
            throw utils::Exception("Bit parity number too big!", false);
        }
    }

    void multicontrolledY(const Eigen::Ref<const Eigen::VectorXcd> &ss, Int halfthesizeofthematrix) {
        // auto start = std::chrono::steady_clock::now();
        Eigen::VectorXd temp =  2*Eigen::asin(ss.array()).real();
        Eigen::CompleteOrthogonalDecomposition<Eigen::MatrixXd> dec(genMk_lookuptable[uint64_log2(halfthesizeofthematrix)-1]);
        Eigen::VectorXd tr = dec.solve(temp);
        // Check is very approximate to account for low-precision input matrices
        if (!temp.isApprox(genMk_lookuptable[uint64_log2(halfthesizeofthematrix)-1]*tr, 10e-2)) {
            QL_EOUT("Multicontrolled Y not correct!");
            throw utils::Exception("Demultiplexing of unitary '"+ name+"' not correct! Failed at demultiplexing of matrix ss: \n"  + to_string(ss), false);
        }

        instruction_list.insert(instruction_list.end(), &tr[0], &tr[halfthesizeofthematrix]);
        // multiplexing_time += std::chrono::steady_clock::now() - start;
    }

    void multicontrolledZ(const Eigen::Ref<const Eigen::VectorXcd> &D, Int halfthesizeofthematrix) {
        // auto start = std::chrono::steady_clock::now();

        Eigen::VectorXd temp =  (Complex(0,-2)*Eigen::log(D.array())).real();
        Eigen::CompleteOrthogonalDecomposition<Eigen::MatrixXd> dec(genMk_lookuptable[uint64_log2(halfthesizeofthematrix)-1]);
        Eigen::VectorXd tr = dec.solve(temp);
        // Check is very approximate to account for low-precision input matrices
        if (!temp.isApprox(genMk_lookuptable[uint64_log2(halfthesizeofthematrix)-1]*tr, 10e-2)) {
            QL_EOUT("Multicontrolled Z not correct!");
            throw utils::Exception("Demultiplexing of unitary '"+ name+"' not correct! Failed at demultiplexing of matrix D: \n"+ to_string(D), false);
        }

        instruction_list.insert(instruction_list.end(), &tr[0], &tr[halfthesizeofthematrix]);
        // multiplexing_time += std::chrono::steady_clock::now() - start;

    }

    ~UnitaryDecomposer() {
        // destroy unitary
        QL_DOUT("destructing unitary: " << name);
    }
};

/**
 * Explicitly runs the matrix decomposition algorithm. Used to be required,
 * nowadays is called implicitly by get_circuit() if not done explicitly.
 */
void Unitary::decompose() {
    if (decomposed) {
        return;
    }
    UnitaryDecomposer decomposer(name, array);
    decomposer.decompose();
    //SU = decomposer.SU;
    //alpha = decomposer.alpha;
    //beta = decomposer.beta;
    //gamma = decomposer.gamma;
    decomposed = decomposer.decomposed;
    instruction_list = decomposer.instruction_list;
}

/**
 * Returns whether unitary decomposition support was enabled in this build
 * of OpenQL.
 */
Bool Unitary::is_decompose_support_enabled() {
    return true;
}

#endif


//controlled qubit is the first in the list.
static void multicontrolled_rz(
    ir::compat::GateRefs &c,
    const Vec<Real> &instruction_list,
    UInt start_index,
    UInt end_index,
    const Vec<UInt> &qubits
) {
    // DOUT("Adding a multicontrolled rz-gate at start index " << start_index << ", to " << to_string(qubits, "qubits: "));
    UInt idx;
    //The first one is always controlled from the last to the first qubit.
    c.emplace<ir::compat::gate_types::RZ>(qubits.back(), -instruction_list[start_index]);
    c.emplace<ir::compat::gate_types::CNot>(qubits[0], qubits.back());
    for (UInt i = 1; i < end_index - start_index; i++) {
        idx = log2(((i)^((i)>>1))^((i+1)^((i+1)>>1)));
        c.emplace<ir::compat::gate_types::RZ>(qubits.back(), -instruction_list[i + start_index]);
        c.emplace<ir::compat::gate_types::CNot>(qubits[idx], qubits.back());
    }
    // The last one is always controlled from the next qubit to the first qubit
    c.emplace<ir::compat::gate_types::RZ>(qubits.back(), -instruction_list[end_index]);
    c.emplace<ir::compat::gate_types::CNot>(qubits.end()[-2], qubits.back());
}

//controlled qubit is the first in the list.
static void multicontrolled_ry(
    ir::compat::GateRefs &c,
    const Vec<Real> &instruction_list,
    UInt start_index,
    UInt end_index,
    const Vec<UInt> &qubits
) {
    // DOUT("Adding a multicontrolled ry-gate at start index "<< start_index << ", to " << to_string(qubits, "qubits: "));
    UInt idx;

    //The first one is always controlled from the last to the first qubit.
    c.emplace<ir::compat::gate_types::RY>(qubits.back(), -instruction_list[start_index]);
    c.emplace<ir::compat::gate_types::CNot>(qubits[0], qubits.back());

    for (UInt i = 1; i < end_index - start_index; i++) {
        idx = log2(((i)^((i)>>1))^((i+1)^((i+1)>>1)));
        c.emplace<ir::compat::gate_types::RY>(qubits.back(), -instruction_list[i + start_index]);
        c.emplace<ir::compat::gate_types::CNot>(qubits[idx], qubits.back());
    }
    // Last one is controlled from the next qubit to the first one.
    c.emplace<ir::compat::gate_types::RY>(qubits.back(), -instruction_list[end_index]);
    c.emplace<ir::compat::gate_types::CNot>(qubits.end()[-2], qubits.back());
}

//recursive gate count function
//n is number of qubits
//i is the start point for the instructionlist
static Int recursiveRelationsForUnitaryDecomposition(
    ir::compat::GateRefs &c,
    const Vec<Real> &insns,
    const Vec<UInt> &qubits,
    UInt n,
    UInt i
) {
    // DOUT("Adding a new unitary starting at index: "<< i << ", to " << n << to_string(qubits, " qubits: "));
    if (n > 1) {
        // Need to be checked here because it changes the structure of the decomposition.
        // This checks whether the first qubit is affected, if not, it applies a unitary to the all qubits except the first one.
        UInt numberforcontrolledrotation = pow2(n - 1);                     //number of gates per rotation

        // code for last one not affected
        if (insns[i] == 100.0) {
            QL_DOUT("[kernel.h] Optimization: last qubit is not affected, skip one step in the recursion. New start_index: " << i + 1);
            Vec<UInt> subvector(qubits.begin() + 1, qubits.end());
            return recursiveRelationsForUnitaryDecomposition(c, insns, subvector, n - 1, i + 1) + 1; // for the number 10.0
        } else if (insns[i] == 200.0) {
            Vec<UInt> subvector(qubits.begin(), qubits.end() - 1);

            // This is a special case of only demultiplexing
            if (insns[i+1] == 300.0) {

                // Two numbers that aren't rotation gate angles
                UInt start_counter = i + 2;
                QL_DOUT("[kernel.h] Optimization: first qubit not affected, skip one step in the recursion. New start_index: " << start_counter);

                return recursiveRelationsForUnitaryDecomposition(c, insns, subvector, n - 1, start_counter) + 2; //for the numbers 20 and 30
            } else {
                UInt start_counter = i + 1;
                QL_DOUT("[kernel.h] Optimization: only demultiplexing will be performed. New start_index: " << start_counter);

                start_counter += recursiveRelationsForUnitaryDecomposition(c, insns, subvector, n - 1, start_counter);
                multicontrolled_rz(c, insns, start_counter, start_counter + numberforcontrolledrotation - 1, qubits);
                start_counter += numberforcontrolledrotation; //multicontrolled rotation always has the same number of gates
                start_counter += recursiveRelationsForUnitaryDecomposition(c, insns, subvector, n - 1, start_counter);
                return start_counter - i;
            }
        } else {
            // The new qubit vector that is passed to the recursive function
            Vec<UInt> subvector(qubits.begin(), qubits.end() - 1);
            UInt start_counter = i;
            start_counter += recursiveRelationsForUnitaryDecomposition(c, insns, subvector, n - 1, start_counter);
            multicontrolled_rz(c, insns, start_counter, start_counter + numberforcontrolledrotation - 1, qubits);
            start_counter += numberforcontrolledrotation;
            start_counter += recursiveRelationsForUnitaryDecomposition(c, insns, subvector, n - 1, start_counter);
            multicontrolled_ry(c, insns, start_counter, start_counter + numberforcontrolledrotation - 1, qubits);
            start_counter += numberforcontrolledrotation;
            start_counter += recursiveRelationsForUnitaryDecomposition(c, insns, subvector, n - 1, start_counter);
            multicontrolled_rz(c, insns, start_counter, start_counter + numberforcontrolledrotation - 1, qubits);
            start_counter += numberforcontrolledrotation;
            start_counter += recursiveRelationsForUnitaryDecomposition(c, insns, subvector, n - 1, start_counter);
            return start_counter -i; //it is just the total
        }
    } else { //n=1
        // DOUT("Adding the zyz decomposition gates at index: "<< i);
        // zyz gates happen on the only qubit in the list.
        c.emplace<ir::compat::gate_types::RZ>(qubits.back(), insns[i]);
        c.emplace<ir::compat::gate_types::RY>(qubits.back(), insns[i + 1]);
        c.emplace<ir::compat::gate_types::RZ>(qubits.back(), insns[i + 2]);
        // How many gates this took
        return 3;
    }
}

/**
 * Returns the decomposed circuit.
 */
ir::compat::GateRefs Unitary::get_decomposition(const utils::Vec<utils::UInt> &qubits) {

    // Decompose now if not done yet.
    if (!decomposed) {
        decompose();
    }

    UInt u_size = log2(size()) / 2;
    if (u_size != qubits.size()) {
        throw Exception(
            "Unitary '" + name + "' has been applied to the wrong number of qubits. " +
            "Cannot be added to kernel! " + to_string(qubits.size()) + " and not " +
            to_string(u_size)
        );
    }
    for (uint64_t i = 0; i < qubits.size()-1; i++) {
        for (uint64_t j = i + 1; j < qubits.size(); j++) {
            if (qubits[i] == qubits[j]) {
                throw Exception(
                    "Qubit numbers used more than once in Unitary: " + name + ". " +
                    "Double qubit is number " + to_string(qubits[j])
                );
            }
        }
    }
    // applying unitary to gates
    QL_DOUT("Applying unitary '" << name << "' to qubits: " << qubits);
    QL_DOUT("The list is this many items long: " << instruction_list.size());
    //COUT("Instructionlist" << to_string(u.instructionlist));
    ir::compat::GateRefs c;
    Int end_index = recursiveRelationsForUnitaryDecomposition(c, instruction_list, qubits, u_size, 0);
    QL_DOUT("Total number of gates added: " << end_index);

    return c;
}


} // namespace com
} // namespace ql
