/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-mpu-6050-web-server/

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/

var socket = io("http://localhost:3000/");

socket.on("message", function (msg) {
    console.log(msg.split(" "));
});

let scene, camera, rendered, cube;

function parentWidth(elem) {
    return elem.parentElement.clientWidth;
}

function parentHeight(elem) {
    return elem.parentElement.clientHeight;
}

function init3D() {
    scene = new THREE.Scene();
    scene.background = new THREE.Color(0xffffff);

    camera = new THREE.PerspectiveCamera(
        70,
        parentWidth(document.getElementById("3Dcube")) /
            parentHeight(document.getElementById("3Dcube")),
        1,
        1000
    );

    renderer = new THREE.WebGLRenderer({ antialias: true });
    renderer.setSize(
        parentWidth(document.getElementById("3Dcube")),
        parentHeight(document.getElementById("3Dcube"))
    );

    document.getElementById("3Dcube").appendChild(renderer.domElement);

    // Create a geometry
    const geometry = new THREE.BoxBufferGeometry(5, 1, 4);

    // Materials of each face
    var cubeMaterials = [
        new THREE.MeshBasicMaterial({ color: 0x03045e }),
        new THREE.MeshBasicMaterial({ color: 0x023e8a }),
        new THREE.MeshBasicMaterial({ color: 0x0077b6 }),
        new THREE.MeshBasicMaterial({ color: 0x03045e }),
        new THREE.MeshBasicMaterial({ color: 0x023e8a }),
        new THREE.MeshBasicMaterial({ color: 0x0077b6 }),
    ];

    const material = new THREE.MeshFaceMaterial(cubeMaterials);

    cube = new THREE.Mesh(geometry, material);
    scene.add(cube);
    camera.position.z = 5;
    renderer.render(scene, camera);
}

// Resize the 3D object when the browser window changes size
function onWindowResize() {
    camera.aspect =
        parentWidth(document.getElementById("3Dcube")) /
        parentHeight(document.getElementById("3Dcube"));
    //camera.aspect = window.innerWidth /  window.innerHeight;
    camera.updateProjectionMatrix();
    //renderer.setSize(window.innerWidth, window.innerHeight);
    renderer.setSize(
        parentWidth(document.getElementById("3Dcube")),
        parentHeight(document.getElementById("3Dcube"))
    );
}

window.addEventListener("resize", onWindowResize, false);

// Create the 3D representation
init3D();

// Create events for the sensor readings
if (!!window.EventSource) {
    socket.on("message", function (msg) {
        let cords = msg.split(" ");
        document.getElementById("accX").innerHTML = cords[0];
        document.getElementById("accY").innerHTML = cords[1];
        document.getElementById("accZ").innerHTML = cords[2];

        document.getElementById("gyroX").innerHTML = cords[3];
        document.getElementById("gyroY").innerHTML = cords[4];
        document.getElementById("gyroZ").innerHTML = cords[5];

        document.getElementById("temp").innerHTML = cords[9];

        let magAccl = Math.sqrt(
            Math.pow(Number(cords[0]), 2) +
                Math.pow(Number(cords[1]), 2) +
                Math.pow(Number(cords[2]), 2)
        );
        let Xrotation = Math.acos(Number(cords[0]) / magAccl);
        let Yrotation = Math.acos(Number(cords[1]) / magAccl);
        let Zrotation = Math.acos(Number(cords[2]) / magAccl);

        // Change cube rotation after receiving the readinds
        cube.rotation.x = 10;
        cube.rotation.z = 10;
        renderer.render(scene, camera);
    });
}

function resetPosition(element) {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/" + element.id, true);
    console.log(element.id);
    xhr.send();
}
