include "constants.circom";
include "node_modules/circomlib/circuits/bitify.circom";

// S 盒：x^5 运算
template SBox5() {
    signal input in;
    signal output out;
    signal t1, t2, t3, t4;
    t1 <== in * in;        // in²
    t2 <== t1 * in;        // in³
    t3 <== t2 * in;        // in⁴
    t4 <== t3 * in;        // in⁵
    out <== t4;
    out === t4;
}

// 全轮变换
template FullRound() {
    signal input state[3];
    signal input constants[3];
    signal input mds[3][3];
    signal output out[3];

    component sboxes[3];
    for (var i = 0; i < 3; i++) {
        sboxes[i] = SBox5();
        sboxes[i].in <== state[i];
    }

    signal after_const[3];
    for (var i = 0; i < 3; i++) {
        after_const[i] <== sboxes[i].out + constants[i];
    }

    for (var i = 0; i < 3; i++) {
        out[i] <== 0;
        for (var j = 0; j < 3; j++) {
            out[i] <== out[i] + mds[i][j] * after_const[j];
        }
        out[i] === out[i];
    }
}

// 半轮变换
template PartialRound() {
    signal input state[3];
    signal input constants[3];
    signal input mds[3][3];
    signal output out[3];

    component sbox;
    sbox = SBox5();
    sbox.in <== state[0];
    signal sboxed[3];
    sboxed[0] <== sbox.out;
    sboxed[1] <== state[1];
    sboxed[2] <== state[2];

    signal after_const[3];
    for (var i = 0; i < 3; i++) {
        after_const[i] <== sboxed[i] + constants[i];
    }

    for (var i = 0; i < 3; i++) {
        out[i] <== 0;
        for (var j = 0; j < 3; j++) {
            out[i] <== out[i] + mds[i][j] * after_const[j];
        }
        out[i] === out[i];
    }
}

// 置换函数
template Poseidon2Permutation() {
    signal input state[3];
    signal input mds[3][3];
    signal input constants[64][3];
    signal output out[3];

    signal current[3];
    for (var i = 0; i < 3; i++) current[i] <== state[i];

    // 前 4 全轮
    for (var r = 0; r < 4; r++) {
        component fr = FullRound();
        for (var i = 0; i < 3; i++) {
            fr.state[i] <== current[i];
            fr.constants[i] <== constants[r][i];
            for (var j = 0; j < 3; j++) fr.mds[i][j] <== mds[i][j];
        }
        for (var i = 0; i < 3; i++) current[i] <== fr.out[i];
    }

    // 56 半轮
    for (var r = 0; r < 56; r++) {
        component pr = PartialRound();
        for (var i = 0; i < 3; i++) {
            pr.state[i] <== current[i];
            pr.constants[i] <== constants[4 + r][i];
            for (var j = 0; j < 3; j++) pr.mds[i][j] <== mds[i][j];
        }
        for (var i = 0; i < 3; i++) current[i] <== pr.out[i];
    }

    // 后 4 全轮
    for (var r = 0; r < 4; r++) {
        component fr = FullRound();
        for (var i = 0; i < 3; i++) {
            fr.state[i] <== current[i];
            fr.constants[i] <== constants[60 + r][i];
            for (var j = 0; j < 3; j++) fr.mds[i][j] <== mds[i][j];
        }
        for (var i = 0; i < 3; i++) current[i] <== fr.out[i];
    }

    for (var i = 0; i < 3; i++) out[i] <== current[i];
}

// 主电路
template Poseidon2Hash() {
    signal public output hash;
    signal private input preimage[2];

    component constants = Poseidon2Constants();
    signal initial_state[3];
    initial_state[0] <== 0;
    initial_state[1] <== preimage[0];
    initial_state[2] <== preimage[1];

    component perm = Poseidon2Permutation();
    for (var i = 0; i < 3; i++) {
        perm.state[i] <== initial_state[i];
        for (var j = 0; j < 3; j++) perm.mds[i][j] <== constants.mds[i][j];
    }
    for (var r = 0; r < 64; r++) {
        for (var i = 0; i < 3; i++) perm.constants[r][i] <== constants.constants[r][i];
    }

    hash <== perm.out[0];
}

component main = Poseidon2Hash();
