/** \file
 * Defines basic primitive types used within the IR.
 */

#pragma once

#include "ql/utils/num.h"
#include "ql/utils/str.h"
#include "ql/utils/vec.h"
#include "ql/utils/exception.h"
#include "ql/utils/tree.h"

namespace ql {
namespace ir {
namespace prim {

/**
 * Generates a default value for the given primitive type. This is specialized
 * for the primitives mapping to builtin types (int, bool, etc, for which the
 * "constructor" doesn't initialize the value at all) such that they actually
 * initialize with a sane default. Used in the default constructors of the
 * generated tree nodes to ensure that there's no garbage in the nodes.
 */
template <class T>
T initialize() { return T(); };

/**
 * Serializes the given primitive object to CBOR.
 */
template <typename T>
void serialize(const T &obj, utils::tree::cbor::MapWriter &map);

/**
 * Deserializes the given primitive object from CBOR.
 */
template <typename T>
T deserialize(const utils::tree::cbor::MapReader &map);

/**
 * String primitive used within the trees.
 */
using Str = utils::Str;
template <>
Str initialize<Str>();
template <>
void serialize(const Str &obj, utils::tree::cbor::MapWriter &map);
template <>
Str deserialize(const utils::tree::cbor::MapReader &map);

/**
 * Boolean primitive used within the trees. Defaults to false.
 */
using Bool = utils::Bool;
template <>
Bool initialize<Bool>();
template <>
void serialize(const Bool &obj, utils::tree::cbor::MapWriter &map);
template <>
Bool deserialize(const utils::tree::cbor::MapReader &map);

/**
 * Integer primitive used within the trees. Defaults to 0.
 */
using Int = utils::Int;
template <>
Int initialize<Int>();
template <>
void serialize(const Int &obj, utils::tree::cbor::MapWriter &map);
template <>
Int deserialize(const utils::tree::cbor::MapReader &map);

/**
 * Real number primitive used within the trees. Defaults to 0.0.
 */
using Real = utils::Real;
template <>
Real initialize<Real>();
template <>
void serialize(const Real &obj, utils::tree::cbor::MapWriter &map);
template <>
Real deserialize(const utils::tree::cbor::MapReader &map);

/**
 * Complex number primitive used within the trees. Defaults to 0.0.
 */
using Complex = utils::Complex;

/**
 * Two-dimensional matrix of some kind of type.
 */
template <typename T>
class Matrix {
private:

    /**
     * The contained data, stored row-major.
     */
    utils::Vec<T> data;

    /**
     * The number of rows in the matrix.
     */
    utils::UInt nrows;

    /**
     * The number of columns in the matrix.
     */
    utils::UInt ncols;

public:

    /**
     * Creates an empty matrix.
     */
    Matrix()
        : data(ncols), nrows(1), ncols(0)
    {}

    /**
     * Creates a vector.
     */
    Matrix(utils::UInt ncols)
        : data(ncols), nrows(1), ncols(ncols)
    {}

    /**
     * Creates a zero-initialized matrix of the given size.
     */
    Matrix(utils::UInt nrows, utils::UInt ncols)
        : data(nrows*ncols), nrows(nrows), ncols(ncols)
    {}

    /**
     * Creates a column vector with the given data.
     */
    Matrix(const utils::Vec<T> &data)
        : data(data), nrows(data.size()), ncols(1)
    {}

    /**
     * Creates a matrix with the given data. The number of rows is inferred. If
     * the number of data elements is not divisible by the number of columns, a
     * range error is thrown.
     */
    Matrix(const utils::Vec<T> &data, utils::UInt ncols)
        : data(data), nrows(data.size() / ncols), ncols(ncols)
    {
        if (data.size() % ncols != 0) {
            throw utils::Exception("invalid matrix shape");
        }
    }

    /**
     * Returns the number of rows.
     */
    utils::UInt size_rows() const {
        return nrows;
    }

    /**
     * Returns the number of columns.
     */
    utils::UInt size_cols() const {
        return ncols;
    }

    /**
     * Returns access to the raw data vector.
     */
    const utils::Vec<T> &get_data() const {
        return data;
    }

    /**
     * Returns the value at the given position. row and col start at 1. Throws
     * an Exception when either or both indices are out of range.
     */
    T at(utils::UInt row, utils::UInt col) const {
        if (row < 1 || row > nrows || col < 1 || col > ncols) {
            throw utils::Exception("matrix index out of range");
        }
        return data[(row - 1) * ncols + col - 1];
    }

    /**
     * Returns a mutable reference to the value at the given position. row and
     * col start at 1. Throws an Exception when either or both indices are out
     * of range.
     */
    T &at(utils::UInt row, utils::UInt col) {
        if (row < 1 || row > nrows || col < 1 || col > ncols) {
            throw utils::Exception("matrix index out of range");
        }
        return data[(row - 1) * ncols + col - 1];
    }

    /**
     * Equality operator for matrices.
     */
    bool operator==(const Matrix<T> &rhs) const {
        return data == rhs.data && nrows == rhs.nrows && ncols == rhs.ncols;
    }

    /**
     * Inequality operator for matrices.
     */
    bool operator!=(const Matrix<T> &rhs) const {
        return !(*this == rhs);
    }

};

/**
 * Matrix of real numbers.
 */
using RMatrix = Matrix<Real>;
template <>
void serialize(const RMatrix &obj, utils::tree::cbor::MapWriter &map);
template <>
RMatrix deserialize(const utils::tree::cbor::MapReader &map);

/**
 * Matrix of complex numbers.
 */
using CMatrix = Matrix<Complex>;
template <>
void serialize(const CMatrix &obj, utils::tree::cbor::MapWriter &map);
template <>
CMatrix deserialize(const utils::tree::cbor::MapReader &map);

/**
 * Stream << overload for matrix nodes.
 */
template <typename T>
std::ostream &operator<<(std::ostream &os, const Matrix<T> &mat) {
    os << "[";
    for (utils::UInt row = 1; row <= mat.size_rows(); row++) {
        if (row > 1) {
            os << "; ";
        }
        for (size_t col = 1; col <= mat.size_cols(); col++) {
            if (col > 1) {
                os << ", ";
            }
            os << mat.at(row, col);
        }
    }
    os << "]";
    return os;
}

} // namespace prim
} // namespace ir
} // namespace ql
