version 1.0
# this file has been automatically generated by the OpenQL compiler please do not modify it manually.
qubits 7

.kernel_edge_ALAP
    cz q[0],q[3]
    wait 3
    { cz q[1],q[4] | cz q[2],q[5] }
    wait 3