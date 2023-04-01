document.getElementById("defaultOpen").click(); // Open Control Menu on startup

/* -------------------------3D DRONE REPRESENTATION------------------------- */

let scene, camera, renderer, cube;

function parentWidth(elem) {
    return elem.parentElement.clientWidth;
}

function parentHeight(elem) {
    return elem.parentElement.clientHeight;
}

function init3D() {
    scene = new THREE.Scene();
    scene.background = new THREE.Color(0xffffff);

    camera = new THREE.PerspectiveCamera(75, parentWidth(document.getElementById("3Dcube")) / parentHeight(document.getElementById("3Dcube")), 0.1, 1000);

    renderer = new THREE.WebGLRenderer({ antialias: true });
    renderer.setSize(parentWidth(document.getElementById("3Dcube")), parentHeight(document.getElementById("3Dcube")));


    document.getElementById('3Dcube').appendChild(renderer.domElement);

    const geometry = new THREE.BoxGeometry(5, 1, 4);

    var cubeMaterials = [
        new THREE.MeshBasicMaterial({ color: 0x03045e }),
        new THREE.MeshBasicMaterial({ color: 0x023e8a }),
        new THREE.MeshBasicMaterial({ color: 0x0077b6 }),
        new THREE.MeshBasicMaterial({ color: 0x03045e }),
        new THREE.MeshBasicMaterial({ color: 0x023e8a }),
        new THREE.MeshBasicMaterial({ color: 0x0077b6 }),
    ];

    // const material = new THREE.MeshFaceMaterial(cubeMaterials);

    cube = new THREE.Mesh(geometry, cubeMaterials);
    scene.add(cube);
    camera.position.z = 5;
    renderer.render(scene, camera);
}

function onWindowResize() {
    camera.aspect = parentWidth(document.getElementById("3Dcube")) / parentHeight(document.getElementById("3Dcube"));

    camera.updateProjectionMatrix();

    renderer.setSize(parentWidth(document.getElementById("3Dcube")), parentHeight(document.getElementById("3Dcube")));
}

window.addEventListener('resize', onWindowResize, false);

init3D();

/* -------------------------READ SENSOR DATA------------------------- */

if (!!window.EventSource) {
    var source = new EventSource('/events');

    source.addEventListener('open', function(e) {
        console.log("Events Connected");
    }, false);

    source.addEventListener('error', function(e) {
        if (e.target.readyState != EventSource.OPEN) {
            console.log("Events Disconnected");
        }
    }, false);

    source.addEventListener('gyro_readings', function(e) {
        var obj = JSON.parse(e.data);
        document.getElementById("eulerX").innerHTML = obj.eulerX;
        document.getElementById("eulerY").innerHTML = obj.eulerY;
        document.getElementById("eulerZ").innerHTML = obj.eulerZ;

        cube.rotation.x = obj.eulerY / 100;
        cube.rotation.z = obj.eulerZ / 100;
        cube.rotation.y = obj.eulerX / 100;
        renderer.render(scene, camera);
    }, false);

    source.addEventListener('baro_readings', function(e) {
        var obj = JSON.parse(e.data);
        document.getElementById('ALT').innerHTML = obj.ALT;
        document.getElementById('PRESS').innerHTML = obj.PRESS;
        document.getElementById('TEMP').innerHTML = obj.TEMP;
    }, false);

    source.addEventListener('calib_stage', function(e) {
        var obj = JSON.parse(e.data);
        document.getElementById('CALIBDAT').innerHTML = obj.CALIBDAT;
        if (obj.CALIBDAT == 1) {
            document.getElementById("stage1").style.display = "grid";
        }
        if (obj.CALIBDAT == 2) {
            document.getElementById("stage2").style.display = "grid";
        }
        if (obj.CALIBDAT == 3) {
            document.getElementById("stage3").style.display = "grid";
        }
        if (obj.CALIBDAT == 4) {
            document.getElementById("stage4").style.display = "grid";
        }
        if (obj.CALIBDAT == 5) {
            document.getElementById("stage5").style.display = "grid";
        }
        if (obj.CALIBDAT == 6) {
            document.getElementById("stage6").style.display = "grid";
        }
    }, false);
}

/* -------------------------MENU SCRIPT------------------------- */

function menuPressed() {
    document.getElementById("Menu").classList.toggle("show");
}

window.onclick = function(event) {
    if (!event.target.matches(".menubtn")) {
        var dropdowns = document.getElementsByClassName("menu-content");
        for (var i = 0; i < dropdowns.length; i++) {
            var openDropdown = dropdowns[i];
            if (openDropdown.classList.contains('show')) {
                openDropdown.classList.remove('show');
            }
        }
    }
}

function openWindow(evt, winName) {
    var tabcontent;

    tabcontent = document.getElementsByClassName("tabcontent");
    for (var i = 0; i < tabcontent.length; i++) {
        tabcontent[i].style.display = "none";
    }

    document.getElementById(winName).style.display = "block";
}

/* -------------------------DISPLAY + SEND THROTTLE VALUE------------------------- */
var throttle = document.getElementById("throttle"),
    throttle_val = document.getElementById("throttle-value");

throttle_val.innerHTML = throttle.value;

throttle.oninput = function() {
    throttle_val.innerHTML = this.value;
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/throttle?value=" + this.value, true);
    xhr.send();
}

/*-------------------------PID TUNING-------------------------*/
// Sets the gain data
function setgain(element) {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/" + element.id, true);
    console.log(element.id);
    xhr.send();
}
//Sets the multiplier of last changed value
function setMult(element) {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/" + element.id, true);
    console.log(element.id);
    xhr.send();
}

// Receives and displays gain values
source.addEventListener('pid_gains', function(e) {
    var obj = JSON.parse(e.data);
    document.getElementById('P_pitchroll').innerHTML = obj.PPitchRoll;
    document.getElementById('I_pitchroll').innerHTML = obj.IPitchRoll;
    document.getElementById('D_pitchroll').innerHTML = obj.DPitchRoll;

    document.getElementById('P_yaw').innerHTML = obj.PYaw;
    document.getElementById('I_yaw').innerHTML = obj.IYaw;
    document.getElementById('D_yaw').innerHTML = obj.DYaw;
}, false);

/*-------------------------CONTROL BUTTONS-------------------------*/
// Sends info to the ESP
function controlMessage(element) {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/" + element.id, true);
    if (element.id == 'calibrateGyro') {
        document.getElementById('gyroCalib').style.display = "grid";
    }
    if (element.id == 'nocalibrate') {
        document.getElementById('gyroCalib').style.display = "none";
    }
    if (element.id == 'yescalibrate') {
        document.getElementById("question").style.display = "none";
        document.getElementById("nocalibrate").style.display = "none";
        document.getElementById("yescalibrate").style.display = "none";
    }
    console.log(element.id);
    xhr.send();
}

source.addEventListener('battery_voltage', function(e) {
    var obj = JSON.parse(e.data);
    document.getElementById('V_Batt').innerHTML = obj.VBATT;
}, false);