# Change Log

## Version 1.0.0
Initial Release.

## Version 1.1.1
- Added KVM and USB hub support
- Added different mouse movement types
- Added "cosine mouse smoothing"
- Small bug fixes
- Latest version of the Pico-SDK
- Latest but modified version of TinyUSB

## Version 1.2.2
- Modified terminal code to be easier to translate.
- German language translation added to serial terminal.
- Multicore Processing. The pi pico has two cores baked into it so might as well use them. The second core is currently dedicated to transmitting the serial mouse data over UART and the first core is dedicated to handing the USB mouse, plus the serial terminal. This should keep the report rate consistant while keeping lag to a minimum.
- Nand wear leveling for saving configuration. Should extend the life of the Pico 200 fold.
- Added Support for 19200 baud rate.
- Changed the Dip Switch 6 to toggle between 19200 and 1200 baud rates.
- PWR LED now flashes while the adapater is in serial terminal mode.
- While adpater is in serial terminal mode changes to the dipswitches are ignored. This was an easy way to ruin your day before!
- Changed how Coasting movement type worked. Should feel the same but work more consistantly.

