const fletcher16 = require("fletcher");
const data = Buffer.from([1, 2, 3, 4]);
const crc = fletcher16(data);
console.log(crc);
