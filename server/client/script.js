let a = [
    [0.89534, -0.017772, -0.01735],
    [-0.017772, 0.883792, -0.022609],
    [-0.01735, -0.022609, 0.971979],
];

let b = [19.03355, 27.47569, 66.334158];
let heading = 0;

var socket = io("http://localhost:3000/");

var map = L.map("mapCanvas").setView([15.455956, 73.802159], 15);
L.tileLayer("https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png", {
    attribution: "© OpenStreetMap",
}).addTo(map);

var theMarker = {};

map.on("click", function (e) {
    lat = e.latlng.lat;
    lon = e.latlng.lng;

    console.log("You clicked the map at LAT: " + lat + " and LONG: " + lon);
    //Clear existing marker,

    if (theMarker != undefined) {
        map.removeLayer(theMarker);
    }

    //Add a marker to show where you clicked.
    theMarker = L.marker([lat, lon]).addTo(map);
});

var marker = L.marker([15.455956, 73.802159]).addTo(map);

function parentWidth(elem) {
    return elem.parentElement.clientWidth;
}

function parentHeight(elem) {
    return elem.parentElement.clientHeight;
}

socket.on("message", function (msg) {
    let cords = msg.split(";");
    if (cords[0] === "$imu") {
        document.getElementById("accl_x").innerHTML = cords[1];
        document.getElementById("accl_y").innerHTML = cords[2];
        document.getElementById("accl_z").innerHTML = cords[3];

        document.getElementById("gyro_x").innerHTML = cords[4];
        document.getElementById("gyro_y").innerHTML = cords[5];
        document.getElementById("gyro_z").innerHTML = cords[6];

        const res = multiply(a, [
            [Number(cords[7]) - b[0]],
            [Number(cords[8]) - b[1]],
            [Number(cords[9]) - b[2]],
        ]);

        document.getElementById("magneto_x").innerHTML = Math.round(res[0] * 10000) / 10000;
        document.getElementById("magneto_y").innerHTML = Math.round(res[1] * 10000) / 10000;
        document.getElementById("magneto_z").innerHTML = Math.round(res[2] * 10000) / 10000;

        if (res[0] >= 0) {
            heading = Math.round(((Math.atan(res[1] / res[0]) * 180) / Math.PI) * 10000) / 10000;
        } else {
            if (res[1] >= 0) {
                heading =
                    Math.round((180 + (Math.atan(res[1] / res[0]) * 180) / Math.PI) * 10000) /
                    10000;
            } else {
                heading =
                    Math.round((-180 + (Math.atan(res[1] / res[0]) * 180) / Math.PI) * 10000) /
                    10000;
            }
        }

        document.getElementById("heading").innerHTML = heading;
    } else if (cords[0] === "$gps") {
        document.getElementById("latitude").innerHTML = cords[3];
        document.getElementById("longitude").innerHTML = cords[4];
        document.getElementById("speed").innerHTML = cords[5];
        document.getElementById("true_course").innerHTML = cords[6];
    }
});

const compassCanvas = document.getElementById("compassCanvas");

function setup() {
    const canvas = createCanvas(200, 200);
    canvas.parent("compassCanvas");
    // compass = new Compass();
    // compass.init(compassReady);
    angleMode(DEGREES);
}

function draw() {
    background(254, 252, 251);
    translate(100, 100);
    rotate(heading);
    fill(0);
    rect(-10, -50, 20, 100);
}

// m1 -
function multiply(a, b) {
    var aNumRows = a.length,
        aNumCols = a[0].length,
        bNumRows = b.length,
        bNumCols = b[0].length,
        m = new Array(aNumRows); // initialize array of rows
    for (var r = 0; r < aNumRows; ++r) {
        m[r] = new Array(bNumCols); // initialize the current row
        for (var c = 0; c < bNumCols; ++c) {
            m[r][c] = 0; // initialize the current cell
            for (var i = 0; i < aNumCols; ++i) {
                m[r][c] += a[r][i] * b[i][c];
            }
        }
    }
    return m;
}
