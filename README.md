# USB Serial Mouse Adapter
A USB to serial mouse adapter!
![Assembled adaptor](https://raw.githubusercontent.com/LimeProgramming/USB-serial-mouse-adapter/main/images/assembled_small.jpg)
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

