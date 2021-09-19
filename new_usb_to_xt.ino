#include "xt_defines.h"

#define DEBUG false

// On SAMD boards where the native USB port is also the serial console, use
// Serial1 for the serial console. This applies to all SAMD boards except for
// Arduino Zero and M0 boards.
#if (USB_VID==0x2341 && defined(ARDUINO_SAMD_ZERO)) || (USB_VID==0x2a03 && defined(ARDUINO_SAM_ZERO))
#define Serial SERIAL_PORT_MONITOR
#else
#define Serial Serial1
#endif

#define xt_data 0
#define xt_clk 2

#include <Adafruit_DotStar.h>
#include <SPI.h>

#define NUMPIXELS 1 // Number of LEDs in strip

// Here's how to control the LEDs from any two pins:
#define DATAPIN   7
#define CLOCKPIN   8

Adafruit_DotStar strip = Adafruit_DotStar(
  NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BGR);


// Require keyboard control library
#include <KeyboardController.h>

// Initialize USB Controller
USBHost usb;


class KeyboardRaw : public KeyboardReportParser {
public:
  KeyboardRaw(USBHost &usb) : hostKeyboard(&usb) {
    hostKeyboard.SetReportParser(0, this);
  };

  void Parse(HID *hid, uint32_t is_rpt_id, uint32_t len, uint8_t *buf);

private:
  HIDBoot<HID_PROTOCOL_KEYBOARD> hostKeyboard;
};

bool canConvertUSB2XT(uint8_t usbScanCode) {
  return usbScanCode <= 69; //TODO: figure it out
}
void xt_write(unsigned char value)
{
   //while (digitalRead(xt_clk) != HIGH) ;
   unsigned char bits[8] ;
   byte p = 0 ;
   byte j = 0 ;
   for (j=0 ; j < 8; j++)
   {
     if (value & 1) bits[j] = 1 ;
     else bits[j] = 0 ;
     value = value >> 1 ;
   }
   digitalWrite(xt_clk, LOW) ;
   digitalWrite(xt_data, HIGH) ;
   delayMicroseconds(120) ;
   digitalWrite(xt_clk, HIGH) ;
   delayMicroseconds(66) ;
   digitalWrite(xt_clk, LOW) ;
   delayMicroseconds(30) ;
   digitalWrite(xt_clk, HIGH) ;
   byte i = 0 ;
   for (i=0; i < 8; i++)
   {
      digitalWrite(xt_clk, HIGH) ;
      digitalWrite(xt_data, bits[p]) ;
      delayMicroseconds(95) ;
      digitalWrite(xt_clk, LOW) ;
      digitalWrite(xt_data, LOW) ;
      p++ ;
   }
   digitalWrite(xt_clk, HIGH) ;
   digitalWrite(xt_data, LOW) ;
   delay(1) ;
}
void key(int usbScan, bool down) {
  if(DEBUG) {
    Serial.print(down ? "pressed: " : "unpressed: ");
    Serial.print("usbOrig: ");
    Serial.print(usbScan);
  }
  // the array is sparse, and this is stupid. Sorry.
  if(usbScan == 154) usbScan = 100;
  if(usbScan >= 224) usbScan -= 123;
  uint8_t xt_first = usbscan_xt[usbScan];
  switch(xt_first) {
    case 0xe0: // two byte
      //TODO: support two byte stuff
      //sparse again, sorry
      switch(usbScan) {
        case 70:
          usbScan =- 70;
          break;
        case 73 ... 82:
          usbScan -= 72;
          break;
        case 84:
          usbScan = 11;
          break;
        case 88:
          usbScan = 12;
          break;
        //it's the 2xx range, which already had 123 subtracted
        case 227 ... 228:
          usbScan -= 91;
          break;
        case 230 ... 231:
          usbScan -= 92;
          break;
      }
      switch(usbscan_xt_byte2[usbScan]) {
        case 0x1d: // RCtrl
          xt_first = 0x1d;
          break;
        case 0x38: // RAlt
          xt_first = 0x38;
          break;
        case 0x1c: // kp-enter
          xt_first = 0x1c;
          break;
      }
      break;
    case 0xe1: // three byte(pause)
      //TODO: ignoring pause
      break;
  }
  if(DEBUG) {
    Serial.print(", usbFinal: ");
    Serial.print(usbScan);
    Serial.print(", xt: 0x");
    Serial.println(xt_first, HEX);
  }
  if(down) {
    xt_write(xt_first);
  } else {
    xt_write(xt_first | 0x80);
  }
}
void KeyboardRaw::Parse(HID *hid, uint32_t is_rpt_id, uint32_t len, uint8_t *buf)
{
  /*
   * USB HID keyboard report
Offset Description
0      Modifier keys status.
1      Reserved field.
2      Keypress #1.
3      Keypress #2.
4      Keypress #3.
5      Keypress #4.
6      Keypress #5.
7      Keypress #6.
   */

  if(DEBUG) {
    Serial.println("keyboard parse buf: ");
    for(int i=0; i<len; ++i) {
      Serial.print("  ");
      Serial.print(i);
      Serial.print(": ");
      Serial.print("  0x");
      Serial.println(buf[i], HEX);
    }
  }

  //curmod/prevmod are for modifiers
  static uint8_t prevMod = 0;
  uint8_t curMod = buf[0];

  for (int i = 0; i < 8; i++) {
    if(curMod & (1 << i)) {
      if(!(prevMod & (1 << i))) {
        // this lets us deal with multi-byte keys in one place converting the usb modifiers
        // to codes
        key(usbModLookup[i], true);
      } // else continued being pressed
    } else if(prevMod & (1 << i)) {
        key(usbModLookup[i], false);
    }
  }

  //buf[2] to buf[len] are a list of pressed keys
  static uint8_t* curScans = NULL;
  static uint8_t* prevScans = NULL;
  static size_t scanLen = 6; //TODO: check len to see if it's bigger and realloc or whatever
  if(curScans == NULL) {
    curScans = (uint8_t*)calloc(scanLen, 1);
  }
  if(prevScans == NULL) {
    prevScans = (uint8_t*)calloc(scanLen, 1);
  }
  memcpy (curScans, &buf[2], len-2);
  int i = 0;
  int j = 0;
  uint8_t p;
  uint8_t c;
  while (i < 7 && j < 7) {
    p = prevScans[j];
    c = curScans[i];
    if (p != c) {
      if (p == 0) { // New key, so send "down"
        if (canConvertUSB2XT(c)) {
          key(c, true);
        }
        i++;
      } else { // Previous key down, so send "up"
        if (canConvertUSB2XT(c)) {
          key(p, false);
        }
        j++;
      }
    } else {
      // Matching keys, advance both
      i++;
      j++;
    }
  }

  // Save current into previous
  prevMod = curMod;
  memcpy (prevScans, curScans, scanLen);
}

// Attach keyboard controller to USB
KeyboardRaw keyboard(usb);

void setup()
{
  if(DEBUG) {
    Serial.begin(9600);
    Serial.println("Program started");
  }

  strip.begin(); // Initialize pins for output
  strip.show();  // Turn all LEDs off ASAP

  pinMode(xt_clk, OUTPUT) ;
  pinMode(xt_data, OUTPUT) ;
  digitalWrite(xt_clk, HIGH) ;
  digitalWrite(xt_data, HIGH) ;

  delay(200);
}

void loop()
{

  // Process USB tasks

  usb.Task();

  static uint32_t lastUSBstate;
  uint32_t currentUSBstate = usb.getUsbTaskState();
  if (lastUSBstate != currentUSBstate)
  {
    switch (currentUSBstate)
    {
    case USB_ATTACHED_SUBSTATE_SETTLE:
      strip.setPixelColor(0, 0x280028); // purple
      strip.show();
      break;
    case USB_DETACHED_SUBSTATE_WAIT_FOR_DEVICE:
      strip.setPixelColor(0, 0x000028); // blue
      strip.show();
      break;
    case USB_ATTACHED_SUBSTATE_RESET_DEVICE:
      strip.setPixelColor(0, 0x282800); // yellow
      strip.show();
      break;
    case USB_ATTACHED_SUBSTATE_WAIT_RESET_COMPLETE:
      strip.setPixelColor(0, 0x281400); // orange
      strip.show();
      break;
    case USB_STATE_CONFIGURING:
      strip.setPixelColor(0, 0x002828); // cyan
      strip.show();
      break;
    case USB_STATE_RUNNING:
      strip.setPixelColor(0, 0x005500); // green, bright
      strip.show();
      //resetkb ();
      strip.setPixelColor(0, 0x000000); // off
      strip.show();
      break;
    }
    lastUSBstate = currentUSBstate;
  }
}
