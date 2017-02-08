////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// OllyDbg Disassembling Engine v2.01                                         //
//                                                                            //
// Copyright (c) 2007-2013 Oleh Yuschuk, ollydbg@t-online.de                  //
//                                                                            //
// This code is part of the OllyDbg Disassembler v2.01                        //
//                                                                            //
// Disassembling engine is free software; you can redistribute it and/or      //
// modify it under the terms of the GNU General Public License as published   //
// by the Free Software Foundation; either version 3 of the License, or (at   //
// your option) any later version.                                            //
//                                                                            //
// This code is distributed in the hope that it will be useful, but WITHOUT   //
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or      //
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for   //
// more details.                                                              //
//                                                                            //
// You should have received a copy of the GNU General Public License along    //
// with this program. If not, see <http://www.gnu.org/licenses/>.             //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This is a fast disassembler that can be used to determine the length of    //
// the binary 80x86 32-bit command and its attributes, to convert it to the   //
// human-readable text form, highlight its operands, and create hexadecimal   //
// dump of the binary command.                                                //
//                                                                            //
// It is a stripped down version of the disassembler used by OllyDbg 2.01.    //
// It can't analyse and comment the contents of the operands, or predict the  //
// results of the command execution. Analysis-dependent features are not      //
// included, too. Most other features are kept.                               //
//                                                                            //
// Disassembler supports integer, FPU, MMX, 3DNow, SSE1-SSE4.1 and AVX        //
// instructions. 64-bit mode, AVX2, FMA and XOP are not (yet) supported.      //
//                                                                            //
// This code can be compiled either in ASCII or UNICODE mode. It is reentrant //
// (thread-safe, feature not available in the original OllyDbg code).         //
//                                                                            //
// Typical operation speed on 3-GHz Phenom II in MASM mode is:                //
//                                                                            //
//   Command length and info:          130 ns/command (7,700,000 commands/s)  //
//   Disassembly:                      290 ns/command (3,400,000 commands/s)  //
//   Disassembly, dump, highlighting:  350 ns/command (2,800,000 commands/s)  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
////////////// PREFERRED SETTINGS AND FIXES FOR BORLAND COMPILERS //////////////

#ifdef __BORLANDC__
  #pragma option -a1                   // Byte alignment
  #pragma option -w-pin                // Disable "partially bracketed" warning
  // Redefinition of MAKELONG removes nasty warning under Borland Builder 4.0:
  // boolean OR in one row with arithmetical shift.
  #undef  MAKELONG
  #define MAKELONG(lo,hi) ((LONG)(((WORD)(lo))|(((DWORD)((WORD)(hi)))<<16)))
#endif

#define MAKEWP(lo,hi)  ((WPARAM)MAKELONG(lo,hi))
#define MAKELP(lo,hi)  ((LPARAM)MAKELONG(lo,hi))

#define LOINT(l)       ((signed short)((WORD)(l)))
#define HIINT(l)       ((signed short)(((DWORD)(l)>>16) & 0xFFFF))

#ifndef FIELD_OFFSET
  #define FIELD_OFFSET(type,field) ((long)&(((type *)0)->field))
#endif


////////////////////////////////////////////////////////////////////////////////

#define TEXTLEN        256             // Max length of text string
#define SHORTNAME      32              // Max length of short or module name

#ifdef UNICODE
  #define tchar        wchar_t         // Or replace it with TCHAR
  #define T(x)         L##x
  #define tstrlen      wcslen
  #define tstrcpy      wcscpy
  #define tstrcmp      wcscmp
  #define tstrlwr      _wcslwr
  #define tsprintf     swprintf
  #define ttolower     towlower
#else
  #define tchar        char
  #define T(x)         x
  #define tstrlen      strlen
  #define tstrcpy      strcpy
  #define tstrcmp      strcmp
  #define tstrlwr      strlwr
  #define tsprintf     sprintf
  #define ttolower     tolower
#endif

#define NOPERAND       4               // Maximal allowed number of operands
#define NREG           8               // Number of registers (of any type)
#define NSEG           6               // Number of valid segment registers
#define MAXCMDSIZE     16              // Maximal length of valid 80x86 command
#define NEGLIMIT       (-16384)        // Limit to decode offsets as negative
#define DECLIMIT       16384           // Limit to decode constants as decimal

typedef unsigned char  uchar;          // Unsigned character (byte)
typedef unsigned short ushort;         // Unsigned short
typedef unsigned int   uint;           // Unsigned integer
typedef unsigned long  ulong;          // Unsigned long

// CMDMASK can be used to balance between the necessary memory size and the
// disassembly time.
#define CMDMASK        0x3FFF          // Search mask for Disassembler, 2**n-1
#define NCHAIN         44300           // Max allowed number of chain links

// Registers.
#define REG_UNDEF      (-1)            // Codes of general purpose registers
#define REG_EAX        0
#define REG_ECX        1
#define REG_EDX        2
#define REG_EBX        3
#define REG_ESP        4
#define REG_EBP        5
#define REG_ESI        6
#define REG_EDI        7

#define REG_AL         0               // Symbolic indices of 8-bit registers
#define REG_CL         1
#define REG_DL         2
#define REG_BL         3
#define REG_AH         4
#define REG_CH         5
#define REG_DH         6
#define REG_BH         7

#define SEG_UNDEF      (-1)            // Codes of segment/selector registers
#define SEG_ES         0
#define SEG_CS         1
#define SEG_SS         2
#define SEG_DS         3
#define SEG_FS         4
#define SEG_GS         5

// Command highlighting.
#define DRAW_PLAIN     '.' //0x0000000C      // Plain commands
#define DRAW_JUMP      '>' //0x0000000D      // Unconditional jump commands
#define DRAW_CJMP      '?' //0x0000000E      // Conditional jump commands
#define DRAW_PUSHPOP   '=' //0x0000000F      // PUSH/POP commands
#define DRAW_CALL      '@' //0x00000010      // CALL commands
#define DRAW_RET       '<' //0x00000011      // RET commands
#define DRAW_FPU       '1' //0x00000012      // FPU, MMX, 3DNow! and SSE commands
#define DRAW_SUSPECT   '!' //0x00000013      // Bad, system and privileged commands
// Operand highlighting.
#define DRAW_IREG      'R' //0x00000018      // General purpose registers
#define DRAW_FREG      'F' //0x00000019      // FPU, MMX and SSE registers
#define DRAW_SYSREG    'S' //0x0000001A      // Segment and system registers
#define DRAW_STKMEM    'K' //0x0000001B      // Memory accessed over ESP or EBP
#define DRAW_MEM       'M' //0x0000001C      // Any other memory
#define DRAW_CONST     'C' //0x0000001E      // Constant

#define D_NONE         0x00000000      // No special features
// General type of command, only one is allowed.
#define D_CMDTYPE      0x0000001F      // Mask to extract type of command
#define   D_CMD        0x00000000      // Ordinary (none of listed below)
#define   D_MOV        0x00000001      // Move to or from integer register
#define   D_MOVC       0x00000002      // Conditional move to integer register
#define   D_SETC       0x00000003      // Conditional set integer register
#define   D_TEST       0x00000004      // Used to test data (CMP, TEST, AND...)
#define   D_STRING     0x00000005      // String command with REPxxx prefix
#define   D_JMP        0x00000006      // Unconditional near jump
#define   D_JMPFAR     0x00000007      // Unconditional far jump
#define   D_JMC        0x00000008      // Conditional jump on flags
#define   D_JMCX       0x00000009      // Conditional jump on (E)CX (and flags)
#define   D_PUSH       0x0000000A      // PUSH exactly 1 (d)word of data
#define   D_POP        0x0000000B      // POP exactly 1 (d)word of data
#define   D_CALL       0x0000000C      // Plain near call
#define   D_CALLFAR    0x0000000D      // Far call
#define   D_INT        0x0000000E      // Interrupt
#define   D_RET        0x0000000F      // Plain near return from call
#define   D_RETFAR     0x00000010      // Far return or IRET
#define   D_FPU        0x00000011      // FPU command
#define   D_MMX        0x00000012      // MMX instruction, incl. SSE extensions
#define   D_3DNOW      0x00000013      // 3DNow! instruction
#define   D_SSE        0x00000014      // SSE instruction
#define   D_IO         0x00000015      // Accesses I/O ports
#define   D_SYS        0x00000016      // Legal but useful in system code only
#define   D_PRIVILEGED 0x00000017      // Privileged (non-Ring3) command
#define   D_AVX        0x00000018      // AVX instruction
#define   D_XOP        0x00000019      // AMD instruction with XOP prefix
#define   D_DATA       0x0000001C      // Data recognized by Analyser
#define   D_PSEUDO     0x0000001D      // Pseudocommand, for search models only
#define   D_PREFIX     0x0000001E      // Standalone prefix
#define   D_BAD        0x0000001F      // Bad or unrecognized command
// Additional parts of the command.
#define D_SIZE01       0x00000020      // Bit 0x01 in last cmd is data size
#define D_POSTBYTE     0x00000040      // Command continues in postbyte
// For string commands, either long or short form can be selected.
#define D_LONGFORM     0x00000080      // Long form of string command
// Decoding of some commands depends on data or address size.
#define D_SIZEMASK     0x00000F00      // Mask for data/address size dependence
#define   D_DATA16     0x00000100      // Requires 16-bit data size
#define   D_DATA32     0x00000200      // Requires 32-bit data size
#define   D_ADDR16     0x00000400      // Requires 16-bit address size
#define   D_ADDR32     0x00000800      // Requires 32-bit address size
// Prefixes that command may, must or must not possess.
#define D_MUSTMASK     0x0000F000      // Mask for fixed set of prefixes
#define   D_NOMUST     0x00000000      // No obligatory prefixes (default)
#define   D_MUST66     0x00001000      // (SSE,AVX) Requires 66, no F2 or F3
#define   D_MUSTF2     0x00002000      // (SSE,AVX) Requires F2, no 66 or F3
#define   D_MUSTF3     0x00003000      // (SSE,AVX) Requires F3, no 66 or F2
#define   D_MUSTNONE   0x00004000      // (MMX,SSE,AVX) Requires no 66, F2, F3
#define   D_NEEDF2     0x00005000      // (SSE,AVX) Requires F2, no F3
#define   D_NEEDF3     0x00006000      // (SSE,AVX) Requires F3, no F2
#define   D_NOREP      0x00007000      // Must not include F2 or F3
#define   D_MUSTREP    0x00008000      // Must include F3 (REP)
#define   D_MUSTREPE   0x00009000      // Must include F3 (REPE)
#define   D_MUSTREPNE  0x0000A000      // Must include F2 (REPNE)
#define D_LOCKABLE     0x00010000      // Allows for F0 (LOCK, memory only)
#define D_BHINT        0x00020000      // Allows for branch hints (2E, 3E)
// Decoding of some commands with ModRM-SIB depends whether register or memory.
#define D_MEMORY       0x00040000      // Mod field must indicate memory
#define D_REGISTER     0x00080000      // Mod field must indicate register
// Side effects caused by command.
#define D_FLAGMASK     0x00700000      // Mask to extract modified flags
#define   D_NOFLAGS    0x00000000      // Flags S,Z,P,O,C remain unchanged
#define   D_ALLFLAGS   0x00100000      // Modifies flags S,Z,P,O,C
#define   D_FLAGZ      0x00200000      // Modifies flag Z only
#define   D_FLAGC      0x00300000      // Modifies flag C only
#define   D_FLAGSCO    0x00400000      // Modifies flag C and O only
#define   D_FLAGD      0x00500000      // Modifies flag D only
#define   D_FLAGSZPC   0x00600000      // Modifies flags Z, P and C only (FPU)
#define   D_NOCFLAG    0x00700000      // S,Z,P,O modified, C unaffected
#define D_FPUMASK      0x01800000      // Mask for effects on FPU stack
#define   D_FPUSAME    0x00000000      // Doesn't rotate FPU stack (default)
#define   D_FPUPOP     0x00800000      // Pops FPU stack
#define   D_FPUPOP2    0x01000000      // Pops FPU stack twice
#define   D_FPUPUSH    0x01800000      // Pushes FPU stack
#define D_CHGESP       0x02000000      // Command indirectly modifies ESP
// Command features.
#define D_HLADIR       0x04000000      // Nonstandard order of operands in HLA
#define D_WILDCARD     0x08000000      // Mnemonics contains W/D wildcard ('*')
#define D_COND         0x10000000      // Conditional (action depends on flags)
#define D_USESCARRY    0x20000000      // Uses Carry flag
#define D_USEMASK      0xC0000000      // Mask to detect unusual commands
#define   D_RARE       0x40000000      // Rare or obsolete in Win32 apps
#define   D_SUSPICIOUS 0x80000000      // Suspicious command
#define   D_UNDOC      0xC0000000      // Undocumented command

// Extension of D_xxx.
#define DX_ZEROMASK    0x00000003      // How to decode FLAGS.Z flag
#define   DX_JE        0x00000001      // JE, JNE instead of JZ, JNZ
#define   DX_JZ        0x00000002      // JZ, JNZ instead of JE, JNE
#define DX_CARRYMASK   0x0000000C      // How to decode FLAGS.C flag
#define   DX_JB        0x00000004      // JAE, JB instead of JC, JNC
#define   DX_JC        0x00000008      // JC, JNC instead of JAE, JB
#define DX_RETN        0x00000010      // The mnemonics is RETN
#define DX_VEX         0x00000100      // Requires VEX prefix
#define DX_VLMASK      0x00000600      // Mask to extract VEX operand length
#define   DX_LSHORT    0x00000000      // 128-bit only
#define   DX_LBOTH     0x00000200      // Both 128- and 256-bit versions
#define   DX_LLONG     0x00000400      // 256-bit only
#define   DX_IGNOREL   0x00000600      // Ignore VEX.L
#define DX_NOVREG      0x00000800      // VEX.vvvv must be set to all 1's
#define DX_VWMASK      0x00003000      // Mask to extract VEX.W
#define   DX_W0        0x00001000      // VEX.W must be 0
#define   DX_W1        0x00002000      // VEX.W must be 1
#define DX_LEADMASK    0x00070000      // Mask to extract leading opcode bytes
#define   DX_LEAD0F    0x00000000      // Implied 0F leading byte (default)
#define   DX_LEAD38    0x00010000      // Implied 0F 38 leading opcode bytes
#define   DX_LEAD3A    0x00020000      // Implied 0F 3A leading opcode bytes
#define DX_WONKYTRAP   0x00800000      // Don't single-step this command
#define DX_TYPEMASK    0xFF000000      // Precised command type mask
#define   DX_ADD       0x01000000      // The command is integer ADD
#define   DX_SUB       0x02000000      // The command is integer SUB
#define   DX_LEA       0x03000000      // The command is LEA
#define   DX_NOP       0x04000000      // The command is NOP

//#define DX_LVEX        (DX_VEX|DX_LBOTH)
//#define DX_GVEX        (DX_VEX|DX_LLONG)

// Type of operand, only one is allowed. Size of SSE operands is given for the
// case of 128-bit operations and usually doubles for 256-bit AVX commands. If
// B_NOVEXSIZE is set, memory may double but XMM registers are not promoted to
// YMM.
#define B_ARGMASK      0x000000FF      // Mask to extract type of argument
#define   B_NONE       0x00000000      // Operand absent
#define   B_AL         0x00000001      // Register AL
#define   B_AH         0x00000002      // Register AH
#define   B_AX         0x00000003      // Register AX
#define   B_CL         0x00000004      // Register CL
#define   B_CX         0x00000005      // Register CX
#define   B_DX         0x00000006      // Register DX
#define   B_DXPORT     0x00000007      // Register DX as I/O port address
#define   B_EAX        0x00000008      // Register EAX
#define   B_EBX        0x00000009      // Register EBX
#define   B_ECX        0x0000000A      // Register ECX
#define   B_EDX        0x0000000B      // Register EDX
#define   B_ACC        0x0000000C      // Accumulator (AL/AX/EAX)
#define   B_STRCNT     0x0000000D      // Register CX or ECX as REPxx counter
#define   B_DXEDX      0x0000000E      // Register DX or EDX in DIV/MUL
#define   B_BPEBP      0x0000000F      // Register BP or EBP in ENTER/LEAVE
#define   B_REG        0x00000010      // 8/16/32-bit register in Reg
#define   B_REG16      0x00000011      // 16-bit register in Reg
#define   B_REG32      0x00000012      // 32-bit register in Reg
#define   B_REGCMD     0x00000013      // 16/32-bit register in last cmd byte
#define   B_REGCMD8    0x00000014      // 8-bit register in last cmd byte
#define   B_ANYREG     0x00000015      // Reg field is unused, any allowed
#define   B_INT        0x00000016      // 8/16/32-bit register/memory in ModRM
#define   B_INT8       0x00000017      // 8-bit register/memory in ModRM
#define   B_INT16      0x00000018      // 16-bit register/memory in ModRM
#define   B_INT32      0x00000019      // 32-bit register/memory in ModRM
#define   B_INT1632    0x0000001A      // 16/32-bit register/memory in ModRM
#define   B_INT64      0x0000001B      // 64-bit integer in ModRM, memory only
#define   B_INT128     0x0000001C      // 128-bit integer in ModRM, memory only
#define   B_IMMINT     0x0000001D      // 8/16/32-bit int at immediate addr
#define   B_INTPAIR    0x0000001E      // Two signed 16/32 in ModRM, memory only
#define   B_SEGOFFS    0x0000001F      // 16:16/16:32 absolute address in memory
#define   B_STRDEST    0x00000020      // 8/16/32-bit string dest, [ES:(E)DI]
#define   B_STRDEST8   0x00000021      // 8-bit string destination, [ES:(E)DI]
#define   B_STRSRC     0x00000022      // 8/16/32-bit string source, [(E)SI]
#define   B_STRSRC8    0x00000023      // 8-bit string source, [(E)SI]
#define   B_XLATMEM    0x00000024      // 8-bit memory in XLAT, [(E)BX+AL]
#define   B_EAXMEM     0x00000025      // Reference to memory addressed by [EAX]
#define   B_LONGDATA   0x00000026      // Long data in ModRM, mem only
#define   B_ANYMEM     0x00000027      // Reference to memory, data unimportant
#define   B_STKTOP     0x00000028      // 16/32-bit int top of stack
#define   B_STKTOPFAR  0x00000029      // Top of stack (16:16/16:32 far addr)
#define   B_STKTOPEFL  0x0000002A      // 16/32-bit flags on top of stack
#define   B_STKTOPA    0x0000002B      // 16/32-bit top of stack all registers
#define   B_PUSH       0x0000002C      // 16/32-bit int push to stack
#define   B_PUSHRET    0x0000002D      // 16/32-bit push of return address
#define   B_PUSHRETF   0x0000002E      // 16:16/16:32-bit push of far retaddr
#define   B_PUSHA      0x0000002F      // 16/32-bit push all registers
#define   B_EBPMEM     0x00000030      // 16/32-bit int at [EBP]
#define   B_SEG        0x00000031      // Segment register in Reg
#define   B_SEGNOCS    0x00000032      // Segment register in Reg, but not CS
#define   B_SEGCS      0x00000033      // Segment register CS
#define   B_SEGDS      0x00000034      // Segment register DS
#define   B_SEGES      0x00000035      // Segment register ES
#define   B_SEGFS      0x00000036      // Segment register FS
#define   B_SEGGS      0x00000037      // Segment register GS
#define   B_SEGSS      0x00000038      // Segment register SS
#define   B_ST         0x00000039      // 80-bit FPU register in last cmd byte
#define   B_ST0        0x0000003A      // 80-bit FPU register ST0
#define   B_ST1        0x0000003B      // 80-bit FPU register ST1
#define   B_FLOAT32    0x0000003C      // 32-bit float in ModRM, memory only
#define   B_FLOAT64    0x0000003D      // 64-bit float in ModRM, memory only
#define   B_FLOAT80    0x0000003E      // 80-bit float in ModRM, memory only
#define   B_BCD        0x0000003F      // 80-bit BCD in ModRM, memory only
#define   B_MREG8x8    0x00000040      // MMX register as 8 8-bit integers
#define   B_MMX8x8     0x00000041      // MMX reg/memory as 8 8-bit integers
#define   B_MMX8x8DI   0x00000042      // MMX 8 8-bit integers at [DS:(E)DI]
#define   B_MREG16x4   0x00000043      // MMX register as 4 16-bit integers
#define   B_MMX16x4    0x00000044      // MMX reg/memory as 4 16-bit integers
#define   B_MREG32x2   0x00000045      // MMX register as 2 32-bit integers
#define   B_MMX32x2    0x00000046      // MMX reg/memory as 2 32-bit integers
#define   B_MREG64     0x00000047      // MMX register as 1 64-bit integer
#define   B_MMX64      0x00000048      // MMX reg/memory as 1 64-bit integer
#define   B_3DREG      0x00000049      // 3DNow! register as 2 32-bit floats
#define   B_3DNOW      0x0000004A      // 3DNow! reg/memory as 2 32-bit floats
#define   B_XMM0I32x4  0x0000004B      // XMM0 as 4 32-bit integers
#define   B_XMM0I64x2  0x0000004C      // XMM0 as 2 64-bit integers
#define   B_XMM0I8x16  0x0000004D      // XMM0 as 16 8-bit integers
#define   B_SREGF32x4  0x0000004E      // SSE register as 4 32-bit floats
#define   B_SREGF32L   0x0000004F      // Low 32-bit float in SSE register
#define   B_SREGF32x2L 0x00000050      // Low 2 32-bit floats in SSE register
#define   B_SSEF32x4   0x00000051      // SSE reg/memory as 4 32-bit floats
#define   B_SSEF32L    0x00000052      // Low 32-bit float in SSE reg/memory
#define   B_SSEF32x2L  0x00000053      // Low 2 32-bit floats in SSE reg/memory
#define   B_SREGF64x2  0x00000054      // SSE register as 2 64-bit floats
#define   B_SREGF64L   0x00000055      // Low 64-bit float in SSE register
#define   B_SSEF64x2   0x00000056      // SSE reg/memory as 2 64-bit floats
#define   B_SSEF64L    0x00000057      // Low 64-bit float in SSE reg/memory
#define   B_SREGI8x16  0x00000058      // SSE register as 16 8-bit sigints
#define   B_SSEI8x16   0x00000059      // SSE reg/memory as 16 8-bit sigints
#define   B_SSEI8x16DI 0x0000005A      // SSE 16 8-bit sigints at [DS:(E)DI]
#define   B_SSEI8x8L   0x0000005B      // Low 8 8-bit ints in SSE reg/memory
#define   B_SSEI8x4L   0x0000005C      // Low 4 8-bit ints in SSE reg/memory
#define   B_SSEI8x2L   0x0000005D      // Low 2 8-bit ints in SSE reg/memory
#define   B_SREGI16x8  0x0000005E      // SSE register as 8 16-bit sigints
#define   B_SSEI16x8   0x0000005F      // SSE reg/memory as 8 16-bit sigints
#define   B_SSEI16x4L  0x00000060      // Low 4 16-bit ints in SSE reg/memory
#define   B_SSEI16x2L  0x00000061      // Low 2 16-bit ints in SSE reg/memory
#define   B_SREGI32x4  0x00000062      // SSE register as 4 32-bit sigints
#define   B_SREGI32L   0x00000063      // Low 32-bit sigint in SSE register
#define   B_SREGI32x2L 0x00000064      // Low 2 32-bit sigints in SSE register
#define   B_SSEI32x4   0x00000065      // SSE reg/memory as 4 32-bit sigints
#define   B_SSEI32x2L  0x00000066      // Low 2 32-bit sigints in SSE reg/memory
#define   B_SREGI64x2  0x00000067      // SSE register as 2 64-bit sigints
#define   B_SSEI64x2   0x00000068      // SSE reg/memory as 2 64-bit sigints
#define   B_SREGI64L   0x00000069      // Low 64-bit sigint in SSE register
#define   B_EFL        0x0000006A      // Flags register EFL
#define   B_FLAGS8     0x0000006B      // Flags (low byte)
#define   B_OFFSET     0x0000006C      // 16/32 const offset from next command
#define   B_BYTEOFFS   0x0000006D      // 8-bit sxt const offset from next cmd
#define   B_FARCONST   0x0000006E      // 16:16/16:32 absolute address constant
#define   B_DESCR      0x0000006F      // 16:32 descriptor in ModRM
#define   B_1          0x00000070      // Immediate constant 1
#define   B_CONST8     0x00000071      // Immediate 8-bit constant
#define   B_CONST8_2   0x00000072      // Immediate 8-bit const, second in cmd
#define   B_CONST16    0x00000073      // Immediate 16-bit constant
#define   B_CONST      0x00000074      // Immediate 8/16/32-bit constant
#define   B_CONSTL     0x00000075      // Immediate 16/32-bit constant
#define   B_SXTCONST   0x00000076      // Immediate 8-bit sign-extended to size
#define   B_CR         0x00000077      // Control register in Reg
#define   B_CR0        0x00000078      // Control register CR0
#define   B_DR         0x00000079      // Debug register in Reg
#define   B_FST        0x0000007A      // FPU status register
#define   B_FCW        0x0000007B      // FPU control register
#define   B_MXCSR      0x0000007C      // SSE media control and status register
#define   B_SVEXF32x4  0x0000007D      // SSE reg in VEX as 4 32-bit floats
#define   B_SVEXF32L   0x0000007E      // Low 32-bit float in SSE in VEX
#define   B_SVEXF64x2  0x0000007F      // SSE reg in VEX as 2 64-bit floats
#define   B_SVEXF64L   0x00000080      // Low 64-bit float in SSE in VEX
#define   B_SVEXI8x16  0x00000081      // SSE reg in VEX as 16 8-bit sigints
#define   B_SVEXI16x8  0x00000082      // SSE reg in VEX as 8 16-bit sigints
#define   B_SVEXI32x4  0x00000083      // SSE reg in VEX as 4 32-bit sigints
#define   B_SVEXI64x2  0x00000084      // SSE reg in VEX as 2 64-bit sigints
#define   B_SIMMI8x16  0x00000085      // SSE reg in immediate 8-bit constant
// Type modifiers, used for interpretation of contents, only one is allowed.
#define B_MODMASK      0x000F0000      // Mask to extract type modifier
#define   B_NONSPEC    0x00000000      // Non-specific operand
#define   B_UNSIGNED   0x00010000      // Decode as unsigned decimal
#define   B_SIGNED     0x00020000      // Decode as signed decimal
#define   B_BINARY     0x00030000      // Decode as binary (full hex) data
#define   B_BITCNT     0x00040000      // Bit count
#define   B_SHIFTCNT   0x00050000      // Shift count
#define   B_COUNT      0x00060000      // General-purpose count
#define   B_NOADDR     0x00070000      // Not an address
#define   B_JMPCALL    0x00080000      // Near jump/call/return destination
#define   B_JMPCALLFAR 0x00090000      // Far jump/call/return destination
#define   B_STACKINC   0x000A0000      // Unsigned stack increment/decrement
#define   B_PORT       0x000B0000      // I/O port
#define   B_ADDR       0x000F0000      // Used internally
// Validity markers.
#define B_MEMORY       0x00100000      // Memory only, reg version different
#define B_REGISTER     0x00200000      // Register only, mem version different
#define B_MEMONLY      0x00400000      // Warn if operand in register
#define B_REGONLY      0x00800000      // Warn if operand in memory
#define B_32BITONLY    0x01000000      // Warn if 16-bit operand
#define B_NOESP        0x02000000      // ESP is not allowed
// Miscellaneous options.
#define B_NOVEXSIZE    0x04000000      // Always 128-bit SSE in 256-bit AVX
#define B_SHOWSIZE     0x08000000      // Always show argument size in disasm
#define B_CHG          0x10000000      // Changed, old contents is not used
#define B_UPD          0x20000000      // Modified using old contents
#define B_PSEUDO       0x40000000      // Pseoudooperand, not in assembler cmd
#define B_NOSEG        0x80000000      // Don't add offset of selector

// Location of operand, only one bit is allowed.
#define OP_SOMEREG     0x000000FF      // Mask for any kind of register
#define   OP_REGISTER  0x00000001      // Operand is a general-purpose register
#define   OP_SEGREG    0x00000002      // Operand is a segment register
#define   OP_FPUREG    0x00000004      // Operand is a FPU register
#define   OP_MMXREG    0x00000008      // Operand is a MMX register
#define   OP_3DNOWREG  0x00000010      // Operand is a 3DNow! register
#define   OP_SSEREG    0x00000020      // Operand is a SSE register
#define   OP_CREG      0x00000040      // Operand is a control register
#define   OP_DREG      0x00000080      // Operand is a debug register
#define OP_MEMORY      0x00000100      // Operand is in memory
#define OP_CONST       0x00000200      // Operand is an immediate constant
// Additional operand properties.
#define OP_PORT        0x00000400      // Used to access I/O port
#define OP_OTHERREG    0x00000800      // Special register like EFL or MXCSR
#define OP_INVALID     0x00001000      // Invalid operand, like reg in mem-only
#define OP_PSEUDO      0x00002000      // Pseudooperand (not in mnenonics)
#define OP_MOD         0x00004000      // Command may change/update operand
#define OP_MODREG      0x00008000      // Memory, but modifies reg (POP,MOVSD)
#define OP_IMPORT      0x00020000      // Value imported from different module
#define OP_SELECTOR    0x00040000      // Includes immediate selector
// Additional properties of memory address.
#define OP_INDEXED     0x00080000      // Memory address contains registers
#define OP_OPCONST     0x00100000      // Memory address contains constant
#define OP_ADDR16      0x00200000      // 16-bit memory address
#define OP_ADDR32      0x00400000      // Explicit 32-bit memory address

#define DAMODE_MASM    0               // MASM assembling/disassembling style
#define DAMODE_IDEAL   1               // IDEAL assembling/disassembling style
#define DAMODE_HLA     2               // HLA assembling/disassembling style
#define DAMODE_ATT     3               // AT&T disassembling style

#define NUM_STYLE     0x0003           // Mask to extract hex style
#define   NUM_STD     0x0000           // 123, 12345678h, 0ABCD1234h
#define   NUM_X       0x0001           // 123, 0x12345678, 0xABCD1234
#define   NUM_OLLY    0x0002           // 123., 12345678, 0ABCD1234
#define NUM_LONG      0x0010           // 00001234h instead of 1234h
#define NUM_DECIMAL   0x0020           // 123 instead of 7Bh if under DECLIMIT

// Disassembling options.
#define DA_TEXT        0x00000001      // Decode command to text and comment
#define   DA_HILITE    0x00000002      // Use syntax highlighting
#define   DA_JZ        0x00000004      // JZ, JNZ instead of JE, JNE
#define   DA_JC        0x00000008      // JC, JNC instead of JAE, JB
#define DA_DUMP        0x00000020      // Dump command to hexadecimal text
#define DA_PSEUDO      0x00000400      // List pseudooperands

// Disassembling errors.
#define DAE_NOERR      0x00000000      // No errors
#define DAE_BADCMD     0x00000001      // Unrecognized command
#define DAE_CROSS      0x00000002      // Command crosses end of memory block
#define DAE_MEMORY     0x00000004      // Register where only memory allowed
#define DAE_REGISTER   0x00000008      // Memory where only register allowed
#define DAE_LOCK       0x00000010      // LOCK prefix is not allowed
#define DAE_BADSEG     0x00000020      // Invalid segment register
#define DAE_SAMEPREF   0x00000040      // Two prefixes from the same group
#define DAE_MANYPREF   0x00000080      // More than 4 prefixes
#define DAE_BADCR      0x00000100      // Invalid CR register
#define DAE_INTERN     0x00000200      // Internal error

// Disassembling warnings.
#define DAW_NOWARN     0x00000000      // No warnings
#define DAW_DATASIZE   0x00000001      // Superfluous data size prefix
#define DAW_ADDRSIZE   0x00000002      // Superfluous address size prefix
#define DAW_SEGPREFIX  0x00000004      // Superfluous segment override prefix
#define DAW_REPPREFIX  0x00000008      // Superfluous REPxx prefix
#define DAW_DEFSEG     0x00000010      // Segment prefix coincides with default
#define DAW_JMP16      0x00000020      // 16-bit jump, call or return
#define DAW_FARADDR    0x00000040      // Far jump or call
#define DAW_SEGMOD     0x00000080      // Modifies segment register
#define DAW_PRIV       0x00000100      // Privileged command
#define DAW_IO         0x00000200      // I/O command
#define DAW_SHIFT      0x00000400      // Shift out of range 1..31
#define DAW_LOCK       0x00000800      // Command with valid LOCK prefix
#define DAW_STACK      0x00001000      // Unaligned stack operation
#define DAW_NOESP      0x00002000      // Suspicious use of stack pointer
#define DAW_RARE       0x00004000      // Rare, seldom used command
#define DAW_NONCLASS   0x00008000      // Non-standard or non-documented code
#define DAW_INTERRUPT  0x00010000      // Interrupt command

// List of prefixes.
#define PF_SEGMASK     0x0000003F      // Mask for segment override prefixes
#define   PF_ES        0x00000001      // 0x26, ES segment override
#define   PF_CS        0x00000002      // 0x2E, CS segment override
#define   PF_SS        0x00000004      // 0x36, SS segment override
#define   PF_DS        0x00000008      // 0x3E, DS segment override
#define   PF_FS        0x00000010      // 0x64, FS segment override
#define   PF_GS        0x00000020      // 0x65, GS segment override
#define PF_DSIZE       0x00000040      // 0x66, data size override
#define PF_ASIZE       0x00000080      // 0x67, address size override
#define PF_LOCK        0x00000100      // 0xF0, bus lock
#define PF_REPMASK     0x00000600      // Mask for repeat prefixes
#define   PF_REPNE     0x00000200      // 0xF2, REPNE prefix
#define   PF_REP       0x00000400      // 0xF3, REP/REPE prefix
#define PF_BYTE        0x00000800      // Size bit in command, used in cmdexec
#define PF_MUSTMASK    D_MUSTMASK      // Necessary prefixes, used in t_asmmod
#define PF_VEX2        0x00010000      // 2-byte VEX prefix
#define PF_VEX3        0x00020000      // 3-byte VEX prefix
// Useful shortcuts.
#define PF_66          PF_DSIZE        // Alternative names for SSE prefixes
#define PF_F2          PF_REPNE
#define PF_F3          PF_REP
#define PF_HINT        (PF_CS|PF_DS)   // Alternative names for branch hints
#define   PF_NOTTAKEN  PF_CS
#define   PF_TAKEN     PF_DS
#define PF_VEX         (PF_VEX2|PF_VEX3)

typedef struct t_modrm {               // ModRM byte decoding
  ulong          size;                 // Total size with SIB and disp, bytes
  struct t_modrm *psib;                // Pointer to SIB table or NULL
  ulong          dispsize;             // Size of displacement or 0 if none
  ulong          features;             // Operand features, set of OP_xxx
  int            reg;                  // Register index or REG_UNDEF
  int            defseg;               // Default selector (SEG_xxx)
  uchar          scale[NREG];          // Scales of registers in memory address
  ulong          aregs;                // List of registers used in address
  int            basereg;              // Register used as base or REG_UNDEF
  tchar          ardec[SHORTNAME];     // Register part of address, INTEL fmt
  tchar          aratt[SHORTNAME];     // Register part of address, AT&T fmt
} t_modrm;

typedef struct t_bincmd {              // Description of 80x86 command
  tchar          *name;                // Symbolic name for this command
  ulong          cmdtype;              // Command's features, set of D_xxx
  ulong          exttype;              // More features, set of DX_xxx
  ulong          length;               // Length of main code (before ModRM/SIB)
  ulong          mask;                 // Mask for first 4 bytes of the command
  ulong          code;                 // Compare masked bytes with this
  ulong          postbyte;             // Postbyte
  ulong          arg[NOPERAND];        // Types of arguments, set of B_xxx
} t_bincmd;

typedef struct t_chain {               // Element of command chain
  const t_bincmd *pcmd;                // Pointer to command descriptor or NULL
  struct t_chain *pnext;               // Pointer to next element in chain
} t_chain;

typedef struct t_config {              // Disassembler configuration
  int            disasmmode;           // Main style, one of DAMODE_xxx
  int            memmode;              // Constant part of address, NUM_xxx
  int            jmpmode;              // Jump/call destination, NUM_xxx
  int            binconstmode;         // Binary constants, NUM_xxx
  int            constmode;            // Numeric constants, NUM_xxx
  int            lowercase;            // Force lowercase display
  int            tabarguments;         // Tab between mnemonic and arguments
  int            extraspace;           // Extra space between arguments
  int            useretform;           // Use RET instead of RETN
  int            shortstringcmds;      // Use short form of string commands
  int            putdefseg;            // Display default segments in listing
  int            showmemsize;          // Always show memory size
  int            shownear;             // Show NEAR modifiers
  int            ssesizemode;          // How to decode size of SSE operands
  int            jumphintmode;         // How to decode jump hints
  int            sizesens;             // How to decode size-sensitive mnemonics
  int            simplifiedst;         // How to decode top of FPU stack
  int            hiliteoperands;       // Highlight operands
} t_config;

typedef struct t_operand {             // Description of disassembled operand
  // Description of operand.
  ulong          features;             // Operand features, set of OP_xxx
  ulong          arg;                  // Operand type, set of B_xxx
//  int            optype;               // DEC_INT, DEC_FLOAT or DEC_UNKNOWN
  int            opsize;               // Total size of data, bytes
  int            granularity;          // Size of element (opsize exc. MMX/SSE)
  int            reg;                  // REG_xxx (also ESP in POP) or REG_UNDEF
  ulong          uses;                 // List of used regs (not in address!)
  ulong          modifies;             // List of modified regs (not in addr!)
  // Description of memory address.
  int            seg;                  // Selector (SEG_xxx)
  uchar          scale[NREG];          // Scales of registers in memory address
  ulong          aregs;                // List of registers used in address
  ulong          opconst;              // Constant or const part of address
  ulong          selector;             // Immediate selector in far jump/call
  // Textual decoding.
  tchar          text[TEXTLEN];        // Operand, decoded to text
} t_operand;

// Note that used registers are those which contents is necessary to create
// result. Modified registers are those which value is changed. For example,
// command MOV EAX,[EBX+ECX] uses EBX and ECX and modifies EAX. Command
// ADD ESI,EDI uses ESI and EDI and modifies ESI.
typedef struct t_disasm {              // Disassembled command
  ulong          ip;                   // Address of first command byte
  ulong          size;                 // Full length of command, bytes
  ulong          cmdtype;              // Type of command, D_xxx
  ulong          exttype;              // More features, set of DX_xxx
  ulong          prefixes;             // List of prefixes, set of PF_xxx
  ulong          nprefix;              // Number of prefixes, including SSE2
  ulong          memfixup;             // Offset of first 4-byte fixup or -1
  ulong          immfixup;             // Offset of second 4-byte fixup or -1
  int            errors;               // Set of DAE_xxx
  int            warnings;             // Set of DAW_xxx
  ulong          uses;                 // List of used registers
  ulong          modifies;             // List of modified registers
  ulong          memconst;             // Constant in memory address or 0
  ulong          stackinc;             // Data size in ENTER/RETN/RETF
  t_operand      op[NOPERAND];         // Operands
  tchar          dump[TEXTLEN];        // Hex dump of the command
  tchar          result[TEXTLEN];      // Fully decoded command as text
  uchar          mask[TEXTLEN];        // Mask to highlight result
  int            masksize;             // Length of mask corresponding to result
} t_disasm;

extern t_modrm   modrm16[256];         // 16-bit ModRM decodings
extern t_modrm   modrm32[256];         // 32-bit ModRM decodings without SIB
extern t_modrm   sib0[256];            // ModRM-SIB decodings with Mod=00
extern t_modrm   sib1[256];            // ModRM-SIB decodings with Mod=01
extern t_modrm   sib2[256];            // ModRM-SIB decodings with Mod=10

extern const t_bincmd bincmd[];        // List of 80x86 commands
extern t_chain   *cmdchain;            // Commands sorted by first CMDMASK bits

int              Preparedisasm(void);
void             Finishdisasm(void);

ulong            Disasm(uchar const *cmd,ulong cmdsize,ulong cmdip,
                   t_disasm *da,int cmdmode,t_config *cmdconfig,
                   int (*decodeaddress)(tchar *s,ulong addr));
tchar            *Geterrwarnmessage(ulong errors,ulong warnings);

