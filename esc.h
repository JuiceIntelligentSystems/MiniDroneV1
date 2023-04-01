#ifndef ESC_H
#define ESC_H

#include "Arduino.h"
#include <ESP32Servo.h>

#define BASE_FORWARD_MS 1000
#define BASE_FORWARD_BACKWARD_MS 1500
#define MAX_MS 2000
#define MIN_MS 1000

class ESC;

class ESC
{
private:
    int dir;
    int speed;
    int base_speed;
    Servo esc;
    bool checkSpeed(int);

public:
    static const int FORWARD = 0;
    static const int BACKWARD = 1;
    static const int MODE_FORWARD = 0;
    static const int MODE_FORWARD_BACKWARD = 1;

    ESC(int mode = MODE_FORWARD);

    void setSpeed(unsigned int);
    void setDirection(int);
    void setRunningMode(int);
    void detach();
    void attach(int);
};

ESC::ESC(int mode)
{
    dir = FORWARD;
    setRunningMode(mode);
}

void ESC::setSpeed(unsigned int speed)
{
    if (dir == FORWARD)
    {
        if (base_speed + speed <= MAX_MS)
        {
            esc.writeMicroseconds(base_speed + speed);
        }
    }
    else
    {
        if (base_speed + speed >= MIN_MS)
        {
            esc.writeMicroseconds(base_speed - speed);
        }
    }
}

void ESC::setRunningMode(int mode)
{
    if (mode == MODE_FORWARD)
    {
        base_speed = BASE_FORWARD_MS;
    }
    else
    {
        if (mode == MODE_FORWARD_BACKWARD)
        {
            base_speed = BASE_FORWARD_BACKWARD_MS;
        }
    }
}

void ESC::setDirection(int direction)
{
    if (direction == FORWARD) {
        dir = FORWARD;
    } else {
        dir = BACKWARD;
    }
}

void ESC::detach() {
    esc.detach();
}

void ESC::attach(int pin) {
    esc.attach(pin);
}

#endif