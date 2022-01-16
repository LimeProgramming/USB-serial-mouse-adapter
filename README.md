# USB Serial Mouse Adapter
A USB to serial mouse adapter!
![Assembled adaptor](https://raw.githubusercontent.com/LimeProgramming/USB-serial-mouse-adapter/main/images/assembled_small.jpg)
## Important
I'm writing this readme before the newest firmware version comes out. 
Wait until this message goes away before considering this readme valid.

## About
This project came about when I got my hands on an old 486 based PC. Having never really dipped my toes into retro computing, I didn't have an old serial mouse on hand. A quick eBay searched revealed that the decent ones cost a nice chunk of change that I didn't feel they were worth. Another search on Vogons revealed that some mad lads out there made active PS/2 to serial adapters using various micro controllers.  
Out of curiosity I examined how these worked and thought "Why can't I do this with a Pi Pico and USB mouse?" which kicked off this project. 

I hope it sparks your interest!
-Lime

## Features
With this adapter you can convert a modern USB mouse to a classic serial mouse. Want to use an optical mouse on a 386 running Wolfenstein 3D? Well now you can! Want to use a wireless optical mouse on a 486 running Doom? Well you might be able to! (depending on the mouse)

**Supported Protocols:**
- Microsoft Two Button
- Logitech Three Button
- Microsoft Wheel

**Unsupported Protocols: (May change based on demand)**
- Mousesystems
- Sun
- MM

### Settings 
The adapter has quite a few settings available to it and these settings are stored on the  pi picos memory. It will remember your settings between uses and the same settings carry between different computers and even firmware upgrades (unless otherwise state in a changes log) Should you need to clear these settings you will need to blank the pico with the nuke.uf2 file and reinstalling the firmware. An easier way is planned but not implemented yet.

 **Dip Switch Settings**
| Num | Setting |
|:--:|:--:|
| 1 | Three Button Logitech Protocol |
| 2 | MS Wheel Protocol |
| 3 | 75% Mouse Travel Modifier / Dip 3 + 4 for 25% |
| 4 | 50% Mouse Travel Modifier / Dip 3 + 4 for 25% |
| 5 | 7N2|
| 6 | 2400 Baud Rate|

<br>

| Additional Available Settings |
|:--:|
| Swap left and right mouse buttons  |
| Use Forward and Back mouse buttons as alternate left and right |
| Independently swap forward and backward buttons |
| XY mouse travel modifier 1% -> 200% |
| X mouse  travel modifier 1% -> 200% |
| y mouse  travel modifier 1% -> 200% |
| One (7N1) or Two (7N2) stop bits  |
| 1200 2400 4800 9600 Baud Rates |

# Configuration
There are two primary ways of editing the settings of the USB-2-232

### Headers
The phat variant of the PCB includes headers in either of the form of dip switches or jumper headers. While not super configurable they do give quick access to some handy settings. The headers can be set while the micro controller is on with changes being applied in real-time or they can be set while the micro controller is off with changes being applied next time the controller is booted.

| Num | Setting |
|:--:|:--:|
| 1 | Three Button Logitech Protocol |
| 2 | MS Wheel Protocol |
| 3 | 75% Mouse Travel Modifier / Dip 3 + 4 for 25% |
| 4 | 50% Mouse Travel Modifier / Dip 3 + 4 for 25% |
| 5 | 7N2|
| 6 | 2400 Baud Rate|

### Serial Terminal
![serial terminal gif](https://raw.githubusercontent.com/LimeProgramming/USB-serial-mouse-adapter/main/images/serial_term.gif)
Both the phat and slim variants of the PCB support being configured via serial terminal. The serial terminal allows for more advanced configuration of the mouse adapter. Any additional settings added in a software update will be available here.

### Serial Terminal Options
+ Mouse Travel
    + List Config
            | List the current mouse travel settings
    + XY Travel 
            | XY mouse travel modifier 1% -> 200%
    + X Travel
             | X (Left and Right) mouse travel modifier 1% -> 200%
    + Y Travel 
            | Y ( Up and Down) mouse travel modifier 1% -> 200%
    + Invert X
            | Invert X (Left and Right) mouse movement.
    + Invert Y
            | Invert Y (Up and Down) mouse movement.
+ Mouse Buttons
    + List Config
        | List the current mouse button settings
    + Swap Left and Right
        | Swap the left and right mouse buttons
    + Use Forward and Backward
        | Use the mouse forward and back buttons as alternative left and right buttons
    + Swap Forward and Backward
        | Swap forward and back buttons if they are being used as alternative left and right buttons
+ Serial Settings
    + List Config
        | List the current serial settings
    + Format
        | One stop bit (7N1) or two stop bits (7N2) 
    + Baud Rate
        | 1200 2400 4800 9600 Baud Rates. Note you will need compatible mouse drivers.
     + Mouse Type
     | Microsoft 2 Button, Logitech Three Button or Microsoft Wheel mouse. Note: You may need to reconfigure your mouse driver upon change.
+ Firmware
    + Information | Information about the firmware
    + Reset | Reset mouse settings back to default
    + List Default Settings | List the default settings of the adapter. 
+ Exit
    + Return to mouse mode

### Accessing the terminal
You can access the serial terminal in one of two ways:

**Method 1: Serial Terminal Emulator**
You can open a serial terminal emulator (like kermit) on the computer you have the mouse connected to, connect to the same com port the mouse driver is using and press the enter key.
```
Baud Rate: User Set
Data Bits: 7
Stop Bits: User Set
Parity: None
Flow Control: None
```
**Method 2: USB to TTL**
You can also connect the unused UART of the Pi Pico with a cheap USB to TTL device or even another Pico running the picoprobe firmware. Open your favorite serial terminal emulator (like putty) connect to the TTL device and tap the enter key. 
```
Baud Rate: 115200
Data Bits: 8
Stop Bits: 1
Parity: None
Flow Control: None
```
Connecting to the adapter in this way will allow you to change settings regardless of the adapter being connected to a PC or not. 
The unused UART of the pico can be accessed from the top two pins between the full size USB port of the PCB and the micro USB port of the Pico.

# KiCad
There are two variants of the PCB available in the KiCad folder, Phat and Slim for your preference. The main circuit for both PCBs is the same so the firmware package will work on both PCBs.
Check the renders from KiCad and the table below to see the differences!

| Differences| Phat | Slim |
|:--|:--:|:--:|
| USB Pin Header | 🟢 | ❌ |
| Dip Switches | 🟢 | ❌ |
| Serial Pin Headers | 🟢 | ❌ |
| Size  | ❌  | 🟢 |


# Compiling the Firmware
The default mouse settings can be edited in `default_config.h` prior to compilation. You can use this to set the mouses default settings for your own configuration. 

The build environment is a normal PicoSDK setup, there are several guides out the for both Linux and Windows.  From experience the setup is easier in Linux. However there is one important thing you need to know:

**PicoSDK version 1.2.0**
The version of TinyUSB included with the newest version of the PicoSDK doesn't work great. You could probably use TinyUSB v10.1 on the newest PicoSDK but that's a bit messy.


