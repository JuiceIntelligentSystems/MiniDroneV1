let current_dir_pos;
let current_throttle_pos;

const throttleJoystick = createThrottleJoystick(document.getElementById("throttleJoystickWrapper"));
const directionJoystick = createDirectionJoystick(document.getElementById("directionJoystickWrapper"));
document.getElementById("defaultOpen").click(); // Open Control Menu on startup

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
/*
var throttle = document.getElementById("throttle"),
    throttle_val = document.getElementById("throttle-value");

throttle_val.innerHTML = throttle.value;

throttle.oninput = function() {
    throttle_val.innerHTML = this.value;
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/throttle?value=" + this.value, true);
    xhr.send();
}
*/

/*-------------------------PID TUNING-------------------------*/
// Sets the gain data
function setgain(element) {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/pidgain?" + element.id, true);
    console.log(element.id);
    xhr.send();
}
//Sets the multiplier of last changed value
function setMult(element) {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/pidmult?" + element.id, true);
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
    xhr.open("GET", "/calibrate?" + element.id, true);
    if (element.id == 'calibrateGyro') {
        document.getElementById('gyroCalib').style.display = "grid";
    }
    if (element.id == 'no') {
        document.getElementById('gyroCalib').style.display = "none";
    }
    if (element.id == 'yes') {
        document.getElementById("question").style.display = "none";
        document.getElementById("no").style.display = "none";
        document.getElementById("yes").style.display = "none";
    }
    console.log(element.id);
    xhr.send();
}

source.addEventListener('battery_voltage', function(e) {
    var obj = JSON.parse(e.data);
    document.getElementById('V_Batt').innerHTML = obj.VBATT;
}, false);

/*------------------------------JOYSTICK CONTROLS-------------------------------*/

function createDirectionJoystick(parent) {
    const offset = 80;

    const maxDiff = 100;
    const stick = document.createElement('div');
    stick.classList.add('joystick');

    stick.addEventListener('mousedown', handleMouseDown);
    document.addEventListener('mousemove', handleMouseMove);
    document.addEventListener('mouseup', handleMouseUp);
    stick.addEventListener('touchstart', handleMouseDown);
    document.addEventListener('touchmove', handleMouseMove);
    document.addEventListener('touchend', handleMouseUp);

    let dragStart = null;
    current_dir_pos = { x: 0, y: 0 };
    stick.style.transform = `translate3d(${offset}px, ${offset}px, 0px)`;

    let xNew, yNew;

    function handleMouseDown(event) {
        stick.style.transition = '0.07s';
        if (event.changedTouches) {
            dragStart = {
                x: event.changedTouches[0].clientX,
                y: event.changedTouches[0].clientY,
            };
            return;
        }
        dragStart = {
            x: event.clientX,
            y: event.clientY,
        };

    }

    function handleMouseMove(event) {
        if (dragStart === null) return;
        event.preventDefault();
        if (event.changedTouches) {
            event.clientX = event.changedTouches[0].clientX;
            event.clientY = event.changedTouches[0].clientY;
        }
        const xDiff = event.clientX - dragStart.x;
        const yDiff = event.clientY - dragStart.y;
        const angle = Math.atan2(yDiff, xDiff);
        const distance = Math.min(maxDiff, Math.hypot(xDiff, yDiff));
        xNew = distance * Math.cos(angle) + offset;
        yNew = distance * Math.sin(angle) + offset;

        stick.style.transform = `translate3d(${xNew}px, ${yNew}px, 0px)`;

        current_dir_pos = { x: (xNew - offset) * 10, y: (yNew - offset) * 10 };

        // Send Data
        var xhr_pitch = new XMLHttpRequest();
        var xhr_roll = new XMLHttpRequest();

        xhr_pitch.open("GET", "/controls?pitch=" + Math.round(current_dir_pos.y), true);
        xhr_roll.open("GET", "/controls?roll=" + Math.round(current_dir_pos.x), true);

        xhr_pitch.send();
        xhr_roll.send();
        //console.log("X: " + Math.round(currentPos.x) + " Y: " + Math.round(currentPos.y));
    }

    function handleMouseUp(event) {
        if (dragStart === null) return;
        stick.style.transition = '.2s';
        stick.style.transform = `translate3d(${offset}px, ${offset}px, 0px)`;
        dragStart = null;
        current_dir_pos = { x: 0, y: 0 };

        // Send data again so that the drone doesn't go in a direction forever
        var xhr_pitch = new XMLHttpRequest();
        var xhr_roll = new XMLHttpRequest();

        xhr_pitch.open("GET", "/controls?pitch=" + current_dir_pos.y, true);
        xhr_roll.open("GET", "/controls?roll=" + current_dir_pos.x, true);

        xhr_pitch.send();
        xhr_roll.send();
    }

    parent.appendChild(stick);
    return {
        getPosition: () => current_dir_pos,
    };
}

function createThrottleJoystick(parent) {
    const offset = 80;

    const maxDiff = 100;
    const stick = document.createElement('div');
    stick.classList.add('joystick');

    stick.addEventListener('mousedown', handleMouseDown);
    document.addEventListener('mousemove', handleMouseMove);
    document.addEventListener('mouseup', handleMouseUp);
    stick.addEventListener('touchstart', handleMouseDown);
    document.addEventListener('touchmove', handleMouseMove);
    document.addEventListener('touchend', handleMouseUp);

    let dragStart = null;
    current_throttle_pos = { x: 0, y: 1000 };
    stick.style.transform = `translate3d(${offset}px, 180px, 0px)`;

    let xNew = 0,
        yNew = 0;

    function handleMouseDown(event) {
        stick.style.transition = '0.07s';
        if (event.changedTouches) {
            dragStart = {
                x: event.changedTouches[0].clientX,
                y: event.changedTouches[0].clientY,
            };
            return;
        }
        dragStart = {
            x: event.clientX,
            y: event.clientY,
        };
    }

    function handleMouseMove(event) {
        if (dragStart === null) return;
        event.preventDefault();
        if (event.changedTouches) {
            event.clientX = event.changedTouches[0].clientX;
            event.clientY = event.changedTouches[0].clientY;
        }
        const xDiff = event.clientX - dragStart.x;
        const yDiff = event.clientY - dragStart.y;
        const angle = Math.atan2(yDiff, xDiff);
        const distance = Math.min(maxDiff, Math.hypot(xDiff, yDiff));

        if (Math.abs(xDiff) > 30) {
            xNew = distance * Math.cos(angle) + offset;
            yNew = distance * Math.sin(angle) + offset;

            current_throttle_pos = { x: (xNew - offset) * 10, y: (yNew - offset) * 10 };
        } else {
            xNew = Math.cos(angle) + offset;
            yNew = event.clientY - stick.offsetTop;
            yNew = Math.max(offset - maxDiff, Math.min(offset + maxDiff, yNew));

            current_throttle_pos = { x: 0, y: (yNew - offset) * 10 };
        }

        stick.style.transform = `translate3d(${xNew}px, ${yNew}px, 0px)`;

        // Send Data
        var xhr_throttle = new XMLHttpRequest();
        var xhr_yaw = new XMLHttpRequest();

        xhr_throttle.open("GET", "/controls?throttle=" + Math.round(current_throttle_pos.y), true);
        xhr_yaw.open("GET", "/controls?yaw=" + Math.round(current_throttle_pos.x), true);

        xhr_throttle.send();
        xhr_yaw.send();
        //console.log("X: " + Math.round(current_throttle_pos.x) + " Y: " + Math.round(current_throttle_pos.y));
    }

    function handleMouseUp(event) {
        if (dragStart === null) return;
        stick.style.transition = '.2s';
        stick.style.transform = `translate3d(${offset}px, ${yNew}px, 0px)`;
        dragStart = null;
        current_throttle_pos = { x: 0, y: (yNew - offset) * 10 };

        // Send data again so that the drone doesn't go in a direction forever
        var xhr_throttle = new XMLHttpRequest();
        var xhr_yaw = new XMLHttpRequest();

        xhr_throttle.open("GET", "/controls?throttle=" + Math.round(current_throttle_pos.y), true);
        xhr_yaw.open("GET", "/controls?yaw=" + Math.round(current_throttle_pos.x), true);

        xhr_throttle.send();
        xhr_yaw.send();
    }

    parent.appendChild(stick);
    return {
        getPosition: () => current_throttle_pos,
    };
}