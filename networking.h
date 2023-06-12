#ifndef NETWORKING_H
#define NETWORKING_H

#include "Arduino.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <SPIFFS.h>

#define BNO055_SAMPLERATE_MS (10)
#define MPL_SAMPLERATE_MS (10)
#define SAMPLE_RATE (10)
#define BATT_SAMPLE_RATE (1000)

AsyncWebServer server(80);
AsyncEventSource events("/events");

int bldc[4] = {0, 0, 0, 0};

extern float pitch_gains[3];
extern float roll_gains[3];
extern float yaw_gains[3];

int pid_roll_mult_amt = 0;
int pid_yaw_mult_amt = 0;

bool calibrateSensors = false;

class Network;

class Network
{
private:
    const char *ssid = "DRONE_NET";
    const char *password = "password";

    unsigned long last_time_gyro = 0;
    unsigned long last_time_baro = 0;
    unsigned long last_time_pid = 0;
    unsigned long last_time_calib = 0;
    unsigned long last_time_vbatt = 0;

public:
    void initNetwork();
    void startNetwork();
    void sendData(String, String, String, String);
    void sendStageOfCalib(String);
};

void Network::initNetwork()
{
    // Initializes the Wifi Access Point
    Serial.println("Setting Up Wifi Access Point...");
    WiFi.softAP(ssid, password, 1, 1, 1);
    Serial.println("Access Point Created.");

    IPAddress IP = WiFi.softAPIP();
    Serial.print("IP Address: ");
    Serial.println(IP);

    // Initializes SPIFFS
    Serial.println("Initializing SPIFFS...");
    if (!SPIFFS.begin())
    {
        Serial.println("Error Mounting SPIFFS");
        while (1)
            ;
    }
    Serial.println("SPIFFS Mounted Successfully");
}

void Network::startNetwork()
{
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/index.html", "text/html"); });

    // Throttle Slider
    server.on("/throttle", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        String throttleInputMessage;
        if (request->hasParam("value")) {
            throttleInputMessage = request->getParam("value")->value();
            for (int i = 0; i < 4; i++) {
                bldc[i] = throttleInputMessage.toInt();
                //Serial.println(throttleInputMessage.toInt());
            }
        } else {
            throttleInputMessage = "No message sent";
        } 
        request->send(200, "text/plain", "OK"); });

    // PID Tuning
    // Set Multiplier Amount For PID Tuning
    server.on("/rollMultinc", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        pid_roll_mult_amt++;
        if (pid_roll_mult_amt > 2) {
            pid_roll_mult_amt = 2;
        }
        request->send(200, "text/plain", "OK"); });
    server.on("/rollMultdec", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        pid_roll_mult_amt--;
        if (pid_roll_mult_amt < 0) {
            pid_roll_mult_amt = 0;
        }
        request->send(200, "text/plain", "OK"); });
    server.on("/yawMultinc", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        pid_yaw_mult_amt++;
        if (pid_yaw_mult_amt > 2) {
            pid_yaw_mult_amt = 2;
        }
        request->send(200, "text/plain", "OK"); });
    server.on("/yawMultdec", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        pid_yaw_mult_amt--;
        if (pid_yaw_mult_amt < 0) {
            pid_yaw_mult_amt = 0;
        }
        request->send(200, "text/plain", "OK"); });

    // Pitch / Roll
    server.on("/Ppitchrollinc", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        if (pid_roll_mult_amt == 0) {
            pitch_gains[0] += 0.01;
            roll_gains[0] += 0.01;
        }
        if (pid_roll_mult_amt == 1) {
            pitch_gains[0] += 0.1;
            roll_gains[0] += 0.1;
        }
        if (pid_roll_mult_amt == 2) {
            pitch_gains[0] += 1.0;
            roll_gains[0] += 1.0;
        }
        request->send(200, "text/plain", "OK"); });
    server.on("/Ipitchrollinc", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        if (pid_roll_mult_amt == 0) {
            pitch_gains[1] += 0.01;
            roll_gains[1] += 0.01;
        }
        if (pid_roll_mult_amt == 1) {
            pitch_gains[1] += 0.1;
            roll_gains[1] += 0.1;
        }
        if (pid_roll_mult_amt == 2) {
            pitch_gains[1] += 1.0;
            roll_gains[1] += 1.0;
        }      
        request->send(200, "text/plain", "OK"); });
    server.on("/Dpitchrollinc", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        if (pid_roll_mult_amt == 0) {
            pitch_gains[2] += 0.01;
            roll_gains[2] += 0.01;
        }
        if (pid_roll_mult_amt == 1) {
            pitch_gains[2] += 0.1;
            roll_gains[2] += 0.1;
        }
        if (pid_roll_mult_amt == 2) {
            pitch_gains[2] += 1.0;
            roll_gains[2] += 1.0;
        }       
        request->send(200, "text/plain", "OK"); });
    server.on("/Ppitchrolldec", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        if (pid_roll_mult_amt == 0) {
            pitch_gains[0] -= 0.01;
            roll_gains[0] -= 0.01;
        }
        if (pid_roll_mult_amt == 1) {
            pitch_gains[0] -= 0.1;
            roll_gains[0] -= 0.1;
        }
        if (pid_roll_mult_amt == 2) {
            pitch_gains[0] -= 1.0;
            roll_gains[0] -= 1.0;
        }        
        request->send(200, "text/plain", "OK"); });
    server.on("/Ipitchrolldec", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        if (pid_roll_mult_amt == 0) {
            pitch_gains[1] -= 0.01;
            roll_gains[1] -= 0.01;
        }
        if (pid_roll_mult_amt == 1) {
            pitch_gains[1] -= 0.1;
            roll_gains[1] -= 0.1;
        }
        if (pid_roll_mult_amt == 2) {
            pitch_gains[1] -= 1.0;
            roll_gains[1] -= 1.0;
        }        
        request->send(200, "text/plain", "OK"); });
    server.on("/Dpitchrolldec", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        if (pid_roll_mult_amt == 0) {
            pitch_gains[2] -= 0.01;
            roll_gains[2] -= 0.01;
        }
        if (pid_roll_mult_amt == 1) {
            pitch_gains[2] -= 0.1;
            roll_gains[2] -= 0.1;
        }
        if (pid_roll_mult_amt == 2) {
            pitch_gains[2] -= 1.0;
            roll_gains[2] -= 1.0;
        }        
        request->send(200, "text/plain", "OK"); });

    // Yaw
    server.on("/Pyawinc", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        if (pid_yaw_mult_amt == 0) {
            yaw_gains[0] += 0.01;
        }
        if (pid_yaw_mult_amt == 1) {
            yaw_gains[0] += 0.1;
        }
        if (pid_yaw_mult_amt == 2) {
            yaw_gains[0] += 1.0;
        }        
        request->send(200, "text/plain", "OK"); });
    server.on("/Iyawinc", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        if (pid_yaw_mult_amt == 0) {
            yaw_gains[1] += 0.01;
        }
        if (pid_yaw_mult_amt == 1) {
            yaw_gains[1] += 0.1;
        }
        if (pid_yaw_mult_amt == 2) {
            yaw_gains[1] += 1.0;
        }       
        request->send(200, "text/plain", "OK"); });
    server.on("/Dyawinc", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        if (pid_yaw_mult_amt == 0) {
            yaw_gains[2] += 0.01;
        }
        if (pid_yaw_mult_amt == 1) {
            yaw_gains[2] += 0.1;
        }
        if (pid_yaw_mult_amt == 2) {
            yaw_gains[2] += 1.0;
        }        
        request->send(200, "text/plain", "OK"); });
    server.on("/Pyawdec", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        if (pid_yaw_mult_amt == 0) {
            yaw_gains[0] -= 0.01;
        }
        if (pid_yaw_mult_amt == 1) {
            yaw_gains[0] -= 0.1;
        }
        if (pid_yaw_mult_amt == 2) {
            yaw_gains[0] -= 1.0;
        }        
        request->send(200, "text/plain", "OK"); });
    server.on("/Iyawdec", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        if (pid_yaw_mult_amt == 0) {
            yaw_gains[1] -= 0.01;
        }
        if (pid_yaw_mult_amt == 1) {
            yaw_gains[1] -= 0.1;
        }
        if (pid_yaw_mult_amt == 2) {
            yaw_gains[1] -= 1.0;
        }         
        request->send(200, "text/plain", "OK"); });
    server.on("/Dyawdec", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        if (pid_yaw_mult_amt == 0) {
            yaw_gains[2] -= 0.01;
        }
        if (pid_yaw_mult_amt == 1) {
            yaw_gains[2] -= 0.1;
        }
        if (pid_yaw_mult_amt == 2) {
            yaw_gains[2] -= 1.0;
        }        
        request->send(200, "text/plain", "OK"); });

    // Calibrations
    server.on("/yescalibrate", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        calibrateSensors = true;        
        request->send(200, "text/plain", "OK"); });

    server.serveStatic("/", SPIFFS, "/");

    events.onConnect([](AsyncEventSourceClient *client)
                     {
        if (client->lastId()) {
            Serial.printf("Client Reconnected, Last message ID that it got is: %u\n", client->lastId());
        }
        client->send("Hello", NULL, millis(), 10000); });

    server.addHandler(&events);

    server.begin();
}

void Network::sendData(String GyroData, String BaroData, String PIDGains, String VBatt)
{
    if ((millis() - last_time_gyro) > BNO055_SAMPLERATE_MS)
    {
        events.send(GyroData.c_str(), "gyro_readings", millis());
        last_time_gyro = millis();
    }
    if ((millis() - last_time_baro) > MPL_SAMPLERATE_MS)
    {
        events.send(BaroData.c_str(), "baro_readings", millis());
        last_time_baro = millis();
    }
    if ((millis() - last_time_pid) > SAMPLE_RATE)
    {
        events.send(PIDGains.c_str(), "pid_gains", millis());
        last_time_pid = millis();
    }
    if ((millis() - last_time_vbatt) > BATT_SAMPLE_RATE)
    {
        events.send(VBatt.c_str(), "battery_voltage", millis());
        last_time_vbatt = millis();
    }
}

void Network::sendStageOfCalib(String stage)
{
    events.send(stage.c_str(), "calib_stage", millis());
}

#endif