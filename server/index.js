require("dotenv").config();
const app = require("express")();
const http = require("http").Server(app);
const io = require("socket.io")(http);
const httpPort = parseInt(process.env.PORT, 10) || 8000;
const { SerialPort } = require("serialport");
const { ReadlineParser } = require("@serialport/parser-readline");
const port = new SerialPort({
    path: process.env.SERIAL_PORT || "COM7",
    baudRate: parseInt(process.env.BAUD_RATE, 10) || 9600,
});
const parser = port.pipe(new ReadlineParser({ delimiter: "\n" }));

// Read the port data

app.get("/", (req, res) => {
    res.sendFile(__dirname + "/client/index.html");
});
app.get("/static/style.css", (req, res) => {
    res.sendFile(__dirname + "/client/style.css");
});
app.get("/static/script.js", (req, res) => {
    res.sendFile(__dirname + "/client/script.js");
});

io.on("connection", (socket) => {
    port.on("open", () => {
        console.log("serial port open");
    });
    parser.on("data", (data) => {
        io.emit("message", data);

        // console.log("got word from arduino:", data);
    });

    socket.on("chat message", (msg) => {
        io.emit("chat message", msg);
    });
});

http.listen(httpPort, () => {
    console.log(`Socket.IO server running at http://localhost:${httpPort}/`);
});
