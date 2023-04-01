# MiniDroneV1
Small drone powered by an ESP32, and controlled by a web server in hopes of a future fully autonomous drone.

The drone is a quadcopter containing:
  - 4 2206 2400KV Motors
  - 4 15a ESCs
  - 1 ESP32 Minikit
  - 1 2s Lipo Battery
  - 1 Adafruit BNO055 Breakout Board
  - 1 Adafruit MPL3115A2 breakout Board
  - 1 Buzzer
 
 The BNO055 and MPL3115A2 are connected via I2C to the ESP32
 
 A voltage divider is also present containing a 2.2K and 1.2K resistor
 
 The website contains 3 main pages:
 This is the main control panel
 *NOTE: The throttle Slider is only present for now just to hover the drone
 ![Screenshot 2023-04-01 070607](https://user-images.githubusercontent.com/129092528/229294071-03cda362-ae69-4ff9-810b-69a6dc2b54f2.png)
 This is the page containing sensor data of the gyro and barometer
 ![Screenshot 2023-04-01 070635](https://user-images.githubusercontent.com/129092528/229294196-ca99a6e9-e574-42e3-8cbe-2aa2d15b07f5.png)
 And this page is for PID Tuning, the arrows on the side are increasing/decreasing the multiplier for ALL PID Gains
 ![Screenshot 2023-04-01 070658](https://user-images.githubusercontent.com/129092528/229294274-1a4cc9b1-b07e-491f-9584-3deefec1eb9a.png)
 
 The one thing not shown is the calibrate gyro page, which prompts the user whether they want to calibrate the gyro, then shows status and instruction messages of whats going on before resetting the ESP32.
