#include <DDrawCompat/v0.3.1/Common/Disasm.h>

namespace
{
	enum OpcodeFlags
	{
		OF_NONE = 0,
		OF_PREFIX = 1 << 0,
		OF_PREFIX_OS = 1 << 1,
		OF_PREFIX_AS = 1 << 2,
		OF_ESCAPE = 1 << 3,
		OF_MODRM = 1 << 4,
		OF_AS = 1 << 5,
		OF_IB = 1 << 6,
		OF_IW = 1 << 7,
		OF_IW_ID = 1 << 8,
		OF_ICOND = 1 << 9,
		OF_CB = 1 << 10,
		OF_CW_CD = 1 << 11,
		OF_CD_CP = 1 << 12,
		OF_INVALID = 1 << 13
	};

	const unsigned g_opcodeFlags[256] = {
		OF_MODRM,                           // 00 /r
		OF_MODRM,                           // 01 /r
		OF_MODRM,                           // 02 /r
		OF_MODRM,                           // 03 /r
		OF_IB,                              // 04 ib
		OF_IW_ID,                           // 05 iw/id
		OF_NONE,                            // 06
		OF_NONE,                            // 07
		OF_MODRM,                           // 08 /r
		OF_MODRM,                           // 09 /r
		OF_MODRM,                           // 0A /r
		OF_MODRM,                           // 0B /r
		OF_IB,                              // 0C ib
		OF_IW_ID,                           // 0D iw/id
		OF_NONE,                            // 0E
		OF_ESCAPE,                          // 0F
		OF_MODRM,                           // 10 /r
		OF_MODRM,                           // 11 /r
		OF_MODRM,                           // 12 /r
		OF_MODRM,                           // 13 /r
		OF_IB,                              // 14 ib
		OF_IW_ID,                           // 15 iw/id
		OF_NONE,                            // 16
		OF_NONE,                            // 17
		OF_MODRM,                           // 18 /r
		OF_MODRM,                           // 19 /r
		OF_MODRM,                           // 1A /r
		OF_MODRM,                           // 1B /r
		OF_IB,                              // 1C ib
		OF_IW_ID,                           // 1D iw/id
		OF_NONE,                            // 1E
		OF_NONE,                            // 1F
		OF_MODRM,                           // 20 /r
		OF_MODRM,                           // 21 /r
		OF_MODRM,                           // 22 /r
		OF_MODRM,                           // 23 /r
		OF_IB,                              // 24 ib
		OF_IW_ID,                           // 25 iw/id
		OF_PREFIX,                          // 26
		OF_NONE,                            // 27
		OF_MODRM,                           // 28 /r
		OF_MODRM,                           // 29 /r
		OF_MODRM,                           // 2A /r
		OF_MODRM,                           // 2B /r
		OF_IB,                              // 2C ib
		OF_IW_ID,                           // 2D iw/id
		OF_PREFIX,                          // 2E
		OF_NONE,                            // 2F
		OF_MODRM,                           // 30 /r
		OF_MODRM,                           // 31 /r
		OF_MODRM,                           // 32 /r
		OF_MODRM,                           // 33 /r
		OF_IB,                              // 34 ib
		OF_IW_ID,                           // 35 iw/id
		OF_PREFIX,                          // 36
		OF_NONE,                            // 37
		OF_MODRM,                           // 38 /r
		OF_MODRM,                           // 39 /r
		OF_MODRM,                           // 3A /r
		OF_MODRM,                           // 3B /r
		OF_IB,                              // 3C ib
		OF_IW_ID,                           // 3D iw/id
		OF_PREFIX,                          // 3E
		OF_NONE,                            // 3F
		OF_NONE,                            // 40 (40+rw/rd)
		OF_NONE,                            // 41 (40+rw/rd)
		OF_NONE,                            // 42 (40+rw/rd)
		OF_NONE,                            // 43 (40+rw/rd)
		OF_NONE,                            // 44 (40+rw/rd)
		OF_NONE,                            // 45 (40+rw/rd)
		OF_NONE,                            // 46 (40+rw/rd)
		OF_NONE,                            // 47 (40+rw/rd)
		OF_NONE,                            // 48 (48+rw/rd)
		OF_NONE,                            // 49 (48+rw/rd)
		OF_NONE,                            // 4A (48+rw/rd)
		OF_NONE,                            // 4B (48+rw/rd)
		OF_NONE,                            // 4C (48+rw/rd)
		OF_NONE,                            // 4D (48+rw/rd)
		OF_NONE,                            // 4E (48+rw/rd)
		OF_NONE,                            // 4F (48+rw/rd)
		OF_NONE,                            // 50 (50+rw/rd)
		OF_NONE,                            // 51 (50+rw/rd)
		OF_NONE,                            // 52 (50+rw/rd)
		OF_NONE,                            // 53 (50+rw/rd)
		OF_NONE,                            // 54 (50+rw/rd)
		OF_NONE,                            // 55 (50+rw/rd)
		OF_NONE,                            // 56 (50+rw/rd)
		OF_NONE,                            // 57 (50+rw/rd)
		OF_NONE,                            // 58 (58+rw/rd)
		OF_NONE,                            // 59 (58+rw/rd)
		OF_NONE,                            // 5A (58+rw/rd)
		OF_NONE,                            // 5B (58+rw/rd)
		OF_NONE,                            // 5C (58+rw/rd)
		OF_NONE,                            // 5D (58+rw/rd)
		OF_NONE,                            // 5E (58+rw/rd)
		OF_NONE,                            // 5F (58+rw/rd)
		OF_NONE,                            // 60
		OF_NONE,                            // 61
		OF_MODRM,                           // 62 /r
		OF_MODRM,                           // 63 /r
		OF_PREFIX,                          // 64
		OF_PREFIX,                          // 65
		OF_PREFIX | OF_PREFIX_OS,           // 66
		OF_PREFIX | OF_PREFIX_AS,           // 67
		OF_IW_ID,                           // 68 iw/id
		OF_MODRM | OF_IW_ID,                // 69 /r iw/id
		OF_IB,                              // 6A ib
		OF_MODRM | OF_IB,                   // 6B /r ib
		OF_NONE,                            // 6C
		OF_NONE,                            // 6D
		OF_NONE,                            // 6E
		OF_NONE,                            // 6F
		OF_CB,                              // 70 cb
		OF_CB,                              // 71 cb
		OF_CB,                              // 72 cb
		OF_CB,                              // 73 cb
		OF_CB,                              // 74 cb
		OF_CB,                              // 75 cb
		OF_CB,                              // 76 cb
		OF_CB,                              // 77 cb
		OF_CB,                              // 78 cb
		OF_CB,                              // 79 cb
		OF_CB,                              // 7A cb
		OF_CB,                              // 7B cb
		OF_CB,                              // 7C cb
		OF_CB,                              // 7D cb
		OF_CB,                              // 7E cb
		OF_CB,                              // 7F cb
		OF_MODRM | OF_IB,                   // 80 /r ib
		OF_MODRM | OF_IW_ID,                // 81 /r iw/id
		OF_MODRM | OF_IB,                   // 82 /r ib
		OF_MODRM | OF_IB,                   // 83 /r ib
		OF_MODRM,                           // 84 /r
		OF_MODRM,                           // 85 /r
		OF_MODRM,                           // 86 /r
		OF_MODRM,                           // 87 /r
		OF_MODRM,                           // 88 /r
		OF_MODRM,                           // 89 /r
		OF_MODRM,                           // 8A /r
		OF_MODRM,                           // 8B /r
		OF_MODRM,                           // 8C /r
		OF_MODRM,                           // 8D /r
		OF_MODRM,                           // 8E /r
		OF_MODRM,                           // 8F /r
		OF_NONE,                            // 90
		OF_NONE,                            // 91 (90+rw/rd)
		OF_NONE,                            // 92 (90+rw/rd)
		OF_NONE,                            // 93 (90+rw/rd)
		OF_NONE,                            // 94 (90+rw/rd)
		OF_NONE,                            // 95 (90+rw/rd)
		OF_NONE,                            // 96 (90+rw/rd)
		OF_NONE,                            // 97 (90+rw/rd)
		OF_NONE,                            // 98
		OF_NONE,                            // 99
		OF_CD_CP,                           // 9A cd/cp
		OF_NONE,                            // 9B
		OF_NONE,                            // 9C
		OF_NONE,                            // 9D
		OF_NONE,                            // 9E
		OF_NONE,                            // 9F
		OF_IW_ID | OF_AS,                   // A0 iw/id (AS)
		OF_IW_ID | OF_AS,                   // A1 iw/id (AS)
		OF_IW_ID | OF_AS,                   // A2 iw/id (AS)
		OF_IW_ID | OF_AS,                   // A3 iw/id (AS)
		OF_NONE,                            // A4
		OF_NONE,                            // A5
		OF_NONE,                            // A6
		OF_NONE,                            // A7
		OF_IB,                              // A8 ib
		OF_IW_ID,                           // A9 iw/id
		OF_NONE,                            // AA
		OF_NONE,                            // AB
		OF_NONE,                            // AC
		OF_NONE,                            // AD
		OF_NONE,                            // AE
		OF_NONE,                            // AF
		OF_IB,                              // B0 (B0+rb) ib
		OF_IB,                              // B1 (B0+rb) ib
		OF_IB,                              // B2 (B0+rb) ib
		OF_IB,                              // B3 (B0+rb) ib
		OF_IB,                              // B4 (B0+rb) ib
		OF_IB,                              // B5 (B0+rb) ib
		OF_IB,                              // B6 (B0+rb) ib
		OF_IB,                              // B7 (B0+rb) ib
		OF_IW_ID,                           // B8 (B8+rw/rd) iw/id
		OF_IW_ID,                           // B9 (B8+rw/rd) iw/id
		OF_IW_ID,                           // BA (B8+rw/rd) iw/id
		OF_IW_ID,                           // BB (B8+rw/rd) iw/id
		OF_IW_ID,                           // BC (B8+rw/rd) iw/id
		OF_IW_ID,                           // BD (B8+rw/rd) iw/id
		OF_IW_ID,                           // BE (B8+rw/rd) iw/id
		OF_IW_ID,                           // BF (B8+rw/rd) iw/id
		OF_MODRM | OF_IB,                   // C0 /r ib
		OF_MODRM | OF_IB,                   // C1 /r ib
		OF_IW,                              // C2 iw
		OF_NONE,                            // C3
		OF_MODRM,                           // C4 /r
		OF_MODRM,                           // C5 /r
		OF_MODRM | OF_IB,                   // C6 /r ib
		OF_MODRM | OF_IW_ID,                // C7 /r iw/id
		OF_IW | OF_IB,                      // C8 iw ib
		OF_NONE,                            // C9
		OF_IW,                              // CA iw
		OF_NONE,                            // CB
		OF_NONE,                            // CC
		OF_IB,                              // CD ib
		OF_NONE,                            // CE
		OF_NONE,                            // CF
		OF_MODRM,                           // D0 /r
		OF_MODRM,                           // D1 /r
		OF_MODRM,                           // D2 /r
		OF_MODRM,                           // D3 /r
		OF_IB,                              // D4 ib
		OF_IB,                              // D5 ib
		OF_INVALID,                         // D6 ?
		OF_NONE,                            // D7
		OF_MODRM,                           // D8 /r
		OF_MODRM,                           // D9 /r
		OF_MODRM,                           // DA /r
		OF_MODRM,                           // DB /r
		OF_MODRM,                           // DC /r
		OF_MODRM,                           // DD /r
		OF_MODRM,                           // DE /r
		OF_MODRM,                           // DF /r
		OF_CB,                              // E0 cb
		OF_CB,                              // E1 cb
		OF_CB,                              // E2 cb
		OF_CB,                              // E3 cb
		OF_IB,                              // E4 ib
		OF_IB,                              // E5 ib
		OF_IB,                              // E6 ib
		OF_IB,                              // E7 ib
		OF_CW_CD,                           // E8 cw/cd
		OF_CW_CD,                           // E9 cw/cd
		OF_CD_CP,                           // EA cd/cp
		OF_CB,                              // EB cb
		OF_NONE,                            // EC
		OF_NONE,                            // ED
		OF_NONE,                            // EE
		OF_NONE,                            // EF
		OF_PREFIX,                          // F0
		OF_NONE,                            // F1
		OF_PREFIX,                          // F2
		OF_PREFIX,                          // F3
		OF_NONE,                            // F4
		OF_NONE,                            // F5
		OF_MODRM | OF_IB | OF_ICOND,        // F6 /r (ib)
		OF_MODRM | OF_IW_ID | OF_ICOND,     // F7 /r (iw/id)
		OF_NONE,                            // F8
		OF_NONE,                            // F9
		OF_NONE,                            // FA
		OF_NONE,                            // FB
		OF_NONE,                            // FC
		OF_NONE,                            // FD
		OF_MODRM,                           // FE /r
		OF_MODRM,                           // FF /r
	};

	const unsigned g_opcodeFlags0F[256] = {
		OF_MODRM,                           // 0F 00 /r
		OF_MODRM,                           // 0F 01 /r
		OF_MODRM,                           // 0F 02 /r
		OF_MODRM,                           // 0F 03 /r
		OF_INVALID,                         // 0F 04 ?
		OF_NONE,                            // 0F 05
		OF_NONE,                            // 0F 06
		OF_NONE,                            // 0F 07
		OF_NONE,                            // 0F 08
		OF_NONE,                            // 0F 09
		OF_INVALID,                         // 0F 0A ?
		OF_NONE,                            // 0F 0B
		OF_INVALID,                         // 0F 0C ?
		OF_MODRM,                           // 0F 0D /r
		OF_INVALID,                         // 0F 0E ?
		OF_INVALID,                         // 0F 0F ?
		OF_MODRM,                           // 0F 10 /r
		OF_MODRM,                           // 0F 11 /r
		OF_MODRM,                           // 0F 12 /r
		OF_MODRM,                           // 0F 13 /r
		OF_MODRM,                           // 0F 14 /r
		OF_MODRM,                           // 0F 15 /r
		OF_MODRM,                           // 0F 16 /r
		OF_MODRM,                           // 0F 17 /r
		OF_MODRM,                           // 0F 18 /r
		OF_INVALID,                         // 0F 19 ?
		OF_MODRM,                           // 0F 1A /r
		OF_MODRM,                           // 0F 1B /r
		OF_MODRM,                           // 0F 1C /r
		OF_INVALID,                         // 0F 1D ?
		OF_MODRM,                           // 0F 1E /r
		OF_MODRM,                           // 0F 1F /r
		OF_MODRM,                           // 0F 20 /r
		OF_MODRM,                           // 0F 21 /r
		OF_MODRM,                           // 0F 22 /r
		OF_MODRM,                           // 0F 23 /r
		OF_INVALID,                         // 0F 24 ?
		OF_INVALID,                         // 0F 25 ?
		OF_INVALID,                         // 0F 26 ?
		OF_INVALID,                         // 0F 27 ?
		OF_MODRM,                           // 0F 28 /r
		OF_MODRM,                           // 0F 29 /r
		OF_MODRM,                           // 0F 2A /r
		OF_MODRM,                           // 0F 2B /r
		OF_MODRM,                           // 0F 2C /r
		OF_MODRM,                           // 0F 2D /r
		OF_MODRM,                           // 0F 2E /r
		OF_MODRM,                           // 0F 2F /r
		OF_NONE,                            // 0F 30
		OF_NONE,                            // 0F 31
		OF_NONE,                            // 0F 32
		OF_NONE,                            // 0F 33
		OF_NONE,                            // 0F 34
		OF_NONE,                            // 0F 35
		OF_INVALID,                         // 0F 36 ?
		OF_NONE,                            // 0F 37
		OF_ESCAPE | OF_MODRM,               // 0F 38 xx /r
		OF_INVALID,                         // 0F 39 ?
		OF_ESCAPE | OF_MODRM | OF_IB,       // 0F 3A xx /r ib
		OF_INVALID,                         // 0F 3B ?
		OF_INVALID,                         // 0F 3C ?
		OF_INVALID,                         // 0F 3D ?
		OF_INVALID,                         // 0F 3E ?
		OF_INVALID,                         // 0F 3F ?
		OF_MODRM,                           // 0F 40 /r
		OF_MODRM,                           // 0F 41 /r
		OF_MODRM,                           // 0F 42 /r
		OF_MODRM,                           // 0F 43 /r
		OF_MODRM,                           // 0F 44 /r
		OF_MODRM,                           // 0F 45 /r
		OF_MODRM,                           // 0F 46 /r
		OF_MODRM,                           // 0F 47 /r
		OF_MODRM,                           // 0F 48 /r
		OF_MODRM,                           // 0F 49 /r
		OF_MODRM,                           // 0F 4A /r
		OF_MODRM,                           // 0F 4B /r
		OF_MODRM,                           // 0F 4C /r
		OF_MODRM,                           // 0F 4D /r
		OF_MODRM,                           // 0F 4E /r
		OF_MODRM,                           // 0F 4F /r
		OF_MODRM,                           // 0F 50 /r
		OF_MODRM,                           // 0F 51 /r
		OF_MODRM,                           // 0F 52 /r
		OF_MODRM,                           // 0F 53 /r
		OF_MODRM,                           // 0F 54 /r
		OF_MODRM,                           // 0F 55 /r
		OF_MODRM,                           // 0F 56 /r
		OF_MODRM,                           // 0F 57 /r
		OF_MODRM,                           // 0F 58 /r
		OF_MODRM,                           // 0F 59 /r
		OF_MODRM,                           // 0F 5A /r
		OF_MODRM,                           // 0F 5B /r
		OF_MODRM,                           // 0F 5C /r
		OF_MODRM,                           // 0F 5D /r
		OF_MODRM,                           // 0F 5E /r
		OF_MODRM,                           // 0F 5F /r
		OF_MODRM,                           // 0F 60 /r
		OF_MODRM,                           // 0F 61 /r
		OF_MODRM,                           // 0F 62 /r
		OF_MODRM,                           // 0F 63 /r
		OF_MODRM,                           // 0F 64 /r
		OF_MODRM,                           // 0F 65 /r
		OF_MODRM,                           // 0F 66 /r
		OF_MODRM,                           // 0F 67 /r
		OF_MODRM,                           // 0F 68 /r
		OF_MODRM,                           // 0F 69 /r
		OF_MODRM,                           // 0F 6A /r
		OF_MODRM,                           // 0F 6B /r
		OF_MODRM,                           // 0F 6C /r
		OF_MODRM,                           // 0F 6D /r
		OF_MODRM,                           // 0F 6E /r
		OF_MODRM,                           // 0F 6F /r
		OF_MODRM | OF_IB,                   // 0F 70 /r ib
		OF_MODRM | OF_IB,                   // 0F 71 /r ib
		OF_MODRM | OF_IB,                   // 0F 72 /r ib
		OF_MODRM | OF_IB,                   // 0F 73 /r ib
		OF_MODRM,                           // 0F 74 /r
		OF_MODRM,                           // 0F 75 /r
		OF_MODRM,                           // 0F 76 /r
		OF_NONE,                            // 0F 77
		OF_INVALID,                         // 0F 78 ?
		OF_INVALID,                         // 0F 79 ?
		OF_INVALID,                         // 0F 7A ?
		OF_INVALID,                         // 0F 7B ?
		OF_MODRM,                           // 0F 7C /r
		OF_MODRM,                           // 0F 7D /r
		OF_MODRM,                           // 0F 7E /r
		OF_MODRM,                           // 0F 7F /r
		OF_CW_CD,                           // 0F 80 cw/cd
		OF_CW_CD,                           // 0F 81 cw/cd
		OF_CW_CD,                           // 0F 82 cw/cd
		OF_CW_CD,                           // 0F 83 cw/cd
		OF_CW_CD,                           // 0F 84 cw/cd
		OF_CW_CD,                           // 0F 85 cw/cd
		OF_CW_CD,                           // 0F 86 cw/cd
		OF_CW_CD,                           // 0F 87 cw/cd
		OF_CW_CD,                           // 0F 88 cw/cd
		OF_CW_CD,                           // 0F 89 cw/cd
		OF_CW_CD,                           // 0F 8A cw/cd
		OF_CW_CD,                           // 0F 8B cw/cd
		OF_CW_CD,                           // 0F 8C cw/cd
		OF_CW_CD,                           // 0F 8D cw/cd
		OF_CW_CD,                           // 0F 8E cw/cd
		OF_CW_CD,                           // 0F 8F cw/cd
		OF_MODRM,                           // 0F 90 /r
		OF_MODRM,                           // 0F 91 /r
		OF_MODRM,                           // 0F 92 /r
		OF_MODRM,                           // 0F 93 /r
		OF_MODRM,                           // 0F 94 /r
		OF_MODRM,                           // 0F 95 /r
		OF_MODRM,                           // 0F 96 /r
		OF_MODRM,                           // 0F 97 /r
		OF_MODRM,                           // 0F 98 /r
		OF_MODRM,                           // 0F 99 /r
		OF_MODRM,                           // 0F 9A /r
		OF_MODRM,                           // 0F 9B /r
		OF_MODRM,                           // 0F 9C /r
		OF_MODRM,                           // 0F 9D /r
		OF_MODRM,                           // 0F 9E /r
		OF_MODRM,                           // 0F 9F /r
		OF_NONE,                            // 0F A0
		OF_NONE,                            // 0F A1
		OF_NONE,                            // 0F A2
		OF_MODRM,                           // 0F A3 /r
		OF_MODRM | OF_IB,                   // 0F A4 /r ib
		OF_MODRM,                           // 0F A5 /r
		OF_INVALID,                         // 0F A6 ?
		OF_INVALID,                         // 0F A7 ?
		OF_NONE,                            // 0F A8
		OF_NONE,                            // 0F A9
		OF_NONE,                            // 0F AA
		OF_MODRM,                           // 0F AB /r
		OF_MODRM | OF_IB,                   // 0F AC /r ib
		OF_MODRM,                           // 0F AD /r
		OF_MODRM,                           // 0F AE /r
		OF_MODRM,                           // 0F AF /r
		OF_MODRM,                           // 0F B0 /r
		OF_MODRM,                           // 0F B1 /r
		OF_MODRM,                           // 0F B2 /r
		OF_MODRM,                           // 0F B3 /r
		OF_MODRM,                           // 0F B4 /r
		OF_MODRM,                           // 0F B5 /r
		OF_MODRM,                           // 0F B6 /r
		OF_MODRM,                           // 0F B7 /r
		OF_MODRM,                           // 0F B8 /r
		OF_MODRM,                           // 0F B9 /r
		OF_MODRM | OF_IB,                   // 0F BA /r ib
		OF_MODRM,                           // 0F BB /r
		OF_MODRM,                           // 0F BC /r
		OF_MODRM,                           // 0F BD /r
		OF_MODRM,                           // 0F BE /r
		OF_MODRM,                           // 0F BF /r
		OF_MODRM,                           // 0F C0 /r
		OF_MODRM,                           // 0F C1 /r
		OF_MODRM | OF_IB,                   // 0F C2 /r ib
		OF_MODRM,                           // 0F C3 /r
		OF_MODRM | OF_IB,                   // 0F C4 /r ib
		OF_MODRM | OF_IB,                   // 0F C5 /r ib
		OF_MODRM | OF_IB,                   // 0F C6 /r ib
		OF_MODRM,                           // 0F C7 /r
		OF_NONE,                            // 0F C8 (C8+rd)
		OF_NONE,                            // 0F C9 (C8+rd)
		OF_NONE,                            // 0F CA (C8+rd)
		OF_NONE,                            // 0F C8 (C8+rd)
		OF_NONE,                            // 0F CC (C8+rd)
		OF_NONE,                            // 0F CD (C8+rd)
		OF_NONE,                            // 0F CE (C8+rd)
		OF_NONE,                            // 0F CF (C8+rd)
		OF_MODRM,                           // 0F D0 /r
		OF_MODRM,                           // 0F D1 /r
		OF_MODRM,                           // 0F D2 /r
		OF_MODRM,                           // 0F D3 /r
		OF_MODRM,                           // 0F D4 /r
		OF_MODRM,                           // 0F D5 /r
		OF_MODRM,                           // 0F D6 /r
		OF_MODRM,                           // 0F D7 /r
		OF_MODRM,                           // 0F D8 /r
		OF_MODRM,                           // 0F D9 /r
		OF_MODRM,                           // 0F DA /r
		OF_MODRM,                           // 0F DB /r
		OF_MODRM,                           // 0F DC /r
		OF_MODRM,                           // 0F DD /r
		OF_MODRM,                           // 0F DE /r
		OF_MODRM,                           // 0F DF /r
		OF_MODRM,                           // 0F E0 /r
		OF_MODRM,                           // 0F E1 /r
		OF_MODRM,                           // 0F E2 /r
		OF_MODRM,                           // 0F E3 /r
		OF_MODRM,                           // 0F E4 /r
		OF_MODRM,                           // 0F E5 /r
		OF_MODRM,                           // 0F E6 /r
		OF_MODRM,                           // 0F E7 /r
		OF_MODRM,                           // 0F E8 /r
		OF_MODRM,                           // 0F E9 /r
		OF_MODRM,                           // 0F EA /r
		OF_MODRM,                           // 0F EB /r
		OF_MODRM,                           // 0F EC /r
		OF_MODRM,                           // 0F ED /r
		OF_MODRM,                           // 0F EE /r
		OF_MODRM,                           // 0F EF /r
		OF_MODRM,                           // 0F F0 /r
		OF_MODRM,                           // 0F F1 /r
		OF_MODRM,                           // 0F F2 /r
		OF_MODRM,                           // 0F F3 /r
		OF_MODRM,                           // 0F F4 /r
		OF_MODRM,                           // 0F F5 /r
		OF_MODRM,                           // 0F F6 /r
		OF_MODRM,                           // 0F F7 /r
		OF_MODRM,                           // 0F F8 /r
		OF_MODRM,                           // 0F F9 /r
		OF_MODRM,                           // 0F FA /r
		OF_MODRM,                           // 0F FB /r
		OF_MODRM,                           // 0F FC /r
		OF_MODRM,                           // 0F FD /r
		OF_MODRM,                           // 0F FE /r
		OF_MODRM                            // 0F FF /r
	};
}

namespace Compat32
{
	unsigned getInstructionLength(const void* instruction)
	{
		auto instr = static_cast<const unsigned char*>(instruction);
		unsigned prefixFlags = 0;
		unsigned opcodeFlags = g_opcodeFlags[instr[0]];
		unsigned length = 1;

		while (opcodeFlags & OF_PREFIX)
		{
			prefixFlags |= opcodeFlags & (OF_PREFIX_AS | OF_PREFIX_OS);
			opcodeFlags = g_opcodeFlags[instr[length]];
			++length;
		}

		if (opcodeFlags & OF_ESCAPE)
		{
			opcodeFlags = g_opcodeFlags0F[instr[length]];
			++length;
			if (opcodeFlags & OF_ESCAPE)
			{
				++length;
			}
		}

		if (0 == opcodeFlags)
		{
			return length;
		}

		if (opcodeFlags & OF_INVALID)
		{
			return 0;
		}

		if (opcodeFlags & OF_MODRM)
		{
			const unsigned char modrm = instr[length];
			++length;

			if ((opcodeFlags & OF_ICOND) && 0 != ((modrm >> 3) & 7))
			{
				opcodeFlags &= ~(OF_IB | OF_IW_ID);
			}

			const unsigned char mod = modrm >> 6;
			const unsigned char rm = modrm & 7;
			if (1 == mod)
			{
				++length;   // disp8
			}
			else if (2 == mod)
			{
				length += (prefixFlags & OF_PREFIX_AS) ? 2 : 4;   // disp16/32
			}
			else if (0 == mod)
			{
				if (prefixFlags & OF_PREFIX_AS)
				{
					if (6 == rm)
					{
						length += 2;   // disp16
					}
				}
				else if (4 == rm && 5 == (instr[length] & 7) ||
					5 == rm)
				{
					length += 4;   // disp32
				}
			}

			if (4 == rm && 3 != mod && !(prefixFlags & OF_PREFIX_AS))
			{
				++length;   // SIB
			}
		}

		if (opcodeFlags & OF_IB)
		{
			++length;   // ib
		}
		if (opcodeFlags & OF_IW)
		{
			length += 2;   // iw
		}
		if (opcodeFlags & OF_IW_ID)
		{
			if (opcodeFlags & OF_AS)
			{
				length += (prefixFlags & OF_PREFIX_AS) ? 2 : 4;   // iw/id
			}
			else
			{
				length += (prefixFlags & OF_PREFIX_OS) ? 2 : 4;   // iw/id
			}
		}

		if (opcodeFlags & OF_CB)
		{
			++length;   // cb
		}
		else if (opcodeFlags & OF_CW_CD)
		{
			length += (prefixFlags & OF_PREFIX_OS) ? 2 : 4;   // cw/cd
		}
		else if (opcodeFlags & OF_CD_CP)
		{
			length += (prefixFlags & OF_PREFIX_OS) ? 4 : 6;   // cd/cp
		}

		return length;
	}
}
