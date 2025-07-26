#!/bin/bash

# 安装依赖
npm install -g circom@2.1.8 snarkjs@0.7.0
npm install circomlib

# 下载 Powers of Tau 文件（若未下载）
if [ ! -f "powersOfTau/pot12_0000.ptau" ]; then
    mkdir -p powersOfTau
    wget https://hermez.s3-eu-west-1.amazonaws.com/powersOfTau28_hez_final_12.ptau -O powersOfTau/pot12_0000.ptau
fi

# 编译电路
cd circuits
circom poseidon2.circom --r1cs --wasm --sym
cd ..

# 生成 Groth16 证明密钥
snarkjs groth16 setup \
  circuits/poseidon2.r1cs \
  powersOfTau/pot12_0000.ptau \
  poseidon2_0000.zkey

# 贡献到 Zkey（模拟多方参与）
snarkjs zkey contribute \
  poseidon2_0000.zkey \
  poseidon2_final.zkey \
  --name="First contribution" \
  -v \
  -e="random text"

# 导出验证密钥
snarkjs zkey export verificationkey \
  poseidon2_final.zkey \
  verification_key.json

# 生成测试输入
node test/test_poseidon2.js

# 生成证明
snarkjs groth16 prove \
  poseidon2_final.zkey \
  input.json \
  proof.json \
  public.json

# 验证证明
snarkjs groth16 verify \
  verification_key.json \
  public.json \
  proof.json
