# Glia ECG project
gliax-ecg-android-application


www.irnas.eu

## Basic Information
Android app for displaying ECG signals (with other features, such as: pdf saving, patient input, etc.).

Made for tablets, tested on Huawei MediaPad M5 10, running Android (EMUI) 8 and 9.

App is using USB protocol to communicate with ECG board (STM32 MCU with TI chip and SL 2102x converter).

Using modified version of usb-serial-for-android library by mik3y (https://github.com/mik3y/usb-serial-for-android).

To build this app, NDK version 12 is needed (r12b used).

## Instructions for users
### Installing procedure
1. Download the latest release file (GliaECG-release-_._.apk)
2. Load it directly onto tablet or on USB drive (plug it into tablet via USB adapter)
3. Tap the file and click install
4. Application icon should appear in the android system

### Using the app
* In order to use this app, the following permissions need to be granted:
    * USB device access (to be able to connect to ECG board),
    * Automatically open GliaECG app when ECG board gets connected (to properly setup and terminate communication with it),
    * Write external storage (used for saving pdf files).
* ECG board is running when two red LEDs are blinking. If power gets reset (charging cable pluged in, tablet turned off), the board will start running. It will stop once the app has been exited once again.
* Tablet needs to be restarted if the application doesn't start drawing signals (HR 000 bpm appears on screen). This can happen if the USB connection stopped working when signals were being drawn.
* Advanced settings can be reached by pressing Patient button.
* About button is located on the settings popup.
