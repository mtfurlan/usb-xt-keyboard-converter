#ifndef XT_DEFINES
#define XT_DEFINES



// go from usb modifier to "usb code" from https://www.scs.stanford.edu/10wi-cs140/pintos/specs/kbd/scancodes-9.html
const uint8_t usbModLookup[] =
{
    224,             // Left control
    225,            // Left Shift
    226,             // Left Alt
    227,             // Left Windows
    228,             // Right control
    229,            // right shift
    230,             // Right Alt
    231              // Right Windows
};


const uint8_t usbscan_xt_byte2[] =
{
    0x37, // usb 70 key PrtScr
    0x52, // usb 73 key Insert
    0x47, // usb 74 key Home
    0x49, // usb 75 key PgUp
    0x53, // usb 76 key Delete
    0x4f, // usb 77 key End
    0x51, // usb 78 key PgDn
    0x4d, // usb 79 key Right
    0x4b, // usb 80 key Left
    0x50, // usb 81 key Down
    0x48, // usb 82 key Up
    0x35, // usb 84 key KP-/
    0x1c, // usb 88 key KP-Enter
    0x5b, // usb 227 key LWin (USB: LGUI)
    0x1d, // usb 228 key RCtrl
    0x38, // usb 230 key RAlt
    0x5c, // usb 231 key RWin (USB: RGUI)

};

const uint8_t usbscan_xt[] =
{
    0,
    0,
    0,
    0,
    0x1e, // usb 4 key A
    0x30, // usb 5 key B
    0x2e, // usb 6 key C
    0x20, // usb 7 key D
    0x12, // usb 8 key E
    0x21, // usb 9 key F
    0x22, // usb 10 key G
    0x23, // usb 11 key H
    0x17, // usb 12 key I
    0x24, // usb 13 key J
    0x25, // usb 14 key K
    0x26, // usb 15 key L
    0x32, // usb 16 key M
    0x31, // usb 17 key N
    0x18, // usb 18 key O
    0x19, // usb 19 key P
    0x10, // usb 20 key Q
    0x13, // usb 21 key R
    0x1f, // usb 22 key S
    0x14, // usb 23 key T
    0x16, // usb 24 key U
    0x2f, // usb 25 key V
    0x11, // usb 26 key W
    0x2d, // usb 27 key X
    0x15, // usb 28 key Y
    0x2c, // usb 29 key Z
    0x02, // usb 30 key 1 !
    0x03, // usb 31 key 2 @
    0x04, // usb 32 key 3 #
    0x05, // usb 33 key 4 $
    0x06, // usb 34 key 5 % E
    0x07, // usb 35 key 6 ^
    0x08, // usb 36 key 7 &
    0x09, // usb 37 key 8 *
    0x0a, // usb 38 key 9 (
    0x0b, // usb 39 key 0 )
    0x1c, // usb 40 key Enter
    0x01, // usb 41 key Esc
    0x0e, // usb 42 key Backspace
    0x0f, // usb 43 key Tab
    0x39, // usb 44 key space
    0x0c, // usb 45 key - _
    0x0d, // usb 46 key = +
    0x1a, // usb 47 key [ {
    0x1b, // usb 48 key ] }
    0x2b, // usb 49 key \ |
    0x00, // usb 50 key non-US-1
    0x27, // usb 51 key ; :
    0x28, // usb 52 key ' "
    0x29, // usb 53 key ` ~
    0x33, // usb 54 key , <
    0x34, // usb 55 key . >
    0x35, // usb 56 key / ?
    0x3a, // usb 57 key CapsLock
    0x3b, // usb 58 key F1
    0x3c, // usb 59 key F2
    0x3d, // usb 60 key F3
    0x3e, // usb 61 key F4
    0x3f, // usb 62 key F5
    0x40, // usb 63 key F6
    0x41, // usb 64 key F7
    0x42, // usb 65 key F8
    0x43, // usb 66 key F9
    0x44, // usb 67 key F10
    0x57, // usb 68 key F11
    0x58, // usb 69 key F12
    0x37, // usb 70 key PrtScr eo-37
    0x46, // usb 71 key ScrollLock
    0xe1, // usb 72 key Pause e1-1d-45
    0x52, // usb 73 key Insert eo-52
    0x47, // usb 74 key Home eo-47
    0x49, // usb 75 key PgUp eo-49
    0x53, // usb 76 key Delete eo-53
    0x4f, // usb 77 key End eo-4f
    0x51, // usb 78 key PgDn eo-51
    0x4d, // usb 79 key Right eo-4d
    0x4b, // usb 80 key Left eo-4b
    0x50, // usb 81 key Down eo-50
    0x48, // usb 82 key Up eo-48
    0x45, // usb 83 key NumLock
    0x35, // usb 84 key KP-/ eo-35
    0x37, // usb 85 key KP-*
    0x4a, // usb 86 key KP--
    0x4e, // usb 87 key KP-+
    0x1c, // usb 88 key KP-Enter eo-1c
    0x4f, // usb 89 key KP-1 / End
    0x50, // usb 90 key KP-2 / Down
    0x51, // usb 91 key KP-3 / PgDn
    0x4b, // usb 92 key KP-4 / Left
    0x4c, // usb 93 key KP-5
    0x4d, // usb 94 key KP-6 / Right
    0x47, // usb 95 key KP-7 / Home
    0x48, // usb 96 key KP-8 / Up
    0x49, // usb 97 key KP-9 / PgUp
    0x52, // usb 98 key KP-0 / Ins
    0x53, // usb 99 key KP-. / Del
    0x54, // usb 154 key Alt+SysRq
    0x1d, // usb 224 key LCtrl
    0x2a, // usb 225 key LShift
    0x38, // usb 226 key LAlt
    0x5b, // usb 227 key LWin (USB: LGUI) eo-5b
    0x1d, // usb 228 key RCtrl eo-1d
    0x36, // usb 229 key RShift
    0x38, // usb 230 key RAlt eo-38
    0x5c, // usb 231 key RWin (USB: RGUI) eo-5c
};

#endif // XT_DEFINES
