version 1.0
# this file has been automatically generated by the OpenQL compiler please do not modify it manually.
qubits 4

.kernel1
    h q[2]
    wait 1
    h q[2]
    wait 1
    cnot q[1],q[2]
    wait 3
    tdag q[2]
    wait 1
    cnot q[0],q[2]
    wait 3
    t q[2]
    wait 1
    cnot q[1],q[2]
    wait 3
    { tdag q[2] | tdag q[1] }
    wait 1
    cnot q[0],q[2]
    wait 3
    { t q[2] | cnot q[0],q[1] }
    wait 1
    h q[2]
    wait 1
    { tdag q[1] | h q[2] }
    wait 1
    cnot q[0],q[1]
    wait 3
    { t q[0] | s q[1] }
    wait 1