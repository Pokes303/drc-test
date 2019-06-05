# drc-test
A WiiU homebrew/app for testing the DRC and read/write some hidden values

## Features for testing
- Buttons/Joysticks
- Screen/Touchscreen
- Gyroscope
- Accelerometer
- Direction
- Angle
- Magnetometer
- Volume
- Battery
- Vibration
- Gamepad Sensor Bar
- Gamepad BASE

## Usage
Take the contents of drc-test-hbl.zip on releases tab and place them on your WiiU SD Card at sd:/wiiu/apps/drc-test
## Compiling
On Linux or WSL, download and install (devkitPro's wut package)[https://github.com/devkitPro/wut], then clone the repository, unzip the files, and write on the command line:
```
cd drc-test-master
make
```
