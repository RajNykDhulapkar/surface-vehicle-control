let a = [
    [0.89534, -0.017772, -0.01735],
    [-0.017772, 0.883792, -0.022609],
    [-0.01735, -0.022609, 0.971979],
];

const t_1 = document.querySelector("#t_1");
const t_2 = document.querySelector("#t_2");

t_1.style.height = `${t_1.dataset.value / 10}%`;
t_2.style.height = `${t_2.dataset.value / 10}%`;

const geoJson = {
    type: "FeatureCollection",
    features: [
        {
            type: "Feature",
            properties: {},
            geometry: {
                type: "LineString",
                coordinates: [
                    [73.803382, 15.457496],
                    [73.802101, 15.456031],
                ],
            },
        },
    ],
};

let b = [19.03355, 27.47569, 66.334158];
let heading = 0;

var socket = io("http://localhost:3000/");

const map = L.map("mapCanvas").setView([15.456031, 73.802101], 17);
L.tileLayer("https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png", {
    attribution: "© OpenStreetMap",
}).addTo(map);

L.geoJSON(geoJson).addTo(map);

var marker1 = L.marker([15.457496, 73.803382]).addTo(map);
var marker2 = L.marker([15.456031, 73.802101]).addTo(map);
//Leaflet Draw Event
// map.on("click", function (e) {
//     var marker = L.marker(e.latlng).addTo(map);
//     var markerpopup = L.popup({});
//     //Set popup lat lng where clicked
//     markerpopup.setLatLng(e.latlng);
//     //console.log(e.layer._latlng);
//     //Set popup content
//     markerpopup.setContent("Popup");
//     //Bind marker popup
//     marker.bindPopup(markerpopup);
//     //Add marker to geojson layer
//     drawnItems.addLayer(marker);
// });

function parentWidth(elem) {
    return elem.parentElement.clientWidth;
}

function parentHeight(elem) {
    return elem.parentElement.clientHeight;
}

socket.on("message", function (msg) {
    let cords = msg.split(",");
    if (cords[0] === "$status") {
        const lat = Number(cords[1]);
        const lon = Number(cords[2]);
        document.getElementById("td_latitude").innerHTML = lat;
        document.getElementById("td_longitude").innerHTML = lon;
        marker2.setLatLng([lat, lon]);
        document.getElementById("td_psi_d").innerHTML = cords[3];
        document.getElementById("td_d").innerHTML = cords[4];
        heading = Number(cords[5]);
        document.getElementById("td_psi").innerHTML = heading;
        t_1.dataset.value = Number(cords[6]);
        t_2.dataset.value = Number(cords[7].split("*")[0]);
        t_1.style.height = `${t_1.dataset.value / 10}%`;
        t_2.style.height = `${t_2.dataset.value / 10}%`;
    }
});

const compassCanvas = document.getElementById("compassCanvas");

function setup() {
    const canvas = createCanvas(150, 150);
    canvas.parent("compassCanvas");
    // compass = new Compass();
    // compass.init(compassReady);
    angleMode(DEGREES);
}

function draw() {
    background(191, 253, 251);
    translate(75, 75);
    rotate(-heading + 90);
    fill(0);
    rect(-10, -50, 20, 100);
    fill(196, 98, 16);
    rect(-10, -50, 20, 20);
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
