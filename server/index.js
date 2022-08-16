require("dotenv").config();
const app = require("express")();
const http = require("http").Server(app);
const io = require("socket.io")(http);
const httpPort = parseInt(process.env.PORT, 10) || 8000;
const { SerialPort } = require("serialport");
const { ReadlineParser } = require("@serialport/parser-readline");
const { ByteLengthParser } = require("@serialport/parser-byte-length");
const port = new SerialPort({
    path: process.env.SERIAL_PORT || "COM7",
    baudRate: parseInt(process.env.BAUD_RATE, 10) || 9600,
});
// const parser = port.pipe(new ReadlineParser({ delimiter: "\r\n" }));
const parser = port.pipe(new ByteLengthParser({ length: 29 }));

// Read the port data
// parser.on("data", (data) => {
//     io.emit("message", data);

//     console.log("got word from arduino:", data);
// });

app.get("/", (req, res) => {
    res.sendFile(__dirname + "/client/index.html");
});
app.get("/static/style.css", (req, res) => {
    res.sendFile(__dirname + "/client/style.css");
});
app.get("/static/script.js", (req, res) => {
    res.sendFile(__dirname + "/client/script.js");
});

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

    socket.on("chat message", (msg) => {
        io.emit("chat message", msg);
    });
});

function processData(data) {
    console.log([...data]);

    if (String.fromCharCode(data[0]) == "s") {
        return handleStatusMessage([...data]);
    }
}

function handleStatusMessage(data) {
    clientMessage = "$status,";
    for (let i = 1; i < 21; i += 4) {
        var intArray = [];
        for (let j = i; j < i + 4; j++) {
            intArray[j - i] = data[j];
        }
        console.log(intArray);
        value = Buffer.from(intArray.reverse()).readFloatBE(0);
        clientMessage += String(value) + ",";
        console.log(value);
    }
    for (let i = 21; i < 27; i += 2) {
        var intArray = [];
        for (let j = i; j < i + 2; j++) {
            intArray[j - i] = data[j];
        }
        console.log(intArray);
        value = Buffer.from(intArray.reverse()).readInt16BE(0);
        if (i < 25) clientMessage += String(value) + ",";
        else clientMessage += "*" + String(value);

        console.log(value);
    }
    console.log(clientMessage);
    return clientMessage;
}
// hexString = yourNumber.toString(16);
//yourNumber = parseInt(hexString, 16);

http.listen(httpPort, () => {
    console.log(`Socket.IO server running at http://localhost:${httpPort}/`);
});
