#ifndef BUZZER_H
#define BUZZER_H

#include "Arduino.h"

class Buzzer;

class Buzzer
{
private:
    const int buzzer_pin = 19;

    const int startup_melody[3] = {235, 345, 455};
    const int calibrated_melody[3] = {200, 200, 200};

    int current_note = 0;
    int previous_time = 0;

    const long noteDuration_startup = 400;
    const long noteDuration_calibrated = 200;

    const long calibrateTone_duration = 500;

public:
    bool init();
    void playStartupTone();
    void playCalibrateTone();
    void playCalibratedTone();
};

bool Buzzer::init()
{
    pinMode(buzzer_pin, OUTPUT);
    return true;
}

void Buzzer::playStartupTone()
{
    bool finished = false;
    while (!finished)
    {
        unsigned long current_time = millis();

        if (current_time - previous_time >= noteDuration_startup)
        {
            if (current_note < sizeof(startup_melody) / sizeof(startup_melody[0]))
            {
                tone(buzzer_pin, startup_melody[current_note], noteDuration_startup);
                previous_time = current_time;
                current_note++;
            }
            else
            {
                noTone(buzzer_pin);
                current_note = 0;
                finished = true;
            }
        }
    }
}

void Buzzer::playCalibratedTone()
{
    bool finished = false;
    while (!finished)
    {
        unsigned long current_time = millis();

        if (current_time - previous_time >= noteDuration_calibrated)
        {
            if (current_note < sizeof(calibrated_melody) / sizeof(calibrated_melody[0]))
            {
                tone(buzzer_pin, calibrated_melody[current_note], 300);
                previous_time = current_time;
                current_note++;
            }
            else
            {
                noTone(buzzer_pin);
                current_note = 0;
                finished = true;
            }
        }
    }
}

void Buzzer::playCalibrateTone()
{
    unsigned long current_time = millis();

    if (current_time - previous_time >= calibrateTone_duration)
    {
        tone(buzzer_pin, 200, 20);
        previous_time = current_time;
    }
    else
    {
        noTone(buzzer_pin);
    }
}

#endif