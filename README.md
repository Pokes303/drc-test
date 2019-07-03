# drc-test
A WiiU homebrew/app to test the DRC and read/write some hidden values

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
### ·HBL Version
Take the contents of `<drctest hbl.zip>` here: [https://github.com/Pokes303/drc-test/releases/latest] and place them on your WiiU SD Card at sd:/wiiu/apps/

### ·Channel Version
Take the contents of `<drctest channel.zip>` here: [https://github.com/Pokes303/drc-test/releases/latest], then move the folder `<drctest channel>` to sd:/install/
Put the SD Card on your WiiU and install the contents with WUP Installer GX2

## Compiling
On Linux or WSL, download and install devkitPro's wut package: [https://github.com/devkitPro/wut] and follow the initial instructions for compiling projects (No extra libraries are needed), then clone the repository, unzip the files and write on the command line:
```
cd drc-test-master
make
```
