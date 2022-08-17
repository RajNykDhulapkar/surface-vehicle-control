require("dotenv").config();
const express = require("express");
const app = express();
const http = require("http").Server(app);
const io = require("socket.io")(http);
const httpPort = parseInt(process.env.PORT, 10) || 8000;
const { SerialPort } = require("serialport");
const { ReadlineParser } = require("@serialport/parser-readline");
const { ByteLengthParser } = require("@serialport/parser-byte-length");
let port = new SerialPort({
    path: process.env.SERIAL_PORT || "COM7",
    baudRate: parseInt(process.env.BAUD_RATE, 10) || 9600,
});
// const parser = port.pipe(new ReadlineParser({ delimiter: "\r\n" }));
let parser = port.pipe(new ByteLengthParser({ length: 30 }));
const path = require("path");
const fletcher16 = require("fletcher");

// Read the port data
// parser.on("data", (data) => {
//     io.emit("message", data);

//     console.log("got word from arduino:", data);
// });

app.get("/", (req, res) => {
    res.sendFile(__dirname + "/client/index.html");
});
app.use("/static", express.static(path.join(__dirname, "client")));
app.use("/images", express.static(path.join(__dirname, "client/images")));

// socket
io.on("connection", (socket) => {
    port.on("open", () => {
        console.log("serial port open");
    });
    parser.on("data", (data) => {
        // io.emit("message", data);
        console.log("recv: ", data);
        io.emit("message", processData(data));
    });

    socket.on("refresh", () => {
        console.log("flush serial");
        if (port.isOpen) {
            port.close();
        }
        port = new SerialPort({
            path: process.env.SERIAL_PORT || "COM7",
            baudRate: parseInt(process.env.BAUD_RATE, 10) || 9600,
        });
    });

    socket.on("cmd_message", (msg) => {
        console.log(msg);
        msg = msg.split(",");
        if (msg[0] == "$cmd") {
            if (msg[3] == "0") {
                if (msg[4] == "2") {
                    let buf = Buffer.from([0x63]); // msg id
                    buf = appendUInt8ToBuff(buf, 0);
                    buf = appendUInt8ToBuff(buf, 0);
                    buf = appendUInt8ToBuff(buf, 0);
                    buf = appendUInt8ToBuff(buf, 2);
                    const crc = fletcher16(buf);
                    console.log(buf);
                    buf = appendUInt16ToBuff(buf, crc);
                    console.log(crc);
                    console.log(Buffer.from([0x24, ...buf, 0x0d, 0x0a]));
                    port.write(Buffer.from([0x24, ...buf, 0x0d, 0x0a]));
                }
                if (msg[4] == "0") {
                    let buf = Buffer.from([0x63]); // msg id
                    buf = appendUInt8ToBuff(buf, 0);
                    buf = appendUInt8ToBuff(buf, 0);
                    buf = appendUInt8ToBuff(buf, 0);
                    buf = appendUInt8ToBuff(buf, 0);
                    const crc = fletcher16(buf);
                    console.log(buf);
                    buf = appendUInt16ToBuff(buf, crc);
                    console.log(crc);
                    console.log(Buffer.from([0x24, ...buf, 0x0d, 0x0a]));
                    port.write(Buffer.from([0x24, ...buf, 0x0d, 0x0a]));
                }
            }
        }
    });
    socket.on("mis_message", (msg) => {
        msg = msg.split(",");
        let buf = Buffer.from([0x6d]); // msg id
        buf = appendFloatBEToBuff(buf, Number(msg[1]));
        buf = appendFloatBEToBuff(buf, Number(msg[2]));
        const crc = fletcher16(buf);
        console.log(buf);
        buf = appendUInt16ToBuff(buf, crc);
        console.log(crc);
        console.log(Buffer.from([0x24, ...buf, 0x0d, 0x0a]));
        port.write(Buffer.from([0x24, ...buf, 0x0d, 0x0a]));
    });
});

function processData(data) {
    // console.log([...data]);

    if (String.fromCharCode(data[0]) == "s") {
        return handleStatusMessage([...data]);
    }
}

function appendUInt8ToBuff(buff, num) {
    let dataBuf = Buffer.allocUnsafe(1);
    dataBuf.writeUint8(num);
    return Buffer.concat([buff, dataBuf]);
}
function appendUInt16ToBuff(buff, num) {
    let dataBuf = Buffer.allocUnsafe(2);
    dataBuf.writeUInt16BE(num);
    return Buffer.concat([buff, dataBuf]);
}
function appendFloatBEToBuff(buff, num) {
    let dataBuf = Buffer.allocUnsafe(4);
    dataBuf.writeFloatBE(num);
    return Buffer.concat([buff, dataBuf]);
}

function handleStatusMessage(data) {
    clientMessage = "$status,";
    for (let i = 1; i < 21; i += 4) {
        var intArray = [];
        for (let j = i; j < i + 4; j++) {
            intArray[j - i] = data[j];
        }
        // console.log(intArray);
        value = Buffer.from(intArray.reverse()).readFloatBE(0);
        clientMessage += String(Math.round(value * 1000000) / 1000000) + ",";
        // console.log(value);
    }
    const mode = Buffer.from([data[21]]).readUInt8(0);
    clientMessage += String(mode + ",");
    for (let i = 22; i < 28; i += 2) {
        var intArray = [];
        for (let j = i; j < i + 2; j++) {
            intArray[j - i] = data[j];
        }
        // console.log(intArray);
        if (i < 24) {
            value = Buffer.from(intArray.reverse()).readInt16BE(0);
            clientMessage += String(value) + ",";
        } else if (i < 26) {
            value = Buffer.from(intArray.reverse()).readInt16BE(0);
            clientMessage += String(value);
        } else {
            // crc
            value = Buffer.from(intArray.reverse()).readUInt16BE(0);
            clientMessage += "*" + String(value);
        }

        // console.log(value);
    }
    console.log(clientMessage);
    return clientMessage;
}
// hexString = yourNumber.toString(16);
//yourNumber = parseInt(hexString, 16);

http.listen(httpPort, () => {
    console.log(`Socket.IO server running at http://localhost:${httpPort}/`);
});
