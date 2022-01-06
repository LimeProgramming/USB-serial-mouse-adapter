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

# KiCad
The KiCad files and Gerber files are publicly available in the KiCad folder. Two variants are available:
| Differences| Phat | Slim |
|:--|:--:|:--:|
| USB Pin Header | 🟢 | ❌ |
| Dip Switches | 🟢 | ❌ |
| Serial Pin Headers | 🟢 | ❌ |
| Size  | ❌  | 🟢 |


# Compiling it yourself
The default mouse settings can be edited in `default_config.h` prior to compilation. You can use this to set the mouses default settings for your own configuration. 

**What you need:**
|  |  |
|--|--|
| PicoSDK | 1.2.0 |

The version of TinyUSB included with the newest version of the PicoSDK doesn't work great. You could probably use TinyUSB v10.1 on the newest PicoSDK but that's a bit messy.
