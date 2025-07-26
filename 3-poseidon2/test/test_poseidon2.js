const fs = require('fs');

// 隐私输入
const preimage = [1n, 2n];

// 公开输入
const hash = 0x1234567890abcdefn; 

// 生成输入文件
const input = {
    preimage: preimage.map(x => x.toString()),
    hash: hash.toString()
};

fs.writeFileSync('input.json', JSON.stringify(input, null, 2));
console.log("生成输入文件：input.json");
