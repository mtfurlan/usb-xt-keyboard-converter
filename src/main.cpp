#include <Arduino.h>
#include "xt_defines.h"

#define xt_data 0
#define xt_clk 2

#define RX_BUF_SIZE 1024
uint8_t uartRxBuff[RX_BUF_SIZE];
int rxPos = 0;
int cmdLength = 0;
uint8_t cmdType = 0;
long lastRxReceive = 0;

String deviceType[] = { "UNKNOWN", "POINTER",  "MOUSE",  "RESERVED",   "JOYSTICK",
                        "GAMEPAD", "KEYBOARD", "KEYPAD", "MULTI_AXIS", "SYSTEM" };
String keyboardstring;


typedef enum {
    MSG_TYPE_CONNECTED = 0x01,
    MSG_TYPE_DISCONNECTED = 0x02,
    MSG_TYPE_ERROR = 0x03,
    MSG_TYPE_DEVICE_POLL = 0x04,
    MSG_TYPE_DEVICE_STRING = 0x05,
    MSG_TYPE_DEVICE_INFO = 0x06,
    MSG_TYPE_HID_INFO = 0x07,
    MSG_TYPE_STARTUP = 0x08,
} msg_type_t;


typedef struct {
    uint8_t prefix;
    uint16_t cmdLength;
    msg_type_t msgType : 8;
    uint8_t devType;
    uint8_t device;
    uint8_t endpoint;
    uint16_t idVendor;
    uint16_t idProduct;
    uint8_t command[];
} __attribute__((packed)) packetthing_t;


bool canConvertUSB2XT(uint8_t usbScanCode) {
    return usbScanCode <= 69; //TODO: figure it out
}

void xt_write(unsigned char value)
{
    //while (digitalRead(xt_clk) != HIGH) ;
    unsigned char bits[8] ;
    byte p = 0 ;
    byte j = 0 ;
    for (j=0 ; j < 8; j++) {
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
    for (i=0; i < 8; i++) {
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
    // TODO: rework array map
    if(usbScan == 154) usbScan = 100;
    if(usbScan >= 224) usbScan -= 123;
    uint8_t xt_first = usbscan_xt[usbScan];
    switch(xt_first) {
        case 0xe0: // two byte
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
            //TODO: support two byte stuff instead of reampping some of it
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

void parseKeyboard(uint8_t* buf, size_t len)
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
     ...
     N-1    Keypress #N.
     */

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


void printHex(uint8_t* buf, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        Serial.print("0x");
        Serial.print(buf[i], HEX);
        Serial.print(" ");
    }
}

void filterCommand(int buffLength, packetthing_t* packet)
{
    //printHex((uint8_t*)packet, buffLength);
    //Serial.println();
    switch (packet->msgType) {
        case MSG_TYPE_CONNECTED:
            Serial.print("Device Connected on port");
            Serial.println(packet->device);
            break;
        case MSG_TYPE_DISCONNECTED:
            Serial.print("Device Disconnected on port");
            Serial.println(packet->device);
            break;
        case MSG_TYPE_ERROR:
            Serial.print("Device Error ");
            Serial.print(packet->device);
            Serial.print(" on port ");
            Serial.println(packet->devType);
            break;
        case MSG_TYPE_DEVICE_POLL:
            Serial.print("Device HID Data from port: ");
            Serial.print(packet->device);
            Serial.print(" , Length: ");
            Serial.print((int)packet->cmdLength);
            Serial.print(" , Type: ");
            Serial.print(deviceType[packet->devType]);
            Serial.printf(" , ID: %04X:%04X", packet->idVendor, packet->idProduct);
            Serial.print(" ,    ");
            printHex(packet->command, packet->cmdLength);
            Serial.println();
            if(packet->devType == 6) {
                parseKeyboard(packet->command, packet->cmdLength);
            }
            break;
        case MSG_TYPE_DEVICE_STRING:
            Serial.print("Device String port ");
            Serial.print(packet->devType);
            Serial.print(" Name: ");
            for (size_t i = 0; i < packet->cmdLength; i++) {
                Serial.write(packet->command[i]);
            }
            Serial.println();
            break;
        case MSG_TYPE_DEVICE_INFO:
            Serial.print("Device info from port");
            Serial.print(packet->device);
            Serial.print(", Descriptor: ");
            printHex(packet->command, packet->cmdLength);
            Serial.println();
            break;
        case MSG_TYPE_HID_INFO:
            Serial.print("HID info from port");
            Serial.print(packet->device);
            Serial.print(", Descriptor: ");
            printHex(packet->command, packet->cmdLength);
            Serial.println();
            break;
        case MSG_TYPE_STARTUP:
            Serial.println("USB host ready");
            break;
    }
}

void setup(void)
{
    Serial.begin(115200);
    Serial2.begin(1000000, SERIAL_8N1);
    Serial.println("OK There");


    pinMode(xt_clk, OUTPUT) ;
    pinMode(xt_data, OUTPUT) ;
    digitalWrite(xt_clk, HIGH) ;
    digitalWrite(xt_data, HIGH) ;
}

void loop()
{
    while (Serial2.available()) {
        lastRxReceive = millis();
        //Serial.print("h0x");//Only for Debug
        //Serial.print(Serial2.peek(),HEX);//Only for Debug
        //Serial.print(" ");//Only for Debug
        uartRxBuff[rxPos] = Serial2.read();
        if (rxPos == 0 && uartRxBuff[rxPos] == 0xFE) {
            cmdType = 1;
        } else if (rxPos == 1 && cmdType == 1) {
            cmdLength = uartRxBuff[rxPos];
        } else if (rxPos == 2 && cmdType == 1) {
            cmdLength += (uartRxBuff[rxPos] << 8);
            //printf( "Length: %i\n", cmdLength);//Only for Debug
        } else if (cmdType == 0 && uartRxBuff[rxPos] == '\n') {
            printf("No COMMAND Received\n");
            for (uint8_t i = 0; i < rxPos; i++) {
                printf("0x%02X ", uartRxBuff[i]);
            }
            printf("\n");
            rxPos = 0;
            cmdType = 0;
        }
        if (rxPos > 0 && rxPos == cmdLength + 11 && cmdType || rxPos > RX_BUF_SIZE) {
            filterCommand(rxPos, (packetthing_t*)uartRxBuff);
            rxPos = 0;
            cmdType = 0;
        } else {
            rxPos++;
        }
    }
    rxPos = 0;

    if (Serial.available()) {
        Serial2.write(Serial.read());
    }
}
