# USB XT Keyboard converter
So I was at VCFMW 2021, and our booth had a handful of XT computers and somewhere between 0 and 1 XT keyboard.
We were able to purchase an XT/AT keyboard from a very nice person, but we still needed more.

So this is a thing to allow you to plug a USB keyboard into an XT computer.


It's run on a [Adafruit trinket M0](https://www.adafruit.com/product/3500).

## Code overview
The basic way it works is we have a USB host mode thing that can read keycodes from the USB keyboard.
We then pass the USB codes through some lookup tables to try to make XT codes, and then we bitbang the XT codes out.

## Wiring
There is a voltage shifter between the trinket and the XT.
| trinket | XT color | XT purpose  |
| ------- | -------- | ----------  |
| D2      | white    | 1 clock     |
| VUSB    | red      | 5 5V        |
| N/C     | yellow   | 3           |
| D0      | black    | 2 data      |
| gnd     | shield   | 4/shell gnd |

D3/D4 are the programming pins

We use the onboard dotstar RGB light for debugging.


## Next Steps
* Figure out what XT keyboards do on boot, because this isn't detected on boot
* Rewrite the entire conversion thing.
  It has so many edge cases, and really shows it was the result of half a weekend of work.
* I bet it wouldn't be hard to do AT as well.
