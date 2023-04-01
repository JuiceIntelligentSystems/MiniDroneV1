#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "Arduino.h"
#include "esc.h"

#define THROTTLE_FULL 1000
#define THROTTLE_OFF 0

#define NUM_OF_MOTORS (4)

ESC esc[NUM_OF_MOTORS];

/*--------------PID TUNING--------------*/
//                       P     I     D
float pitch_gains[3] = {0.0, 0.0, 0.0};
float roll_gains[3] = {0.0, 0.0, 0.0};
float yaw_gains[3] = {0.0, 0.0, 0.0};
/*---------------------------------------*/

class Controller;

class Controller
{
private:
    float angles[3];
    float old_error[3] = {0.0, 0.0, 0.0};
    float total_error[3] = {0.0, 0.0, 0.0};
    float new_error[3] = {0.0, 0.0, 0.0};
    float pitch_setpoint = 0.0, roll_setpoint = 0.0, yaw_setpoint = 0.0;
    float angle_yaw = 0.0;
    int adjustments[3] = {0, 0, 0};

    unsigned long current_time, prev_time;
    float dt;

public:
    int new_speeds[4] = {0, 0, 0, 0};

    String PID_gains_json;
    String vbatt_json;

    void init();
    void calibrateESCs();
    void control(int *);
    void close();
    void calculatePID(float, float, float);
    void sendPIDGains();
    int batteryVoltagePercent();
    void sendVBatt();
    void loopTimer();
    void loopRate(int);
};

void Controller::init()
{
    pinMode(32, INPUT); // Battery Voltage

    for (int i = 0; i < NUM_OF_MOTORS; i++)
    {
        esc[i].setSpeed(THROTTLE_FULL); // For the Calibration Routine
        if (i == 0)
        {
            esc[i].attach(23);
            Serial.println("BLDC Pin 23 Set");
        }
        else
        {
            esc[i].attach(i + 24);
            Serial.print("BLDC Pin ");
            Serial.print(i + 24);
            Serial.println(" Set");
        }
        esc[i].setRunningMode(esc[i].MODE_FORWARD);
        esc[i].setDirection(esc[i].FORWARD);
    }
}

void Controller::calibrateESCs()
{
    Serial.println("Calibrating ESCs...");
    for (int i = 0; i < NUM_OF_MOTORS; i++)
    {
        esc[i].setSpeed(THROTTLE_FULL);
    }
    Serial.println("Throttle at max...");
    delay(50);
    for (int i = 0; i < NUM_OF_MOTORS; i++)
    {
        esc[i].setSpeed(THROTTLE_OFF);
    }
    Serial.println("Throttle at Min...");
    delay(500);
}

void Controller::calculatePID(float gyro_roll, float gyro_pitch, float gyro_yaw)
{
    for (int i = 0; i < 3; i++)
    {
        if (pitch_gains[i] < 0 || roll_gains[i] < 0)
        {
            pitch_gains[i] = 0;
            roll_gains[i] = 0;
        }
        if (yaw_gains[i] < 0)
        {
            yaw_gains[i] = 0;
        }
    }

    angle_yaw += gyro_yaw * 0.012;
    // Serial.println(angle_yaw);

    angles[0] = gyro_pitch;
    angles[1] = gyro_roll;
    angles[2] = angle_yaw;

    new_error[0] = pitch_setpoint - angles[0];
    new_error[1] = roll_setpoint - angles[1];
    new_error[2] = yaw_setpoint - angles[2];

    // Integral Calculations
    total_error[0] += pitch_gains[1] * new_error[0] * dt;
    total_error[1] += roll_gains[1] * new_error[1] * dt;
    total_error[2] += yaw_gains[1] * new_error[2] * dt;

    // Calculate Whole PID
    //                              Proportional              Integral                   Derivative
    adjustments[0] = (int)(pitch_gains[0] * new_error[0] + total_error[0] + pitch_gains[2] * ((new_error[0] - old_error[0]) / dt));
    adjustments[1] = (int)(roll_gains[0] * new_error[1] + total_error[1] + roll_gains[2] * ((new_error[1] - old_error[1]) / dt));
    adjustments[2] = (int)(yaw_gains[0] * new_error[2] + total_error[2]);
}

void Controller::control(int *bldc)
{
    //                          Pitch            Roll             Yaw
    new_speeds[0] = bldc[0] - adjustments[0] - adjustments[1] + adjustments[2];
    new_speeds[1] = bldc[1] - adjustments[0] + adjustments[1] - adjustments[2];
    new_speeds[2] = bldc[2] + adjustments[0] + adjustments[1] + adjustments[2];
    new_speeds[3] = bldc[3] + adjustments[0] - adjustments[1] - adjustments[2];

    for (int i = 0; i < NUM_OF_MOTORS; i++)
    {
        (new_speeds[i] < THROTTLE_OFF) ? (new_speeds[i] = THROTTLE_OFF) : new_speeds[i];
        (new_speeds[i] > THROTTLE_FULL) ? (new_speeds[i] = THROTTLE_FULL) : new_speeds[i];

        esc[i].setSpeed(new_speeds[i]);
        // Serial.printf("BLDC%i Speed: %i\n", i, bldc[i]);
    }

    // Update Previous Error
    for (int i = 0; i < 3; i++)
    {
        old_error[i] = new_error[i];
    }
}

void Controller::sendPIDGains()
{
    readings["PPitchRoll"] = String(pitch_gains[0]);
    readings["IPitchRoll"] = String(pitch_gains[1]);
    readings["DPitchRoll"] = String(pitch_gains[2]);

    readings["PYaw"] = String(yaw_gains[0]);
    readings["IYaw"] = String(yaw_gains[1]);
    readings["DYaw"] = String(yaw_gains[2]);

    PID_gains_json = JSON.stringify(readings);
}

void Controller::close()
{
    for (int i = 0; i < NUM_OF_MOTORS; i++)
    {
        esc[i].detach();
    }
}

int Controller::batteryVoltagePercent()
{
    int voltageDivider = analogRead(32);
    int percent = map(voltageDivider, 2803, 3680, 0, 100);

    if (percent < 0)
    {
        percent = 0;
    }
    if (percent > 100)
    {
        percent = 100;
    }

    return percent;
}

void Controller::sendVBatt()
{
    readings["VBATT"] = String(batteryVoltagePercent());

    vbatt_json = JSON.stringify(readings);
}

void Controller::loopTimer()
{
    prev_time = current_time;
    current_time = millis();
    dt = (current_time - prev_time) / 1000.0;
}

void Controller::loopRate(int freq)
{
    float invFreq = 1.0 / freq * 1000.0;
    unsigned long checker = millis();

    while (invFreq > (checker - current_time)) {
        checker = millis();
    }
}

#endif