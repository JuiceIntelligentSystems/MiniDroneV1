/*
/---------------------------------------------------------------------------/
/                    23 1 *                    * 2 25                       /
/                 SWITCH   \------------------/                             /
/                 FRONT    /------------------\                             /
/                    27 4 *                    * 3 26                       /
/---------------------------------------------------------------------------/

TODO:
    -Add Joystick Controls
    -Update PID Setpoint to match controls
    -Add Buzzer Support
    -Add Altitude Correction
    -Fix Battery Voltage Reading
    -Add Safety Features
    -Tune PID

BUZZER = GPIO19
BATTERY VOLTAGE = GPIO32
*/

#include "sensors.h"
#include "controller.h"

Controller controller;
Sensors sensors;

void setup()
{
    Serial.begin(115200);

    Serial.println("Initializing Flight Controller...");
    controller.init();
    sensors.initSensors();

    net.initNetwork();
    net.startNetwork();

    controller.calibrateESCs();

    Serial.println("Drone Ready");
}

void loop()
{
    controller.loopTimer();

    sensors.getGyroReadings(false);
    sensors.getBaroReadings();

    controller.sendPIDGains();
    controller.sendVBatt();

    controller.setSetpoints(control_input);
    controller.calculatePID(sensors.euler_z, sensors.euler_y, sensors.gyro_yaw);

    controller.loopRate(100);

    controller.control(throttle);

    net.sendData(sensors.gyro_json, sensors.baro_json, controller.PID_gains_json, controller.vbatt_json);
    if (calibrateSensors == true)
    {
        sensors.calibrateBNO();
        calibrateSensors = false;
    }
}
