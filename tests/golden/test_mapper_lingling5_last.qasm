# Generated by OpenQL 0.10.0 for program test_mapper_lingling5
version 1.2

pragma @ql.name("test_mapper_lingling5")


.kernel_lingling5
    { # start at cycle 0
        prepz q[5]
        prepz q[6]
    }
    skip 1
    prepz q[2]
    skip 28
    x q[5]
    y q[5]
    cz q[2], q[5]
    skip 1
    { # start at cycle 35
        y90 q[5]
        ym90 q[2]
    }
    cz q[5], q[2]
    skip 1
    { # start at cycle 38
        y90 q[2]
        ym90 q[0]
    }
    cz q[2], q[0]
    skip 1
    { # start at cycle 41
        y90 q[6]
        x q[2]
    }
    { # start at cycle 42
        x q[6]
        y q[2]
    }
    cz q[6], q[2]
    skip 1
    cz q[6], q[2]
    skip 1
    cz q[5], q[2]
    y q[3]
    cz q[0], q[3]
    ym90 q[5]
    cz q[2], q[5]
    skip 1
    { # start at cycle 53
        y90 q[3]
        ym90 q[0]
    }
    cz q[3], q[0]
    { # start at cycle 55
        y90 q[5]
        ym90 q[2]
    }
    cz q[5], q[2]
    skip 1
    { # start at cycle 58
        y90 q[0]
        ym90 q[3]
    }
    cz q[0], q[3]
    { # start at cycle 60
        y90 q[2]
        ym90 q[5]
    }
    cz q[2], q[5]
    skip 1
    ym90 q[0]
    cz q[2], q[0]
    skip 1
    y90 q[5]
    cz q[1], q[5]
    { # start at cycle 68
        y90 q[0]
        ym90 q[2]
    }
    cz q[0], q[2]
    skip 1
    x q[5]
    { # start at cycle 72
        y90 q[2]
        y q[5]
    }
    cz q[5], q[2]
    skip 1
    y90 q[5]
    x q[5]
    { # start at cycle 77
        measure q[6]
        measure q[5]
    }
    skip 14
    { # start at cycle 92
        prepz q[6]
        prepz q[5]
    }
    skip 30
    ym90 q[0]
    cz q[2], q[0]
    skip 1
    x q[6]
    { # start at cycle 127
        y90 q[2]
        y q[6]
    }
    cz q[2], q[6]
    skip 1
    y90 q[5]
    { # start at cycle 131
        x q[5]
        ym90 q[1]
    }
    cz q[5], q[1]
    skip 1
    x q[5]
    y q[5]
    cz q[2], q[5]
    skip 1
    { # start at cycle 138
        y90 q[6]
        ym90 q[2]
    }
    cz q[6], q[2]
    skip 1
    { # start at cycle 141
        y90 q[2]
        ym90 q[6]
    }
    cz q[2], q[6]
    skip 1
    cz q[2], q[5]
    skip 1
    cz q[2], q[5]
    skip 1
    measure q[2]
    skip 14
    prepz q[2]
    skip 30
    y90 q[2]
    x q[2]
    cz q[2], q[0]
    skip 1
    { # start at cycle 198
        y90 q[0]
        ym90 q[2]
    }
    cz q[0], q[2]
    skip 1
    y90 q[2]
    cz q[2], q[5]
    skip 1
    x q[5]
    { # start at cycle 205
        y q[5]
        ym90 q[1]
    }
    cz q[5], q[1]
    skip 1
    { # start at cycle 208
        y90 q[1]
        ym90 q[5]
    }
    cz q[1], q[5]
    skip 1
    { # start at cycle 211
        y90 q[5]
        ym90 q[1]
    }
    cz q[5], q[1]
    skip 1
    { # start at cycle 214
        y90 q[1]
        ym90 q[4]
    }
    cz q[1], q[4]
    skip 1
    y90 q[1]
    x q[1]
    measure q[1]
    skip 14
    prepz q[1]
    skip 27
    ym90 q[0]
    cz q[2], q[0]
    skip 1
    y q[2]
    cz q[5], q[2]
    skip 1
    { # start at cycle 268
        y90 q[2]
        ym90 q[5]
    }
    cz q[2], q[5]
    skip 1
    { # start at cycle 271
        y90 q[5]
        ym90 q[2]
    }
    cz q[5], q[2]
    skip 1
    y90 q[2]
    cz q[2], q[0]
    skip 1
    { # start at cycle 277
        y90 q[0]
        ym90 q[2]
    }
    cz q[0], q[2]
    skip 1
    { # start at cycle 280
        y90 q[2]
        ym90 q[0]
    }
    { # start at cycle 281
        y90 q[1]
        cz q[2], q[0]
    }
    x q[1]
    cz q[1], q[5]
    skip 1
    { # start at cycle 285
        y90 q[5]
        ym90 q[2]
    }
    cz q[5], q[2]
    skip 1
    { # start at cycle 288
        y90 q[2]
        ym90 q[5]
    }
    cz q[2], q[5]
    skip 1
    { # start at cycle 291
        y90 q[5]
        ym90 q[2]
    }
    { # start at cycle 292
        x q[1]
        cz q[5], q[2]
    }
    y q[1]
    cz q[5], q[1]
    skip 1
    cz q[5], q[1]
    skip 1
    measure q[5]
    skip 14
    prepz q[5]
    skip 29
    y90 q[2]
    cz q[2], q[6]
    skip 1
    { # start at cycle 346
        y90 q[6]
        ym90 q[2]
    }
    cz q[6], q[2]
    skip 1
    { # start at cycle 349
        y90 q[2]
        ym90 q[6]
    }
    cz q[2], q[6]
    skip 1
    y90 q[0]
    { # start at cycle 353
        y90 q[5]
        cz q[0], q[3]
    }
    { # start at cycle 354
        x q[5]
        ym90 q[2]
    }
    cz q[5], q[2]
    skip 1
    { # start at cycle 357
        y90 q[3]
        ym90 q[0]
    }
    cz q[3], q[0]
    { # start at cycle 359
        y90 q[2]
        ym90 q[5]
    }
    cz q[2], q[5]
    skip 1
    { # start at cycle 362
        y90 q[0]
        ym90 q[3]
    }
    cz q[0], q[3]
    { # start at cycle 364
        y90 q[5]
        ym90 q[2]
    }
    cz q[5], q[2]
    skip 1
    { # start at cycle 367
        y90 q[2]
        ym90 q[0]
    }
    cz q[2], q[0]
    skip 1
    { # start at cycle 370
        y90 q[0]
        ym90 q[2]
    }
    cz q[0], q[2]
    y90 q[4]
    cz q[5], q[1]
    { # start at cycle 374
        y90 q[2]
        ym90 q[0]
    }
    { # start at cycle 375
        cz q[4], q[1]
        cz q[2], q[0]
    }
    skip 1
    { # start at cycle 377
        ym90 q[5]
        ym90 q[2]
    }
    cz q[5], q[2]
    skip 1
    { # start at cycle 380
        y90 q[2]
        ym90 q[5]
    }
    cz q[2], q[5]
    x q[1]
    { # start at cycle 383
        y90 q[5]
        y q[1]
    }
    cz q[1], q[5]
    skip 1
    y90 q[1]
    x q[1]
    measure q[1]
    skip 14
    prepz q[1]
    skip 30
    ym90 q[2]
    { # start at cycle 435
        cz q[5], q[2]
        y90 q[1]
    }
    x q[1]
    cz q[1], q[5]
    skip 1
    { # start at cycle 439
        y90 q[5]
        ym90 q[1]
    }
    cz q[5], q[1]
    skip 1
    { # start at cycle 442
        y90 q[1]
        ym90 q[5]
    }
    cz q[1], q[5]
    skip 1
    { # start at cycle 445
        y90 q[5]
        y90 q[2]
    }
    cz q[5], q[2]
    skip 1
    y90 q[2]
    cz q[2], q[0]
    skip 1
    { # start at cycle 451
        y90 q[0]
        ym90 q[2]
    }
    cz q[0], q[2]
    skip 1
    { # start at cycle 454
        y90 q[2]
        ym90 q[0]
    }
    cz q[2], q[0]
    skip 1
    x q[5]
    y q[5]
    cz q[2], q[5]
    skip 1
    cz q[2], q[5]
    skip 1
    measure q[2]
    skip 2
    y90 q[0]
    cz q[0], q[3]
    ym90 q[4]
    cz q[1], q[4]
    { # start at cycle 470
        y90 q[3]
        ym90 q[0]
    }
    cz q[3], q[0]
    skip 1
    cz q[1], q[5]
    { # start at cycle 474
        y90 q[0]
        ym90 q[3]
    }
    cz q[0], q[3]
    { # start at cycle 476
        y90 q[4]
        ym90 q[1]
    }
    { # start at cycle 477
        cz q[4], q[1]
        ym90 q[0]
    }
    cz q[2], q[0]
    y90 q[1]
    cz q[1], q[5]
    { # start at cycle 481
        y90 q[0]
        ym90 q[2]
    }
    cz q[0], q[2]
    skip 1
    x q[5]
    { # start at cycle 485
        y90 q[2]
        y q[5]
    }
    cz q[5], q[2]
    skip 1
    y90 q[5]
    x q[5]
    measure q[5]
    skip 10
    { # start at cycle 501
        ym90 q[4]
        ym90 q[0]
    }
    { # start at cycle 502
        cz q[1], q[4]
        cz q[2], q[0]
    }
    skip 1
    { # start at cycle 504
        y90 q[0]
        y90 q[2]
        y90 q[3]
        y90 q[4]
        y90 q[6]
    }
