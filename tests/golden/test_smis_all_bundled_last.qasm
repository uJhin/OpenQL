version 1.0
# this file has been automatically generated by the OpenQL compiler please do not modify it manually.
qubits 7

.aKernel
    { prepz q[0] | prepz q[1] | prepz q[2] | prepz q[3] | prepz q[4] | prepz q[5] | prepz q[6] }
    wait 1
    { x q[0] | x q[1] | x q[2] | x q[3] | x q[4] | x q[5] | x q[6] }
    wait 1