# USB XT Keyboard converter
So this is a thing to allow you to plug a USB keyboard into an XT computer.


It's run on any arduino ish MCU, connected to a CH559 running this [ch559 usb host](https://github.com/mtfurlan/ch559-usb-host) software.

## Code overview
We get keyboard USB HID data from the CH559, and do magic lookup bullshit to convert it to XT codes, which we bitbang out.

## Wiring
There is a voltage shifter between the trinket and the esp.
| esp     | XT color | XT purpose  |
| ------- | -------- | ----------  |
| D2      | white    | 1 clock     |
| VUSB    | red      | 5 5V        |
| N/C     | yellow   | 3           |
| D0      | black    | 2 data      |
| gnd     | shield   | 4/shell gnd |

## Next Steps
* Figure out what XT keyboards do on boot, because this isn't detected on boot
* Rewrite the entire conversion thing.
  It has so many edge cases, and really shows it was the result of half a weekend of work.
* I bet it wouldn't be hard to do AT as well.
