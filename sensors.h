#ifndef SENSORS_H
#define SENSORS_H

#include "Arduino.h"
#include "networking.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <Adafruit_MPL3115A2.h>
#include <utility/imumaths.h>
#include <Arduino_JSON.h>
#include <EEPROM.h>

#define EEPROM_SIZE 4096

JSONVar readings;
Network net;

class Sensors;

class Sensors
{
private:
    Adafruit_MPL3115A2 mpl;

    sensor_t sensor;

    void displayCalStatus();
    void displaySensorOffsets(const adafruit_bno055_offsets_t &);

public:
    Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire);

    float quat_x, quat_y, quat_z, quat_w;
    float euler_x, euler_y, euler_z;
    float gyro_yaw;

    int eeAddress = 0;
    long bnoID;
    bool foundCalib = false;

    String gyro_json;
    String baro_json;
    String calibration_json;

    void initSensors();
    void getGyroReadings(bool);
    void getBaroReadings();
    void calibrateBNO();
};

void Sensors::initSensors()
{
    EEPROM.begin(EEPROM_SIZE);

    Serial.println("Initializing Sensors...");
    if (!bno.begin())
    {
        Serial.print("Error Connecting To Gyroscope. Please Connect to Continue.");
        while (1)
            ;
    }

    EEPROM.get(eeAddress, bnoID);

    adafruit_bno055_offsets_t calibrationData;

    bno.getSensor(&sensor);
    if (bnoID != sensor.sensor_id)
    {
        Serial.println("No Calibration Data Stored in EEPROM");
        delay(500);
    }
    else
    {
        Serial.println("Found Calibration Data For This Sensor");
        eeAddress += sizeof(long);
        EEPROM.get(eeAddress, calibrationData);

        displaySensorOffsets(calibrationData);

        Serial.println("Restoring Data To BNO055...");
        bno.setSensorOffsets(calibrationData);

        Serial.println("Done.");
        foundCalib = true;
    }

    bno.setExtCrystalUse(true);
    bno.setMode(OPERATION_MODE_IMUPLUS);
    Serial.println("Gyroscope Initialized Successfully");

    Serial.println("Initializing Barometer...");
    if (!mpl.begin())
    {
        Serial.println("Error Connecting To Barometer. Please Connect to Continue.");
        while (1)
            ;
    }
    mpl.setSeaPressure(1016); // Setting the current location pressure
    mpl.write8(0x26, 16);        // How Fast and accurate the barometer should be
    Serial.println("Barometer Initialized Successfully");
}

void Sensors::getGyroReadings(bool quat)
{
    imu::Vector<3> gyro = bno.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);
    gyro_yaw = gyro.z();

    if (!quat)
    {
        imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);

        euler_x = euler.x();
        euler_y = euler.y();
        euler_z = euler.z();

        readings["eulerX"] = String(euler.x());
        readings["eulerY"] = String(euler.y());
        readings["eulerZ"] = String(euler.z());
    }
    else
    {
        imu::Quaternion quat = bno.getQuat();

        quat_w = quat.w();
        quat_x = quat.x();
        quat_y = quat.y();
        quat_z = quat.z();

        readings["quatW"] = String(quat.w());
        readings["quatX"] = String(quat.x());
        readings["quatY"] = String(quat.y());
        readings["quatZ"] = String(quat.z());
    }
    gyro_json = JSON.stringify(readings);
}

void Sensors::getBaroReadings()
{
    float pressure = mpl.getPressure();
    float altitude = mpl.getAltitude();
    float temp = mpl.getTemperature();

    readings["PRESS"] = String(pressure);
    readings["ALT"] = String(altitude);
    readings["TEMP"] = String(temp);

    baro_json = JSON.stringify(readings);
}

void Sensors::displayCalStatus()
{
    uint8_t system, gyro, accel, mag;
    bno.getCalibration(&system, &gyro, &accel, &mag);

    Serial.print("System: ");
    Serial.println(system);
    Serial.print("Gyro: ");
    Serial.println(gyro);
    Serial.print("Accel: ");
    Serial.println(accel);
    Serial.print("Mag: ");
    Serial.println(mag);
}

void Sensors::displaySensorOffsets(const adafruit_bno055_offsets_t &calibData)
{
    Serial.println("Accelerometer: ");
    Serial.print(calibData.accel_offset_x);
    Serial.print(" ");
    Serial.print(calibData.accel_offset_y);
    Serial.print(" ");
    Serial.println(calibData.accel_offset_z);

    Serial.println("Gyro: ");
    Serial.print(calibData.gyro_offset_x);
    Serial.print(" ");
    Serial.print(calibData.gyro_offset_y);
    Serial.print(" ");
    Serial.println(calibData.gyro_offset_z);

    Serial.println("Mag: ");
    Serial.print(calibData.mag_offset_x);
    Serial.print(" ");
    Serial.print(calibData.mag_offset_y);
    Serial.print(" ");
    Serial.println(calibData.mag_offset_z);

    Serial.print("Accel Radius: ");
    Serial.println(calibData.accel_radius);

    Serial.print("Mag Radius: ");
    Serial.println(calibData.mag_radius);
}

void Sensors::calibrateBNO()
{
    int stage = 0;
    String stage_json;

    sensors_event_t event;
    bno.getEvent(&event);

    Serial.println("Calibrating Sensor...");
    stage++;
    readings["CALIBDAT"] = String(stage);
    stage_json = JSON.stringify(readings);
    net.sendStageOfCalib(stage_json);

    while (!bno.isFullyCalibrated())
    {
        bno.getEvent(&event);

        displayCalStatus();
    }

    Serial.println("Please Place Drone on a Flat Surface...");

    stage++;
    readings["CALIBDAT"] = String(stage);
    stage_json = JSON.stringify(readings);
    net.sendStageOfCalib(stage_json);

    delay(5000);

    Serial.println("Calculating Offsets...");

    stage++;
    readings["CALIBDAT"] = String(stage);
    stage_json = JSON.stringify(readings);
    net.sendStageOfCalib(stage_json);

    imu::Vector<3> gyro_sum = {0, 0, 0};
    imu::Vector<3> accel_sum = {0, 0, 0};
    int num_cycles = 500;
    for (int i = 0; i < num_cycles; i++)
    {
        imu::Vector<3> gyro_readings = bno.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);
        imu::Vector<3> accel_readings = bno.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER);
        gyro_sum.x() = gyro_sum.x() + gyro_readings.x();
        gyro_sum.y() = gyro_sum.y() + gyro_readings.y();
        gyro_sum.z() = gyro_sum.z() + gyro_readings.z();
        accel_sum.x() = accel_sum.x() + accel_readings.x();
        accel_sum.y() = accel_sum.y() + accel_readings.y();
        accel_sum.z() = accel_sum.z() + accel_readings.z();
        delay(10);
    }
    imu::Vector<3> gyro_mean;
    imu::Vector<3> accel_mean;
    gyro_mean.x() = gyro_sum.x() / num_cycles;
    gyro_mean.y() = gyro_sum.y() / num_cycles;
    gyro_mean.z() = gyro_sum.z() / num_cycles;
    accel_mean.x() = accel_sum.x() / num_cycles;
    accel_mean.y() = accel_sum.y() / num_cycles;
    accel_mean.z() = accel_sum.z() / num_cycles;

    Serial.println("Calibration Complete");

    stage++;
    readings["CALIBDAT"] = String(stage);
    stage_json = JSON.stringify(readings);
    net.sendStageOfCalib(stage_json);

    Serial.println("-------------------------------------");
    Serial.println("Calibration Results: ");
    adafruit_bno055_offsets_t newCalib;
    bno.getSensorOffsets(newCalib);
    newCalib.accel_offset_x += -accel_mean.x();
    newCalib.accel_offset_y += -accel_mean.y();
    newCalib.accel_offset_z += -accel_mean.z();
    newCalib.gyro_offset_x += -gyro_mean.x();
    newCalib.gyro_offset_y += -gyro_mean.y();
    newCalib.gyro_offset_z += -gyro_mean.z();
    displaySensorOffsets(newCalib);

    Serial.println("Storing Calibration Data To EEPROM...");

    stage++;
    readings["CALIBDAT"] = String(stage);
    stage_json = JSON.stringify(readings);
    net.sendStageOfCalib(readings);

    eeAddress = 0;
    bno.getSensor(&sensor);
    bnoID = sensor.sensor_id;

    EEPROM.put(eeAddress, bnoID);

    eeAddress += sizeof(long);
    EEPROM.put(eeAddress, newCalib);
    EEPROM.commit();
    Serial.println("Data Stored To EEPROM");

    delay(1000);

    stage++;
    readings["CALIBDAT"] = String(stage);
    stage_json = JSON.stringify(readings);
    net.sendStageOfCalib(readings);

    Serial.println("\n-------------------------------------\n");
    Serial.println("Resetting...");
    delay(5000);
    
    ESP.restart();
}

#endif