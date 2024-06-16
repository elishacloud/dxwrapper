#pragma once

#define HID_USAGE_PAGE_GENERIC 0x01
#define HID_USAGE_GENERIC_MOUSE 0x02
#define HID_USAGE_GENERIC_JOYSTICK 0x04
#define HID_USAGE_GENERIC_GAMEPAD 0x05
#define HID_USAGE_GENERIC_KEYBOARD 0x06

#define DEFINE_GUID2(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        const GUID name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

DEFINE_GUID2(GUID_Xbox360Controller, 0x028E045E, 0x0000, 0x0000, 0x00, 0x00, 0x50, 0x49, 0x44, 0x56, 0x49, 0x44);

#define DIDOI_GUIDISUSAGE       0x00010000

#define DIPH_DEVICE             0
#define DIPH_BYOFFSET           1

#define DIGFFS_POWEROFF     0x00000080
#define DIGFFS_EMPTY        0x00000001

#define IS_DIPROP(x)    (((ULONG_PTR)(x) >> 16) == 0)

#define MAKEDIPROP(prop)    (*(const GUID *)(prop))
#define DIPROP_BUFFERSIZE   MAKEDIPROP(1)
#define DIPROP_AXISMODE     MAKEDIPROP(2)

#define DIPROPAXISMODE_ABS  0
#define DIPROPAXISMODE_REL  1

#define DIPROP_GRANULARITY  MAKEDIPROP(3)
#define DIPROP_RANGE        MAKEDIPROP(4)
#define DIPROP_DEADZONE     MAKEDIPROP(5)
#define DIPROP_SATURATION   MAKEDIPROP(6)
#define DIPROP_FFGAIN       MAKEDIPROP(7)
#define DIPROP_FFLOAD       MAKEDIPROP(8)
#define DIPROP_AUTOCENTER   MAKEDIPROP(9)

#define DIPROPAUTOCENTER_OFF    0
#define DIPROPAUTOCENTER_ON 1

#define DIPROP_CALIBRATIONMODE  MAKEDIPROP(10)

#define DIPROPCALIBRATIONMODE_COOKED    0
#define DIPROPCALIBRATIONMODE_RAW   1

#define DIPROP_CALIBRATION  MAKEDIPROP(11)
#define DIPROP_GUIDANDPATH  MAKEDIPROP(12)

#define DIPROP_INSTANCENAME     MAKEDIPROP(13)
#define DIPROP_PRODUCTNAME      MAKEDIPROP(14)
#define DIPROP_JOYSTICKID       MAKEDIPROP(15)

#define DIPROP_KEYNAME     MAKEDIPROP(20)
#define DIPROP_CPOINTS     MAKEDIPROP(21)
#define DIPROP_APPDATA     MAKEDIPROP(22)
#define DIPROP_SCANCODE    MAKEDIPROP(23)
#define DIPROP_VIDPID      MAKEDIPROP(24)
#define DIPROP_USERNAME    MAKEDIPROP(25)
#define DIPROP_TYPENAME    MAKEDIPROP(26)

#define DIK_ESCAPE   0x01
#define DIK_1   0x02
#define DIK_2   0x03
#define DIK_3   0x04
#define DIK_4   0x05
#define DIK_5   0x06
#define DIK_6   0x07
#define DIK_7   0x08
#define DIK_8   0x09
#define DIK_9   0x0A
#define DIK_0   0x0B
#define DIK_MINUS   0x0C
#define DIK_EQUALS   0x0D
#define DIK_BACK   0x0E
#define DIK_TAB   0x0F
#define DIK_Q   0x10
#define DIK_W   0x11
#define DIK_E   0x12
#define DIK_R   0x13
#define DIK_T   0x14
#define DIK_Y   0x15
#define DIK_U   0x16
#define DIK_I   0x17
#define DIK_O   0x18
#define DIK_P   0x19
#define DIK_LBRACKET   0x1A
#define DIK_RBRACKET   0x1B
#define DIK_RETURN   0x1C
#define DIK_LCONTROL   0x1D
#define DIK_A   0x1E
#define DIK_S   0x1F
#define DIK_D   0x20
#define DIK_F   0x21
#define DIK_G   0x22
#define DIK_H   0x23
#define DIK_J   0x24
#define DIK_K   0x25
#define DIK_L   0x26
#define DIK_SEMICOLON   0x27
#define DIK_APOSTROPHE   0x28
#define DIK_GRAVE   0x29
#define DIK_LSHIFT   0x2A
#define DIK_BACKSLASH   0x2B
#define DIK_Z   0x2C
#define DIK_X   0x2D
#define DIK_C   0x2E
#define DIK_V   0x2F
#define DIK_B   0x30
#define DIK_N   0x31
#define DIK_M   0x32
#define DIK_COMMA   0x33
#define DIK_PERIOD   0x34
#define DIK_SLASH   0x35
#define DIK_RSHIFT   0x36
#define DIK_MULTIPLY   0x37
#define DIK_LMENU   0x38
#define DIK_SPACE   0x39
#define DIK_CAPITAL   0x3A
#define DIK_F1   0x3B
#define DIK_F2   0x3C
#define DIK_F3   0x3D
#define DIK_F4   0x3E
#define DIK_F5   0x3F
#define DIK_F6   0x40
#define DIK_F7   0x41
#define DIK_F8   0x42
#define DIK_F9   0x43
#define DIK_F10   0x44
#define DIK_NUMLOCK   0x45
#define DIK_SCROLL   0x46
#define DIK_NUMPAD7   0x47
#define DIK_NUMPAD8   0x48
#define DIK_NUMPAD9   0x49
#define DIK_SUBTRACT   0x4A
#define DIK_NUMPAD4   0x4B
#define DIK_NUMPAD5   0x4C
#define DIK_NUMPAD6   0x4D
#define DIK_ADD   0x4E
#define DIK_NUMPAD1   0x4F
#define DIK_NUMPAD2   0x50
#define DIK_NUMPAD3   0x51
#define DIK_NUMPAD0   0x52
#define DIK_DECIMAL   0x53
#define DIK_F11   0x57
#define DIK_F12   0x58
#define DIK_F13   0x64
#define DIK_F14   0x65
#define DIK_F15   0x66
#define DIK_KANA   0x70
#define DIK_CONVERT   0x79
#define DIK_NOCONVERT   0x7B
#define DIK_YEN   0x7D
#define DIK_NUMPADEQUALS   0x8D
#define DIK_AT   0x91
#define DIK_COLON   0x92
#define DIK_UNDERLINE   0x93
#define DIK_KANJI   0x94
#define DIK_STOP   0x95
#define DIK_AX   0x96
#define DIK_UNLABELED   0x97
#define DIK_NUMPADENTER   0x9C
#define DIK_RCONTROL   0x9D
#define DIK_NUMPADCOMMA   0xB3
#define DIK_DIVIDE   0xB5
#define DIK_SYSRQ   0xB7
#define DIK_RMENU   0xB8
#define DIK_PAUSE   0xC5
#define DIK_HOME   0xC7
#define DIK_UP   0xC8
#define DIK_PRIOR   0xC9
#define DIK_LEFT   0xCB
#define DIK_RIGHT   0xCD
#define DIK_END   0xCF
#define DIK_DOWN   0xD0
#define DIK_NEXT   0xD1
#define DIK_INSERT   0xD2
#define DIK_DELETE   0xD3
#define DIK_LWIN   0xDB
#define DIK_RWIN   0xDC
#define DIK_APPS   0xDD
#define DIK_POWER   0xDE
#define DIK_SLEEP   0xDF

#define DI8DEVTYPE_MOUSE            0x12
#define DI8DEVTYPE_KEYBOARD         0x13
#define DI8DEVTYPE_JOYSTICK         0x14
#define DI8DEVTYPE_GAMEPAD          0x15



#define DI8DEVTYPEMOUSE_UNKNOWN         1
#define DI8DEVTYPEMOUSE_TRADITIONAL     2

#define DIDEVTYPEKEYBOARD_PCENH         4

#define DI8DEVTYPEGAMEPAD_STANDARD      2

#define DIENUM_STOP                     0
#define DIENUM_CONTINUE                 1

#define DI_OK                           S_OK
#define DIERR_INVALIDPARAM              E_INVALIDARG
#define DIERR_UNSUPPORTED               E_NOTIMPL
#define DI_NOEFFECT                     S_FALSE

#define DIDFT_OPTIONAL		0x80000000
#define DIDFT_ABSAXIS       0x00000002
#define DIDFT_ALIAS		0x08000000
#define DIDFT_ALL           0x00000000
#define DIDFT_AXIS          0x00000003
#define DIDFT_BUTTON        0x0000000C
#define DIDFT_COLLECTION	0x00000040
#define DIDFT_ENUMCOLLECTION(n)	((WORD)(n) << 8)
#define DIDFT_FFACTUATOR	0x01000000
#define DIDFT_FFEFFECTTRIGGER	0x02000000
#define DIDFT_NOCOLLECTION	0x00FFFF00
#define DIDFT_NODATA	0x00000080
#define DIDFT_OUTPUT	0x10000000
#define DIDFT_POV	0x00000010
#define DIDFT_PSHBUTTON	0x00000004
#define DIDFT_RELAXIS	0x00000001
#define DIDFT_TGLBUTTON	0x00000008
#define DIDFT_VENDORDEFINED	0x04000000

#define DISCL_EXCLUSIVE 0x00000001
#define DISCL_NONEXCLUSIVE 0x00000002
#define DISCL_FOREGROUND 0x00000004
#define DISCL_BACKGROUND 0x00000008
#define DISCL_NOWINKEY 0x00000010

#ifndef D3DCOLOR_DEFINED
typedef DWORD D3DCOLOR;
#define D3DCOLOR_DEFINED
#endif
