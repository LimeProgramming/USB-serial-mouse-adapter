# USB Serial Mouse Adapter
A USB to serial mouse adapter!
![Assembled adaptor](https://raw.githubusercontent.com/LimeProgramming/USB-serial-mouse-adapter/main/images/assembled_small.jpg)
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

**Unsupported Protocols:**
- Mousesystems
- Sun
- MM
Note: support for these can be added if the demand is there. 

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

# KiCad
The KiCad files and Gerber files are publicly available in the KiCad folder. Two variants are available:
| Differences| Phat | Slim |
|:--|:--:|:--:|
| USB Pin Header | 🟢 | ❌ |
| Dip Switches | 🟢 | ❌ |
| Serial Pin Headers | 🟢 | ❌ |
| Size  | ❌  | 🟢 |


# Compiling it yourself

## PicoSDK
Use the 1.2.0 version of the Pico SDK with TinyUSB version 10.1
