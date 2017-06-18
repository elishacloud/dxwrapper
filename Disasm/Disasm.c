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


#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#pragma hdrstop

#pragma warning (disable : 4090 4245 4701 4703 4996)
#include "disasm.h"

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// SYMBOLIC NAMES ////////////////////////////////

// 8-bit register names, sorted by 'natural' index (as understood by CPU, not
// in the alphabetical order as some 'programmers' prefer).
const tchar *regname8[NREG] = {
  T("AL"),       T("CL"),       T("DL"),       T("BL"),
  T("AH"),       T("CH"),       T("DH"),       T("BH")  };

// 16-bit register names.
const tchar *regname16[NREG] = {
  T("AX"),       T("CX"),       T("DX"),       T("BX"),
  T("SP"),       T("BP"),       T("SI"),       T("DI")  };

// 32-bit register names.
const tchar *regname32[NREG] = {
  T("EAX"),      T("ECX"),      T("EDX"),      T("EBX"),
  T("ESP"),      T("EBP"),      T("ESI"),      T("EDI") };

// Names of segment registers.
const tchar *segname[NREG] = {
  T("ES"),       T("CS"),       T("SS"),       T("DS"),
  T("FS"),       T("GS"),       T("SEG6:"),    T("SEG7:") };

// Names of FPU registers, classical form.
const tchar *fpulong[NREG] = {
  T("ST(0)"),    T("ST(1)"),    T("ST(2)"),    T("ST(3)"),
  T("ST(4)"),    T("ST(5)"),    T("ST(6)"),    T("ST(7)") };

// Names of FPU registers, short form.
const tchar *fpushort[NREG] = {
  T("ST0"),      T("ST1"),      T("ST2"),      T("ST3"),
  T("ST4"),      T("ST5"),      T("ST6"),      T("ST7") };

// Names of MMX/3DNow! registers.
const tchar *mmxname[NREG] = {
  T("MM0"),      T("MM1"),      T("MM2"),      T("MM3"),
  T("MM4"),      T("MM5"),      T("MM6"),      T("MM7") };

// Names of 128-bit SSE registers.
const tchar *sse128[NREG] = {
  T("XMM0"),     T("XMM1"),     T("XMM2"),     T("XMM3"),
  T("XMM4"),     T("XMM5"),     T("XMM6"),     T("XMM7") };

// Names of 256-bit SSE registers.
const tchar *sse256[NREG] = {
  T("YMM0"),     T("YMM1"),     T("YMM2"),     T("YMM3"),
  T("YMM4"),     T("YMM5"),     T("YMM6"),     T("YMM7") };

// Names of control registers.
const tchar *crname[NREG] = {
  T("CR0"),      T("CR1"),      T("CR2"),      T("CR3"),
  T("CR4"),      T("CR5"),      T("CR6"),      T("CR7") };

// Names of debug registers.
const tchar *drname[NREG] = {
  T("DR0"),      T("DR1"),      T("DR2"),      T("DR3"),
  T("DR4"),      T("DR5"),      T("DR6"),      T("DR7") };

// Declarations for data types. Depending on ssesizemode, name of 16-byte data
// type (DQWORD) may be changed to XMMWORD and that of 32-bit type (QQWORD) to
// YMMWORD.
const tchar *sizename[33] = {
  NULL,          T("BYTE"),     T("WORD"),     NULL,
  T("DWORD"),    NULL,          T("FWORD"),    NULL,
  T("QWORD"),    NULL,          T("TBYTE"),    NULL,
  NULL,          NULL,          NULL,          NULL,
  T("DQWORD"),   NULL,          NULL,          NULL,
  NULL,          NULL,          NULL,          NULL,
  NULL,          NULL,          NULL,          NULL,
  NULL,          NULL,          NULL,          NULL,
  T("QQWORD") };

// Keywords for immediate data. HLA uses sizename[] instead of sizekey[].
const tchar *sizekey[33] = {
  NULL,          T("DB"),       T("DW"),       NULL,
  T("DD"),       NULL,          T("DF"),       NULL,
  T("DQ"),       NULL,          T("DT"),       NULL,
  NULL,          NULL,          NULL,          NULL,
  T("DDQ"),      NULL,          NULL,          NULL,
  NULL,          NULL,          NULL,          NULL,
  NULL,          NULL,          NULL,          NULL,
  NULL,          NULL,          NULL,          NULL,
  T("DQQ") };

// Keywords for immediate data in AT&T format.
const tchar *sizeatt[33] = {
  NULL,          T(".BYTE"),    T(".WORD"),    NULL,
  T(".LONG"),    NULL,          T(".FWORD"),   NULL,
  T(".QUAD"),    NULL,          T(".TBYTE"),   NULL,
  NULL,          NULL,          NULL,          NULL,
  T(".DQUAD"),   NULL,          NULL,          NULL,
  NULL,          NULL,          NULL,          NULL,
  NULL,          NULL,          NULL,          NULL,
  NULL,          NULL,          NULL,          NULL,
  T(".QQUAD") };

// Comparison predicates in SSE [0..7] and VEX commands [0..31].
const tchar *ssepredicate[32] = {
  T("EQ"),       T("LT"),       T("LE"),       T("UNORD"),
  T("NEQ"),      T("NLT"),      T("NLE"),      T("ORD"),
  T("EQ_UQ"),    T("NGE"),      T("NGT"),      T("FALSE"),
  T("NEQ_OQ"),   T("GE"),       T("GT"),       T("TRUE"),
  T("EQ_OS"),    T("LT_OQ"),    T("LE_OQ"),    T("UNORD_S"),
  T("NEQ_US"),   T("NLT_UQ"),   T("NLE_UQ"),   T("ORD_S"),
  T("EQ_US"),    T("NGE_UQ"),   T("NGT_UQ"),   T("FALSE_OS"),
  T("NEQ_OS"),   T("GE_OQ"),    T("GT_OQ"),    T("TRUE_US") };


////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// DISASSEMBLER /////////////////////////////////

typedef struct t_imdata {              // Intermediate disassembler data
  t_disasm       *da;                  // Result of disassembly
  ulong          damode;               // Disassembling mode, set of DA_xxx
  t_config       *config;              // Disassembler configuration
  int            (*decodeaddress)(tchar *s,ulong addr);
  ulong          prefixlist;           // List of command's prefixes, PF_xxx
  int            ssesize;              // Size of SSE operands (16/32 bytes)
  ulong          immsize1;             // Size of first immediate constant
  ulong          immsize2;             // Size of second immediate constant
  ulong          mainsize;             // Size of command with prefixes
  ulong          modsize;              // Size of ModRegRM/SIB bytes
  ulong          dispsize;             // Size of address offset
  int            usesdatasize;         // May have data size prefix
  int            usesaddrsize;         // May have address size prefix
  int            usessegment;          // May have segment override prefix
} t_imdata;

static t_config  defconfig = {         // Default disassembler configuration
  DAMODE_MASM,                         // Main style, one of DAMODE_xxx
  NUM_STD|NUM_DECIMAL,                 // Constant part of address, NUM_xxx
  NUM_STD|NUM_LONG,                    // Jump/call destination, NUM_xxx
  NUM_STD|NUM_LONG,                    // Binary constants, NUM_xxx
  NUM_STD|NUM_DECIMAL,                 // Numeric constants, NUM_xxx
  0,                                   // Force lowercase display
  0,                                   // Tab between mnemonic and arguments
  0,                                   // Extra space between arguments
  0,                                   // Use RET instead of RETN
  1,                                   // Use short form of string commands
  0,                                   // Display default segments in listing
  1,                                   // Always show memory size
  0,                                   // Show NEAR modifiers
  1,                                   // How to decode size of SSE operands
  0,                                   // How to decode jump hints
  0,                                   // How to decode size-sensitive mnemonics
  0,                                   // How to decode top of FPU stack
  0                                    // Highlight operands
};

static t_config  attconfig = {         // AT&T disassembler configuration
  DAMODE_ATT,                          // Main style, one of DAMODE_xxx
  NUM_X|NUM_DECIMAL,                   // Constant part of address, NUM_xxx
  NUM_X|NUM_LONG,                      // Jump/call destination, NUM_xxx
  NUM_X|NUM_LONG,                      // Binary constants, NUM_xxx
  NUM_X|NUM_DECIMAL,                   // Numeric constants, NUM_xxx
  1,                                   // Force lowercase display
  1,                                   // Tab between mnemonic and arguments
  1,                                   // Extra space between arguments
  0,                                   // Use RET instead of RETN
  1,                                   // Use short form of string commands
  0,                                   // Display default segments in listing
  0,                                   // Always show memory size
  0,                                   // Show NEAR modifiers
  1,                                   // How to decode size of SSE operands
  0,                                   // How to decode jump hints
  0,                                   // How to decode size-sensitive mnemonics
  0,                                   // How to decode top of FPU stack
  0                                    // Highlight operands
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////// SERVICE FUNCTIONS ///////////////////////////////

static tchar hexcharu[16] = {          // Nibble-to-hexdigit table, uppercase
  T('0'), T('1'), T('2'), T('3'), T('4'), T('5'), T('6'), T('7'),
  T('8'), T('9'), T('A'), T('B'), T('C'), T('D'), T('E'), T('F') };

static tchar hexcharl[16] = {          // Nibble-to-hexdigit table, lowercase
  T('0'), T('1'), T('2'), T('3'), T('4'), T('5'), T('6'), T('7'),
  T('8'), T('9'), T('a'), T('b'), T('c'), T('d'), T('e'), T('f') };

static tchar cvtlower[256];

// Copies at most n-1 wide characters from src to dest and assures that dest is
// null-terminated. Slow but reliable. Returns number of copied characters, not
// including the terminal null. Attention, does not check that input parameters
// are correct!
static int Tstrcopy(tchar *dest,int n,const tchar *src) {
  int i;
  if (n<=0)
    return 0;
  for (i=0; i<n-1; i++) {
    if (*src==T('\0')) break;
    *dest++=*src++; };
  *dest=T('\0');
  return i;
};

// Copies at most n-1 wide characters from src to dest and assures that dest is
// null-terminated. If lowercase is 1, simultaneously converts it to lower
// case. Slow but reliable. Returns number of copied characters, not including
// the terminal null. Attention, does not check that input parameters are
// correct!
static int Tcopycase(tchar *dest,int n,const tchar *src,int lowercase) {
  int i;
  if (n<=0)
    return 0;
  for (i=0; i<n-1; i++) {
    if (*src==T('\0')) break;
    if (lowercase)
      *dest++=cvtlower[*src++];        // Much faster than call to tolower()
    else
      *dest++=*src++;
    ;
  };
  *dest=T('\0');
  return i;
};

// Dumps ncode bytes of code to the string s. Returns length of resulting text,
// characters, not including terminal zero. Attention, does not check that
// input parameters are correct or that s has sufficient length!
static int Thexdump(tchar *s,uchar *code,int ncode,int lowercase) {
  int d,n;
  static tchar *hexchar;
  hexchar=(lowercase?hexcharl:hexcharu);
  n=0;
  while (ncode>0) {
    d=*code++;
    s[n++]=hexchar[(d>>4) & 0x0F];
    s[n++]=hexchar[d & 0x0F];
    ncode--;
  };
  s[n]=T('\0');
  return n;
};

// Converts unsigned 1-, 2- or 4-byte number to hexadecimal text, according to
// the specified mode and type of argument. String s must be at least SHORTNAME
// characters long. Returns length of resulting text in characters, not
// including the terminal zero.
static int Hexprint(int size,tchar *s,ulong u,const t_imdata *im,ulong arg) {
  int i,k,ndigit,lastdigit;
  ulong nummode,mod;
  tchar buf[SHORTNAME];
  static tchar *hexchar;
  if (size==1)
    u&=0x000000FF;                     // 8-bit number
  else if (size==2)
    u&=0x0000FFFF;                     // 16-bit number
  else
    size=4;                            // Correct possible errors
  mod=arg & B_MODMASK;
  if (mod==B_ADDR)
    nummode=im->config->memmode;
  else if (mod==B_JMPCALL || mod==B_JMPCALLFAR)
    nummode=im->config->jmpmode;
  else if (mod==B_BINARY)
    nummode=im->config->binconstmode;
  else
    nummode=im->config->constmode;
  hexchar=(im->config->lowercase?hexcharl:hexcharu);
  buf[SHORTNAME-1]=T('\0');
  k=SHORTNAME-1;
  if ((nummode & NUM_DECIMAL)!=0 && (mod==B_SIGNED || mod==B_UNSIGNED ||
    (u<DECLIMIT && mod!=B_BINARY && mod!=B_JMPCALL && mod!=B_JMPCALLFAR))
  ) {
    // Decode as decimal unsigned number.
    if ((nummode & NUM_STYLE)==NUM_OLLY && u>=10)
      buf[--k]=T('.');                 // Period marks decimals in OllyDbg
    do {
      buf[--k]=hexchar[u%10];
      u/=10;
    } while (u!=0); }
  else {
    // Decode as hexadecimal number.
    if (nummode & NUM_LONG)            // 2, 4 or 8 significant digits
      ndigit=size*2;
    else
      ndigit=1;
    if ((nummode & NUM_STYLE)==NUM_STD)
      buf[--k]=T('h');
    for (i=0; i<ndigit || u!=0; i++) {
      lastdigit=u & 0x0F;
      buf[--k]=hexchar[lastdigit];
      u=(u>>4) & 0x0FFFFFFF; };
    if ((nummode & NUM_STYLE)==NUM_X) {
      buf[--k]=T('x');
      buf[--k]=T('0'); }
    else if (lastdigit>=10 &&
      ((nummode & NUM_STYLE)!=NUM_OLLY || i<(mod==B_BINARY?size*2:8)))
      buf[--k]=T('0');
    ;
  };
  return Tstrcopy(s,SHORTNAME,buf+k);
};


////////////////////////////////////////////////////////////////////////////////
///////////////////////// INTERNAL DISASSEMBLER TABLES /////////////////////////

t_chain          *cmdchain;            // Commands sorted by first CMDMASK bits
t_modrm          modrm16[256];         // 16-bit ModRM decodings
t_modrm          modrm32[256];         // 32-bit ModRM decodings without SIB
t_modrm          sib0[256];            // ModRM-SIB decodings with Mod=00
t_modrm          sib1[256];            // ModRM-SIB decodings with Mod=01
t_modrm          sib2[256];            // ModRM-SIB decodings with Mod=10

// Initializes disassembler tables. Call this function once during startup.
// Returns 0 on success and -1 if initialization was unsuccessful. In the last
// case, continuation is not possible and program must terminate.
int Preparedisasm(void) {
  int n,c,reg,sreg,scale,nchain;
  ulong u,code,mask;
  const t_bincmd *pcmd;
  t_chain *pchain;
  t_modrm *pmrm,*psib;
  if (cmdchain!=NULL)
    return 0;                          // Already initialized
  // Sort command descriptors into command chains by first CMDMASK bits.
  cmdchain=(t_chain *)malloc(NCHAIN*sizeof(t_chain));
  if (cmdchain==NULL)                  // Low memory
    return -1;
  memset(cmdchain,0,NCHAIN*sizeof(t_chain));
  nchain=CMDMASK+1;                    // Number of command chains
  for (pcmd=bincmd; pcmd->length!=0; pcmd++) {
    if ((pcmd->cmdtype & D_CMDTYPE)==D_PSEUDO)
      continue;                        // Pseudocommand, for search models only
    code=pcmd->code;
    mask=pcmd->mask & CMDMASK;
    for (u=0; u<CMDMASK+1; u++) {
      if (((u ^ code) & mask)!=0)
        continue;                      // Command has different first bytes
      pchain=cmdchain+u;
      while (pchain->pcmd!=NULL && pchain->pnext!=NULL)
        pchain=pchain->pnext;          // Walk chain to the end
      if (pchain->pcmd==NULL)
        pchain->pcmd=pcmd;
      else if (nchain>=NCHAIN)
        return -1;                     // Too many commands
      else {
        pchain->pnext=cmdchain+nchain; // Prolongate chain
        pchain=pchain->pnext;
        pchain->pcmd=pcmd;
        nchain++;
      };
    };
  };
  // Prepare 16-bit ModRM decodings.
  memset(modrm16,0,sizeof(modrm16));
  for (c=0x00,pmrm=modrm16; c<=0xFF; c++,pmrm++) {
    reg=c & 0x07;
    if ((c & 0xC0)==0xC0) {
      // Register in ModRM.
      pmrm->size=1;
      pmrm->features=0;                // Register, its type as yet unknown
      pmrm->reg=reg;
      pmrm->defseg=SEG_UNDEF;
      pmrm->basereg=REG_UNDEF; }
    else if ((c & 0xC7)==0x06) {
      // Special case of immediate address.
      pmrm->size=3;
      pmrm->dispsize=2;
      pmrm->features=OP_MEMORY|OP_OPCONST|OP_ADDR16;
      pmrm->reg=REG_UNDEF;
      pmrm->defseg=SEG_DS;
      pmrm->basereg=REG_UNDEF; }
    else {
      pmrm->features=OP_MEMORY|OP_INDEXED|OP_ADDR16;
      if ((c & 0xC0)==0x40) {
        pmrm->dispsize=1; pmrm->features|=OP_OPCONST; }
      else if ((c & 0xC0)==0x80) {
        pmrm->dispsize=2; pmrm->features|=OP_OPCONST; };
      pmrm->size=pmrm->dispsize+1;
      pmrm->reg=REG_UNDEF;
      switch (reg) {
        case 0:
          pmrm->scale[REG_EBX]=1; pmrm->scale[REG_ESI]=1;
          pmrm->defseg=SEG_DS;
          tstrcpy(pmrm->ardec,T("BX+SI"));
          tstrcpy(pmrm->aratt,T("%BX,%SI"));
          pmrm->aregs=(1<<REG_EBX)|(1<<REG_ESI);
          pmrm->basereg=REG_ESI; break;
        case 1:
          pmrm->scale[REG_EBX]=1; pmrm->scale[REG_EDI]=1;
          pmrm->defseg=SEG_DS;
          tstrcpy(pmrm->ardec,T("BX+DI"));
          tstrcpy(pmrm->aratt,T("%BX,%DI"));
          pmrm->aregs=(1<<REG_EBX)|(1<<REG_EDI);
          pmrm->basereg=REG_EDI; break;
        case 2:
          pmrm->scale[REG_EBP]=1; pmrm->scale[REG_ESI]=1;
          pmrm->defseg=SEG_SS;
          tstrcpy(pmrm->ardec,T("BP+SI"));
          tstrcpy(pmrm->aratt,T("%BP,%SI"));
          pmrm->aregs=(1<<REG_EBP)|(1<<REG_ESI);
          pmrm->basereg=REG_ESI; break;
        case 3:
          pmrm->scale[REG_EBP]=1; pmrm->scale[REG_EDI]=1;
          pmrm->defseg=SEG_SS;
          tstrcpy(pmrm->ardec,T("BP+DI"));
          tstrcpy(pmrm->aratt,T("%BP,%DI"));
          pmrm->aregs=(1<<REG_EBP)|(1<<REG_EDI);
          pmrm->basereg=REG_EDI; break;
        case 4:
          pmrm->scale[REG_ESI]=1;
          pmrm->defseg=SEG_DS;
          tstrcpy(pmrm->ardec,T("SI"));
          tstrcpy(pmrm->aratt,T("%SI"));
          pmrm->aregs=(1<<REG_ESI);
          pmrm->basereg=REG_ESI; break;
        case 5:
          pmrm->scale[REG_EDI]=1;
          pmrm->defseg=SEG_DS;
          tstrcpy(pmrm->ardec,T("DI"));
          tstrcpy(pmrm->aratt,T("%DI"));
          pmrm->aregs=(1<<REG_EDI);
          pmrm->basereg=REG_EDI; break;
        case 6:
          pmrm->scale[REG_EBP]=1;
          pmrm->defseg=SEG_SS;
          tstrcpy(pmrm->ardec,T("BP"));
          tstrcpy(pmrm->aratt,T("%BP"));
          pmrm->aregs=(1<<REG_EBP);
          pmrm->basereg=REG_EBP; break;
        case 7:
          pmrm->scale[REG_EBX]=1;
          pmrm->defseg=SEG_DS;
          tstrcpy(pmrm->ardec,T("BX"));
          tstrcpy(pmrm->aratt,T("%BX"));
          pmrm->aregs=(1<<REG_EBX);
          pmrm->basereg=REG_EBX;
        break;
      };
    };
  };
  // Prepare 32-bit ModRM decodings without SIB.
  memset(modrm32,0,sizeof(modrm32));
  for (c=0x00,pmrm=modrm32; c<=0xFF; c++,pmrm++) {
    reg=c & 0x07;
    if ((c & 0xC0)==0xC0) {
      // Register in ModRM.
      pmrm->size=1;
      pmrm->features=0;                // Register, its type as yet unknown
      pmrm->reg=reg;
      pmrm->defseg=SEG_UNDEF;
      pmrm->basereg=REG_UNDEF; }
    else if ((c & 0xC7)==0x05) {
      // Special case of 32-bit immediate address.
      pmrm->size=5;
      pmrm->dispsize=4;
      pmrm->features=OP_MEMORY|OP_OPCONST;
      pmrm->reg=REG_UNDEF;
      pmrm->defseg=SEG_DS;
      pmrm->basereg=REG_UNDEF; }
    else {
      // Regular memory address.
      pmrm->features=OP_MEMORY;
      pmrm->reg=REG_UNDEF;
      if ((c & 0xC0)==0x40) {
        pmrm->dispsize=1;              // 8-bit sign-extended displacement
        pmrm->features|=OP_OPCONST; }
      else if ((c & 0xC0)==0x80) {
        pmrm->dispsize=4;              // 32-bit displacement
        pmrm->features|=OP_OPCONST; };
      if (reg==REG_ESP) {
        // SIB byte follows, decode with sib32.
        if ((c & 0xC0)==0x00) pmrm->psib=sib0;
        else if ((c & 0xC0)==0x40) pmrm->psib=sib1;
        else pmrm->psib=sib2;
        pmrm->basereg=REG_UNDEF; }
      else {
        pmrm->size=1+pmrm->dispsize;
        pmrm->features|=OP_INDEXED;
        pmrm->defseg=(reg==REG_EBP?SEG_SS:SEG_DS);
        pmrm->scale[reg]=1;
        tstrcpy(pmrm->ardec,regname32[reg]);
        pmrm->aratt[0]=T('%');
        Tstrcopy(pmrm->aratt+1,SHORTNAME-1,regname32[reg]);
        pmrm->aregs=(1<<reg);
        pmrm->basereg=reg;
      };
    };
  };
  // Prepare 32-bit ModRM decodings with SIB, case Mod=00: usually no disp.
  memset(sib0,0,sizeof(sib0));
  for (c=0x00,psib=sib0; c<=0xFF; c++,psib++) {
    psib->features=OP_MEMORY;
    psib->reg=REG_UNDEF;
    reg=c & 0x07;
    sreg=(c>>3) & 0x07;
    if ((c & 0xC0)==0) scale=1;
    else if ((c & 0xC0)==0x40) scale=2;
    else if ((c & 0xC0)==0x80) scale=4;
    else scale=8;
    if (sreg!=REG_ESP) {
      psib->scale[sreg]=(uchar)scale;
      n=Tstrcopy(psib->ardec,SHORTNAME,regname32[sreg]);
      psib->aregs=(1<<sreg);
      psib->features|=OP_INDEXED;
      if (scale>1) {
        psib->ardec[n++]=T('*');
        psib->ardec[n++]=(tchar)(T('0')+scale);
        psib->ardec[n]=T('\0');
      }; }
    else
      n=0;
    if (reg==REG_EBP) {
      psib->size=6;
      psib->dispsize=4;
      psib->features|=OP_OPCONST;
      psib->defseg=SEG_DS;
      psib->basereg=REG_UNDEF; }
    else {
      psib->size=2;
      psib->defseg=((reg==REG_ESP || reg==REG_EBP)?SEG_SS:SEG_DS);
      psib->scale[reg]++;
      psib->features|=OP_INDEXED;
      if (n!=0) psib->ardec[n++]=T('+');
      Tstrcopy(psib->ardec+n,SHORTNAME-n,regname32[reg]);
      psib->aregs|=(1<<reg);
      psib->basereg=reg; };
    if (reg!=REG_EBP) {
      psib->aratt[0]=T('%'); n=1;
      n+=Tstrcopy(psib->aratt+n,SHORTNAME-n,regname32[reg]); }
    else
      n=0;
    if (sreg!=REG_ESP) {
      n+=Tstrcopy(psib->aratt+n,SHORTNAME-n,T(",%"));
      n+=Tstrcopy(psib->aratt+n,SHORTNAME-n,regname32[sreg]);
      if (scale>1) {
        psib->aratt[n++]=T(',');
        psib->aratt[n++]=(tchar)(T('0')+scale);
        psib->aratt[n]=T('\0');
      };
    };
  };
  // Prepare 32-bit ModRM decodings with SIB, case Mod=01: 8-bit displacement.
  memset(sib1,0,sizeof(sib1));
  for (c=0x00,psib=sib1; c<=0xFF; c++,psib++) {
    psib->features=OP_MEMORY|OP_INDEXED|OP_OPCONST;
    psib->reg=REG_UNDEF;
    reg=c & 0x07;
    sreg=(c>>3) & 0x07;
    if ((c & 0xC0)==0) scale=1;
    else if ((c & 0xC0)==0x40) scale=2;
    else if ((c & 0xC0)==0x80) scale=4;
    else scale=8;
    psib->size=3;
    psib->dispsize=1;
    psib->defseg=((reg==REG_ESP || reg==REG_EBP)?SEG_SS:SEG_DS);
    psib->scale[reg]=1;
    psib->basereg=reg;
    psib->aregs=(1<<reg);
    if (sreg!=REG_ESP) {
      psib->scale[sreg]+=(uchar)scale;
      n=Tstrcopy(psib->ardec,SHORTNAME,regname32[sreg]);
      psib->aregs|=(1<<sreg);
      if (scale>1) {
        psib->ardec[n++]=T('*');
        psib->ardec[n++]=(tchar)(T('0')+scale);
      }; }
    else
      n=0;
    if (n!=0) psib->ardec[n++]=T('+');
    Tstrcopy(psib->ardec+n,SHORTNAME-n,regname32[reg]);
    psib->aratt[0]=T('%'); n=1;
    n+=Tstrcopy(psib->aratt+n,SHORTNAME-n,regname32[reg]);
    if (sreg!=REG_ESP) {
      n+=Tstrcopy(psib->aratt+n,SHORTNAME-n,T(",%"));
      n+=Tstrcopy(psib->aratt+n,SHORTNAME-n,regname32[sreg]);
      if (scale>1) {
        psib->aratt[n++]=T(',');
        psib->aratt[n++]=(tchar)(T('0')+scale);
        psib->aratt[n]=T('\0');
      };
    };
  };
  // Prepare 32-bit ModRM decodings with SIB, case Mod=10: 32-bit displacement.
  memset(sib2,0,sizeof(sib2));
  for (c=0x00,psib=sib2; c<=0xFF; c++,psib++) {
    psib->features=OP_MEMORY|OP_INDEXED|OP_OPCONST;
    psib->reg=REG_UNDEF;
    reg=c & 0x07;
    sreg=(c>>3) & 0x07;
    if ((c & 0xC0)==0) scale=1;
    else if ((c & 0xC0)==0x40) scale=2;
    else if ((c & 0xC0)==0x80) scale=4;
    else scale=8;
    psib->size=6;
    psib->dispsize=4;
    psib->defseg=((reg==REG_ESP || reg==REG_EBP)?SEG_SS:SEG_DS);
    psib->scale[reg]=1;
    psib->basereg=reg;
    psib->aregs=(1<<reg);
    if (sreg!=REG_ESP) {
      psib->scale[sreg]+=(uchar)scale;
      n=Tstrcopy(psib->ardec,SHORTNAME,regname32[sreg]);
      psib->aregs|=(1<<sreg);
      if (scale>1) {
        psib->ardec[n++]=T('*');
        psib->ardec[n++]=(tchar)(T('0')+scale);
      }; }
    else
      n=0;
    if (n!=0) psib->ardec[n++]=T('+');
    Tstrcopy(psib->ardec+n,SHORTNAME-n,regname32[reg]);
    psib->aratt[0]=T('%'); n=1;
    n+=Tstrcopy(psib->aratt+n,SHORTNAME-n,regname32[reg]);
    if (sreg!=REG_ESP) {
      n+=Tstrcopy(psib->aratt+n,SHORTNAME-n,T(",%"));
      n+=Tstrcopy(psib->aratt+n,SHORTNAME-n,regname32[sreg]);
      if (scale>1) {
        psib->aratt[n++]=T(',');
        psib->aratt[n++]=(tchar)(T('0')+scale);
        psib->aratt[n]=T('\0');
      };
    };
  };
  // Fill lowercase conversion table. This table replaces tolower(). When
  // compiled with Borland C++ Builder, spares significant time.
  for (c=0; c<256; c++)
    cvtlower[c]=(tchar)ttolower(c);
  // Report success.
  return 0;
};

// Frees resources allocated by Preparedisasm(). Call this function once
// during shutdown after disassembling service is no longer necessary.
void Finishdisasm(void) {
  if (cmdchain!=NULL) {
    free(cmdchain);
    cmdchain=NULL;
  };
};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////// AUXILIARY ROUTINES //////////////////////////////

// Given index of byte register, returns index of 32-bit container.
static int Byteregtodwordreg(int bytereg) {
  if (bytereg<0 || bytereg>=NREG)
    return REG_UNDEF;
  if (bytereg>=4)
    return bytereg-4;
  return bytereg;
};

// Checks prefix override flags and generates warnings if prefix is superfluous.
// Returns index of segment register. Note that Disasm() assures that two
// segment override bits in im->prefixlist can't be set simultaneously.
static int Getsegment(t_imdata *im,int arg,int defseg) {
  if ((im->prefixlist & PF_SEGMASK)==0)
    return defseg;                     // Optimization for most frequent case
  switch (im->prefixlist & PF_SEGMASK) {
    case PF_ES:
      if (defseg==SEG_ES) im->da->warnings|=DAW_DEFSEG;
      if (arg & B_NOSEG) im->da->warnings|=DAW_SEGPREFIX;
      return SEG_ES;
    case PF_CS:
      if (defseg==SEG_CS) im->da->warnings|=DAW_DEFSEG;
      if (arg & B_NOSEG) im->da->warnings|=DAW_SEGPREFIX;
      return SEG_CS;
    case PF_SS:
      if (defseg==SEG_SS) im->da->warnings|=DAW_DEFSEG;
      if (arg & B_NOSEG) im->da->warnings|=DAW_SEGPREFIX;
      return SEG_SS;
    case PF_DS:
      if (defseg==SEG_DS) im->da->warnings|=DAW_DEFSEG;
      if (arg & B_NOSEG) im->da->warnings|=DAW_SEGPREFIX;
      return SEG_DS;
    case PF_FS:
      if (defseg==SEG_FS) im->da->warnings|=DAW_DEFSEG;
      if (arg & B_NOSEG) im->da->warnings|=DAW_SEGPREFIX;
      return SEG_FS;
    case PF_GS:
      if (defseg==SEG_GS) im->da->warnings|=DAW_DEFSEG;
      if (arg & B_NOSEG) im->da->warnings|=DAW_SEGPREFIX;
      return SEG_GS;
    default: return defseg;            // Most frequent case of default segment
  };
};

// Decodes generalized memory address to text.
static void Memaddrtotext(t_imdata *im,int arg,int datasize,int seg,
  const tchar *regpart,long constpart,tchar *s) {
  int n;
  tchar label[TEXTLEN];
  if (im->config->disasmmode==DAMODE_ATT) {
    // AT&T memory address syntax is so different from Intel that I process it
    // separately from the rest.
    n=0;
    if ((arg & B_MODMASK)==B_JMPCALL)
      s[n++]=T('*');
    // On request, I show only explicit segments.
    if ((im->config->putdefseg && (arg & B_NOSEG)==0) ||
      (im->prefixlist & PF_SEGMASK)!=0
    ) {
      s[n++]=T('%');
      n+=Tcopycase(s+n,TEXTLEN-n,segname[seg],im->config->lowercase);
      s[n++]=T(':'); };
    // Add constant part (offset).
    if (constpart<0 && constpart>NEGLIMIT) {
      s[n++]=T('-');
      n+=Hexprint((im->prefixlist & PF_ASIZE?2:4),s+n,-constpart,im,B_ADDR); }
    else if (constpart!=0) {
      if (seg!=SEG_FS && seg!=SEG_GS &&
        im->decodeaddress!=NULL &&
        im->decodeaddress(label,constpart)!=0)
        n+=Tstrcopy(s+n,TEXTLEN-n,label);
      else
        n+=Hexprint((im->prefixlist & PF_ASIZE?2:4),s+n,constpart,im,B_ADDR);
      ;
    };
    // Add register part of address, may be absent.
    if (regpart[0]!=T('\0')) {
      n+=Tstrcopy(s+n,TEXTLEN-n,T("("));
      n+=Tcopycase(s+n,TEXTLEN-n,regpart,im->config->lowercase);
      n+=Tstrcopy(s+n,TEXTLEN-n,T(")"));
    }; }
  else {
    // Mark far and near jump/call addresses.
    if ((arg & B_MODMASK)==B_JMPCALLFAR)
      n=Tcopycase(s,TEXTLEN,T("FAR "),im->config->lowercase);
    else if (im->config->shownear && (arg & B_MODMASK)==B_JMPCALL)
      n=Tcopycase(s,TEXTLEN,T("NEAR "),im->config->lowercase);
    else
      n=0;
    if (im->config->disasmmode!=DAMODE_MASM) {
      s[n++]=T('[');
      if ((im->prefixlist & PF_ASIZE)!=0 && regpart[0]==T('\0'))
        n+=Tcopycase(s+n,TEXTLEN-n,T("SMALL "),im->config->lowercase);
      ;
    };
    // If operand is longer than 32 bytes or of type B_ANYMEM (memory contents
    // unimportant), its size is not displayed. Otherwise, bit B_SHOWSIZE
    // indicates that explicit operand's size can't be omitted.
    if (datasize<=32 && (arg & B_ARGMASK)!=B_ANYMEM &&
      (im->config->showmemsize!=0 || (arg & B_SHOWSIZE)!=0)
    ) {
      if (im->config->disasmmode==DAMODE_HLA)
        n+=Tcopycase(s+n,TEXTLEN-n,T("TYPE "),im->config->lowercase);
      if ((arg & B_ARGMASK)==B_INTPAIR && im->config->disasmmode==DAMODE_IDEAL){
        // If operand is a pair of integers (BOUND), Borland in IDEAL mode
        // expects size of single integer, whereas MASM requires size of the
        // whole pair.
        n+=Tcopycase(s+n,TEXTLEN-n,sizename[datasize/2],im->config->lowercase);
        s[n++]=T(' '); }
      else if (datasize==16 && im->config->ssesizemode==1)
        n+=Tcopycase(s+n,TEXTLEN-n,T("XMMWORD "),im->config->lowercase);
      else if (datasize==32 && im->config->ssesizemode==1)
        n+=Tcopycase(s+n,TEXTLEN-n,T("YMMWORD "),im->config->lowercase);
      else {
        n+=Tcopycase(s+n,TEXTLEN-n,sizename[datasize],im->config->lowercase);
        s[n++]=T(' '); };
      if (im->config->disasmmode==DAMODE_MASM)
        n+=Tcopycase(s+n,TEXTLEN-n,T("PTR "),im->config->lowercase);
      ;
    };
    // On request, I show only explicit segments.
    if ((im->config->putdefseg && (arg & B_NOSEG)==0) ||
      (im->prefixlist & PF_SEGMASK)!=0
    ) {
      n+=Tcopycase(s+n,TEXTLEN-n,segname[seg],im->config->lowercase);
      s[n++]=T(':'); };
    if (im->config->disasmmode==DAMODE_MASM) {
      s[n++]=T('[');
      if ((im->prefixlist & PF_ASIZE)!=0 && regpart[0]==T('\0'))
        n+=Tcopycase(s+n,TEXTLEN-n,T("SMALL "),im->config->lowercase);
      ;
    };
    // Add register part of address, may be absent.
    if (regpart[0]!=T('\0'))
      n+=Tcopycase(s+n,TEXTLEN-n,regpart,im->config->lowercase);
    if (regpart[0]!=T('\0') && constpart<0 && constpart>NEGLIMIT) {
      s[n++]=T('-');
      n+=Hexprint((im->prefixlist & PF_ASIZE?2:4),s+n,-constpart,im,B_ADDR); }
    else if (constpart!=0 || regpart[0]==T('\0')) {
      if (regpart[0]!=T('\0')) s[n++]=T('+');
      if (seg!=SEG_FS && seg!=SEG_GS &&
        im->decodeaddress!=NULL &&
        im->decodeaddress(label,constpart)!=0)
        n+=Tstrcopy(s+n,TEXTLEN-n,label);
      else
        n+=Hexprint((im->prefixlist & PF_ASIZE?2:4),s+n,constpart,im,B_ADDR);
      ;
    };
    n+=Tstrcopy(s+n,TEXTLEN-n,T("]"));
  };
  s[n]=T('\0');
};

// Service function, returns granularity of MMX, 3DNow! and SSE operands.
static int Getgranularity(ulong arg) {
  int granularity;
  switch (arg & B_ARGMASK) {
    case B_MREG8x8:                    // MMX register as 8 8-bit integers
    case B_MMX8x8:                     // MMX reg/memory as 8 8-bit integers
    case B_MMX8x8DI:                   // MMX 8 8-bit integers at [DS:(E)DI]
    case B_XMM0I8x16:                  // XMM0 as 16 8-bit integers
    case B_SREGI8x16:                  // SSE register as 16 8-bit sigints
    case B_SVEXI8x16:                  // SSE reg in VEX as 16 8-bit sigints
    case B_SIMMI8x16:                  // SSE reg in immediate 8-bit constant
    case B_SSEI8x16:                   // SSE reg/memory as 16 8-bit sigints
    case B_SSEI8x16DI:                 // SSE 16 8-bit sigints at [DS:(E)DI]
    case B_SSEI8x8L:                   // Low 8 8-bit ints in SSE reg/memory
    case B_SSEI8x4L:                   // Low 4 8-bit ints in SSE reg/memory
    case B_SSEI8x2L:                   // Low 2 8-bit ints in SSE reg/memory
      granularity=1; break;
    case B_MREG16x4:                   // MMX register as 4 16-bit integers
    case B_MMX16x4:                    // MMX reg/memory as 4 16-bit integers
    case B_SREGI16x8:                  // SSE register as 8 16-bit sigints
    case B_SVEXI16x8:                  // SSE reg in VEX as 8 16-bit sigints
    case B_SSEI16x8:                   // SSE reg/memory as 8 16-bit sigints
    case B_SSEI16x4L:                  // Low 4 16-bit ints in SSE reg/memory
    case B_SSEI16x2L:                  // Low 2 16-bit ints in SSE reg/memory
      granularity=2; break;
    case B_MREG32x2:                   // MMX register as 2 32-bit integers
    case B_MMX32x2:                    // MMX reg/memory as 2 32-bit integers
    case B_3DREG:                      // 3DNow! register as 2 32-bit floats
    case B_3DNOW:                      // 3DNow! reg/memory as 2 32-bit floats
    case B_SREGF32x4:                  // SSE register as 4 32-bit floats
    case B_SVEXF32x4:                  // SSE reg in VEX as 4 32-bit floats
    case B_SREGF32L:                   // Low 32-bit float in SSE register
    case B_SVEXF32L:                   // Low 32-bit float in SSE in VEX
    case B_SREGF32x2L:                 // Low 2 32-bit floats in SSE register
    case B_SSEF32x4:                   // SSE reg/memory as 4 32-bit floats
    case B_SSEF32L:                    // Low 32-bit float in SSE reg/memory
    case B_SSEF32x2L:                  // Low 2 32-bit floats in SSE reg/memory
      granularity=4; break;
    case B_XMM0I32x4:                  // XMM0 as 4 32-bit integers
    case B_SREGI32x4:                  // SSE register as 4 32-bit sigints
    case B_SVEXI32x4:                  // SSE reg in VEX as 4 32-bit sigints
    case B_SREGI32L:                   // Low 32-bit sigint in SSE register
    case B_SREGI32x2L:                 // Low 2 32-bit sigints in SSE register
    case B_SSEI32x4:                   // SSE reg/memory as 4 32-bit sigints
    case B_SSEI32x2L:                  // Low 2 32-bit sigints in SSE reg/memory
      granularity=4; break;
    case B_MREG64:                     // MMX register as 1 64-bit integer
    case B_MMX64:                      // MMX reg/memory as 1 64-bit integer
    case B_XMM0I64x2:                  // XMM0 as 2 64-bit integers
    case B_SREGF64x2:                  // SSE register as 2 64-bit floats
    case B_SVEXF64x2:                  // SSE reg in VEX as 2 64-bit floats
    case B_SREGF64L:                   // Low 64-bit float in SSE register
    case B_SVEXF64L:                   // Low 64-bit float in SSE in VEX
    case B_SSEF64x2:                   // SSE reg/memory as 2 64-bit floats
    case B_SSEF64L:                    // Low 64-bit float in SSE reg/memory
      granularity=8; break;
    case B_SREGI64x2:                  // SSE register as 2 64-bit sigints
    case B_SVEXI64x2:                  // SSE reg in VEX as 2 64-bit sigints
    case B_SSEI64x2:                   // SSE reg/memory as 2 64-bit sigints
    case B_SREGI64L:                   // Low 64-bit sigint in SSE register
      granularity=8; break;
    default:
      granularity=1;                   // Treat unknown ops as string of bytes
    break; };
  return granularity;
};


////////////////////////////////////////////////////////////////////////////////
////////////////////////// OPERAND DECODING ROUTINES ///////////////////////////

// Decodes 8/16/32-bit integer register operand. ATTENTION, calling routine
// must set usesdatasize and usesaddrsize by itself!
static void Operandintreg(t_imdata *im,ulong datasize,int index,t_operand *op) {
  int n,reg32;
  op->features=OP_REGISTER;
  op->opsize=op->granularity=datasize;
  op->reg=index;
  op->seg=SEG_UNDEF;
  // Add container register to lists of used and modified registers.
  if (datasize==1)
    reg32=Byteregtodwordreg(index);
  else
    reg32=index;
  if ((op->arg & B_CHG)==0) {
    op->uses=(1<<reg32);
    im->da->uses|=(1<<reg32); };
  if (op->arg & (B_CHG|B_UPD)) {
    op->modifies=(1<<reg32);
    im->da->modifies|=(1<<reg32); };
  // Warn if ESP is misused.
  if ((op->arg & B_NOESP)!=0 && reg32==REG_ESP)
    im->da->warnings|=DAW_NOESP;
  // Decode name of integer register.
  if (im->damode & DA_TEXT) {
    n=0;
    if (im->config->disasmmode==DAMODE_ATT) {
      if ((op->arg & B_MODMASK)==B_JMPCALL) op->text[n++]=T('*');
      op->text[n++]=T('%'); };
    if (datasize==4)                   // Most frequent case first
      Tcopycase(op->text+n,TEXTLEN-n,regname32[index],im->config->lowercase);
    else if (datasize==1)
      Tcopycase(op->text+n,TEXTLEN-n,regname8[index],im->config->lowercase);
    else                               // 16-bit registers are seldom
      Tcopycase(op->text+n,TEXTLEN-n,regname16[index],im->config->lowercase);
    ;
  };
};

// Decodes 16/32-bit memory address in ModRM/SIB bytes. Returns full length of
// address (ModRM+SIB+displacement) in bytes, 0 if ModRM indicates register
// operand and -1 on error. ATTENTION, calling routine must set usesdatasize,
// granularity (preset to datasize) and reg together with OP_MODREG by itself!
static int Operandmodrm(t_imdata *im,ulong datasize,uchar *cmd,ulong cmdsize,
  t_operand *op) {
  tchar *ardec;
  t_modrm *pmrm;
  if (cmdsize==0) {
    im->da->errors|=DAE_CROSS;         // Command crosses end of memory block
    return -1; };
  // Decode ModRM/SIB. Most of the work is already done in Preparedisasm(), we
  // only need to find corresponding t_modrm.
  if (im->prefixlist & PF_ASIZE) {
    pmrm=modrm16+cmd[0];               // 16-bit address
    im->modsize=1; }
  else {
    pmrm=modrm32+cmd[0];
    if (pmrm->psib==NULL)
      im->modsize=1;                   // No SIB byte
    else {
      if (cmdsize<2) {
        im->da->errors|=DAE_CROSS;     // Command crosses end of memory block
        return -1; };
      pmrm=pmrm->psib+cmd[1];
      im->modsize=2;                   // Both ModRM and SIB
    };
  };
  // Check whether ModRM indicates register operand and immediately return if
  // true. As a side effect, modsize is already set.
  if ((cmd[0] & 0xC0)==0xC0)
    return 0;
  // Operand in memory.
  op->opsize=datasize;
  op->granularity=datasize;            // Default, may be overriden later
  op->reg=REG_UNDEF;
  im->usesaddrsize=1;                  // Address size prefix is meaningful
  im->usessegment=1;                   // Segment override prefix is meaningful
  // Fetch precalculated t_modrm fields.
  op->features=pmrm->features;
  memcpy(op->scale,pmrm->scale,8);
  op->aregs=pmrm->aregs;
  im->da->uses|=pmrm->aregs;           // Mark registers used to form address
  // Get displacement, if any.
  im->dispsize=pmrm->dispsize;
  if (pmrm->dispsize!=0) {
    if (cmdsize<pmrm->size) {
      im->da->errors|=DAE_CROSS;       // Command crosses end of memory block
      return -1; };
    if (pmrm->dispsize==1)             // 8-bit displacement is sign-extended
      op->opconst=im->da->memconst=(signed char)cmd[im->modsize];
    else if (pmrm->dispsize==4) {      // 32-bit full displacement
      im->da->memfixup=
        im->mainsize+im->modsize;      // Possible 32-bit fixup
      op->opconst=im->da->memconst=*(ulong *)(cmd+im->modsize); }
    else                               // 16-bit displacement, very rare
      op->opconst=im->da->memconst=*(ushort *)(cmd+im->modsize);
    ;
  };
  // Get segment.
  op->seg=Getsegment(im,op->arg,pmrm->defseg);
  // Warn if memory contents is 16-bit jump/call destination.
  if (datasize==2 && (op->arg & B_MODMASK)==B_JMPCALL)
    im->da->warnings|=DAW_JMP16;
  // Decode memory operand to text, if requested.
  if (im->damode & DA_TEXT) {
    ardec=(im->config->disasmmode==DAMODE_ATT?pmrm->aratt:pmrm->ardec);
    Memaddrtotext(im,op->arg,datasize,op->seg,ardec,op->opconst,op->text);
  };
  return pmrm->size;
};

// Decodes 16/32-bit immediate address (used only for 8/16/32-bit memory-
// accumulator moves). ATTENTION, calling routine must set usesdatasize by
// itself!
static void Operandimmaddr(t_imdata *im,ulong datasize,uchar *cmd,
  ulong cmdsize,t_operand *op) {
  if (im->prefixlist & PF_ASIZE)
    im->dispsize=2;
  else
    im->dispsize=4;
  if (cmdsize<im->dispsize) {
    im->da->errors|=DAE_CROSS;         // Command crosses end of memory block
    return; };
  op->features=OP_MEMORY|OP_OPCONST;
  op->opsize=op->granularity=datasize;
  op->reg=REG_UNDEF;
  im->usesaddrsize=1;                  // Address size prefix is meaningful
  im->usessegment=1;                   // Segment override prefix is meaningful
  if (im->dispsize==4) {               // 32-bit immediate address
    // 32-bit address means possible fixup, calculate offset.
    im->da->memfixup=im->mainsize;
    op->opconst=im->da->memconst=*(ulong *)cmd; }
  else {                               // 16-bit immediate address, very rare
    op->opconst=im->da->memconst=*(ushort *)cmd;
    op->features|=OP_ADDR16; };
  // Get segment.
  op->seg=Getsegment(im,op->arg,SEG_DS);
  // Decode memory operand to text, if requested.
  if (im->damode & DA_TEXT)
    Memaddrtotext(im,op->arg,datasize,op->seg,T(""),op->opconst,op->text);
  ;
};

// Decodes simple register address ([reg16] or [reg32]). Flag changesreg must
// be 0 if register remains unchanged and 1 if it changes. If fixseg is set to
// SEG_UNDEF, assumes overridable DS:, otherwise assumes fixsegment that cannot
// be overriden with segment prefix. If fixaddrsize is 2 or 4, assumes 16- or
// 32-bit addressing only, otherwise uses default. ATTENTION, calling routine
// must set usesdatasize by itself!
static void Operandindirect(t_imdata *im,int index,int changesreg,int fixseg,
  int fixaddrsize,ulong datasize,t_operand *op) {
  int n;
  ulong originallist;
  tchar ardec[SHORTNAME];
  op->features=OP_MEMORY|OP_INDEXED;
  if (changesreg) {
    op->features|=OP_MODREG;
    op->reg=index;
    im->da->modifies|=(1<<index); }
  else
    op->reg=REG_UNDEF;
  if (fixaddrsize==2)
    op->features|=OP_ADDR16;
  else if (fixaddrsize==0) {
    im->usesaddrsize=1;                // Address size prefix is meaningful
    if (im->prefixlist & PF_ASIZE) {
      op->features|=OP_ADDR16;
      fixaddrsize=2;
    };
  };
  // Get segment.
  if (fixseg==SEG_UNDEF) {
    op->seg=Getsegment(im,op->arg,SEG_DS);
    im->usessegment=1; }               // Segment override prefix is meaningful
  else
    op->seg=fixseg;
  op->opsize=datasize;
  op->granularity=datasize;            // Default, may be overriden later
  op->scale[index]=1;
  op->aregs=(1<<index);
  im->da->uses|=(1<<index);
  // Warn if memory contents is 16-bit jump/call destination.
  if (datasize==2 && (op->arg & B_MODMASK)==B_JMPCALL)
    im->da->warnings|=DAW_JMP16;
  // Decode source operand to text, if requested.
  if (im->damode & DA_TEXT) {
    if (im->config->disasmmode==DAMODE_ATT) {
      ardec[0]=T('%'); n=1; }
    else
      n=0;
    if (fixaddrsize==2)
      Tstrcopy(ardec+n,SHORTNAME-n,regname16[index]);
    else
      Tstrcopy(ardec+n,SHORTNAME-n,regname32[index]);
    if (fixseg==SEG_UNDEF)
      Memaddrtotext(im,op->arg,datasize,op->seg,ardec,0,op->text);
    else {
      originallist=im->prefixlist;
      im->prefixlist&=~PF_SEGMASK;
      Memaddrtotext(im,op->arg,datasize,op->seg,ardec,0,op->text);
      im->prefixlist=originallist;
    };
  };
};

// Decodes XLAT source address ([(E)BX+AL]). Note that I set scale of EAX to 1,
// which is not exactly true. ATTENTION, calling routine must set usesdatasize
// by itself!
static void Operandxlat(t_imdata *im,t_operand *op) {
  tchar *ardec;
  op->features=OP_MEMORY|OP_INDEXED;
  if (im->prefixlist & PF_ASIZE)
    op->features|=OP_ADDR16;
  im->usesaddrsize=1;                  // Address size prefix is meaningful
  im->usessegment=1;                   // Segment override prefix is meaningful
  op->opsize=1;
  op->granularity=1;
  op->reg=REG_UNDEF;
  // Get segment.
  op->seg=Getsegment(im,op->arg,SEG_DS);
  op->scale[REG_EAX]=1;                // This is not correct!
  op->scale[REG_EBX]=1;
  op->aregs=(1<<REG_EAX)|(1<<REG_EBX);
  im->da->uses|=op->aregs;
  // Decode address to text, if requested.
  if (im->damode & DA_TEXT) {
    if (im->config->disasmmode==DAMODE_ATT)
      ardec=(im->prefixlist & PF_ASIZE?T("%BX,%AL"):T("%EBX,%AL"));
    else
      ardec=(im->prefixlist & PF_ASIZE?T("BX+AL"):T("EBX+AL"));
    Memaddrtotext(im,op->arg,1,op->seg,ardec,0,op->text);
  };
};

// Decodes stack pushes of any size, including implicit return address in
// CALLs. ATTENTION, calling routine must set usesdatasize by itself!
static void Operandpush(t_imdata *im,ulong datasize,t_operand *op) {
  int n,addrsize;
  ulong originallist;
  tchar ardec[SHORTNAME];
  op->features=OP_MEMORY|OP_INDEXED|OP_MODREG;
  op->reg=REG_ESP;
  op->aregs=(1<<REG_ESP);
  im->da->modifies|=op->aregs;
  im->usesaddrsize=1;                  // Address size prefix is meaningful
  if (im->prefixlist & PF_ASIZE) {
    op->features|=OP_ADDR16;
    addrsize=2; }
  else
    addrsize=4;                        // Flat model!
  op->seg=SEG_SS;
  if ((op->arg & B_ARGMASK)==B_PUSHA) {
    im->da->uses=0xFF;                 // Uses all general registers
    op->opsize=datasize*8; }
  else if ((op->arg & B_ARGMASK)==B_PUSHRETF) {
    im->da->uses|=op->aregs;
    op->opsize=datasize*2; }
  else {
    im->da->uses|=op->aregs;
    // Warn if memory contents is 16-bit jump/call destination.
    if (datasize==2 && (op->arg & B_MODMASK)==B_JMPCALL)
      im->da->warnings|=DAW_JMP16;
    op->opsize=datasize;
  };
  op->opconst=-(long)op->opsize;       // ESP is predecremented
  op->granularity=datasize;            // Default, may be overriden later
  op->scale[REG_ESP]=1;
  // Decode source operand to text, if requested.
  if (im->damode & DA_TEXT) {
    if (im->config->disasmmode==DAMODE_ATT) {
      ardec[0]=T('%'); n=1; }
    else
      n=0;
    if (addrsize==2)
      Tstrcopy(ardec+n,SHORTNAME-n,regname16[REG_ESP]);
    else
      Tstrcopy(ardec+n,SHORTNAME-n,regname32[REG_ESP]);
    originallist=im->prefixlist;
    im->prefixlist&=~PF_SEGMASK;
    Memaddrtotext(im,op->arg,datasize,op->seg,ardec,0,op->text);
    im->prefixlist=originallist;
  };
};

// Decodes segment register.
static void Operandsegreg(t_imdata *im,int index,t_operand *op) {
  int n;
  op->features=OP_SEGREG;
  if (index>=NSEG) {
    op->features|=OP_INVALID;          // Invalid segment register
    im->da->errors|=DAE_BADSEG; };
  op->opsize=op->granularity=2;
  op->reg=index;
  op->seg=SEG_UNDEF;                   // Because this is not a memory address
  if (op->arg & (B_CHG|B_UPD))
    im->da->warnings|=DAW_SEGMOD;      // Modifies segment register
  // Decode name of segment register.
  if (im->damode & DA_TEXT) {
    n=0;
    if (im->config->disasmmode==DAMODE_ATT) op->text[n++]=T('%');
    Tcopycase(op->text+n,TEXTLEN-n,segname[index],im->config->lowercase);
  };
};

// Decodes FPU register operand.
static void Operandfpureg(t_imdata *im,int index,t_operand *op) {
  op->features=OP_FPUREG;
  op->opsize=op->granularity=10;
  op->reg=index;
  op->seg=SEG_UNDEF;                   // Because this is not a memory address
  // Decode name of FPU register.
  if (im->damode & DA_TEXT) {
    if (im->config->disasmmode==DAMODE_ATT) {
      if (im->config->simplifiedst && index==0)
        Tcopycase(op->text,TEXTLEN,T("%ST"),im->config->lowercase);
      else {
        op->text[0]=T('%');
        Tcopycase(op->text+1,TEXTLEN-1,fpushort[index],im->config->lowercase);
      }; }
    else if (im->config->simplifiedst && index==0)
      Tcopycase(op->text,TEXTLEN,T("ST"),im->config->lowercase);
    else if (im->config->disasmmode!=DAMODE_HLA)
      Tcopycase(op->text,TEXTLEN,fpulong[index],im->config->lowercase);
    else
      Tcopycase(op->text,TEXTLEN,fpushort[index],im->config->lowercase);
    ;
  };
};

// Decodes MMX register operands. ATTENTION, does not set correct granularity!
static void Operandmmxreg(t_imdata *im,int index,t_operand *op) {
  int n;
  op->features=OP_MMXREG;
  op->opsize=8;
  op->granularity=4;                   // Default, correct it later!
  op->reg=index;
  op->seg=SEG_UNDEF;
  // Decode name of MMX/3DNow! register.
  if (im->damode & DA_TEXT) {
    n=0;
    if (im->config->disasmmode==DAMODE_ATT) op->text[n++]=T('%');
    Tcopycase(op->text+n,TEXTLEN-n,mmxname[index],im->config->lowercase);
  };
};

// Decodes 3DNow! register operands. ATTENTION, does not set correct
// granularity!
static void Operandnowreg(t_imdata *im,int index,t_operand *op) {
  int n;
  op->features=OP_3DNOWREG;
  op->opsize=8;
  op->granularity=4;                   // Default, correct it later!
  op->reg=index;
  op->seg=SEG_UNDEF;
  // Decode name of MMX/3DNow! register.
  if (im->damode & DA_TEXT) {
    n=0;
    if (im->config->disasmmode==DAMODE_ATT) op->text[n++]=T('%');
    Tcopycase(op->text+n,TEXTLEN-n,mmxname[index],im->config->lowercase);
  };
};

// Decodes SSE register operands. ATTENTION, does not set correct granularity!
static void Operandssereg(t_imdata *im,int index,t_operand *op) {
  int n;
  op->features=OP_SSEREG;
  if (op->arg & B_NOVEXSIZE)
    op->opsize=16;
  else
    op->opsize=im->ssesize;
  op->granularity=4;                   // Default, correct it later!
  op->reg=index;
  op->seg=SEG_UNDEF;
  // Note that some rare SSE commands may use Reg without ModRM.
  if (im->modsize==0)
    im->modsize=1;
  // Decode name of SSE register.
  if (im->damode & DA_TEXT) {
    n=0;
    if (im->config->disasmmode==DAMODE_ATT) op->text[n++]=T('%');
    if (op->opsize==32)
      Tcopycase(op->text+n,TEXTLEN-n,sse256[index],im->config->lowercase);
    else
      Tcopycase(op->text+n,TEXTLEN-n,sse128[index],im->config->lowercase);
    ;
  };
};

// Decodes flag register EFL.
static void Operandefl(t_imdata *im,ulong datasize,t_operand *op) {
  op->features=OP_OTHERREG;
  op->opsize=op->granularity=datasize;
  op->reg=REG_UNDEF;
  op->seg=SEG_UNDEF;
  // Decode name of register.
  if (im->damode & DA_TEXT) {
    if (im->config->disasmmode==DAMODE_ATT)
      Tcopycase(op->text,TEXTLEN,T("%EFL"),im->config->lowercase);
    else
      Tcopycase(op->text,TEXTLEN,T("EFL"),im->config->lowercase);
    ;
  };
};

// Decodes 8/16/32-bit immediate jump/call offset relative to EIP of next
// command.
static void Operandoffset(t_imdata *im,ulong offsetsize,ulong datasize,
  uchar *cmd,ulong cmdsize,ulong offsaddr,t_operand *op) {
  int n;
  tchar label[TEXTLEN];
  if (cmdsize<offsetsize) {
    im->da->errors|=DAE_CROSS;         // Command crosses end of memory block
    return; };
  op->features=OP_CONST;
  op->opsize=op->granularity=datasize; // NOT offsetsize!
  im->immsize1=offsetsize;
  op->reg=REG_UNDEF;
  op->seg=SEG_UNDEF;
  offsaddr+=offsetsize;
  if (offsetsize==1)                   // Sign-extandable constant
    op->opconst=*(signed char *)cmd+offsaddr;
  else if (offsetsize==2)              // 16-bit immediate offset, rare
    op->opconst=*(ushort *)cmd+offsaddr;
  else                                 // 32-bit immediate offset
    op->opconst=*(ulong *)cmd+offsaddr;
  if (datasize==2) {
    op->opconst&=0x0000FFFF;
    im->da->warnings|=DAW_JMP16; };    // Practically unused in Win32 code
  im->usesdatasize=1;
  // Decode address of destination to text, if requested.
  if (im->damode & DA_TEXT) {
    if (offsetsize==1 && im->config->disasmmode!=DAMODE_HLA &&
      im->config->disasmmode!=DAMODE_ATT)
      n=Tcopycase(op->text,TEXTLEN,T("SHORT "),im->config->lowercase);
    else
      n=0;
    if (datasize==4) {
      if (im->decodeaddress!=NULL &&
        im->decodeaddress(label,op->opconst)!=0)
        Tstrcopy(op->text+n,TEXTLEN-n,label);
      else {
        if (im->config->disasmmode==DAMODE_ATT)
          op->text[n++]=T('$');
        Hexprint(4,op->text+n,op->opconst,im,op->arg);
      }; }
    else {
      if (im->config->disasmmode==DAMODE_ATT)
        op->text[n++]=T('$');
      Hexprint(2,op->text+n,op->opconst,im,op->arg);
    };
  };
};

// Decodes 16:16/16:32-bit immediate absolute far jump/call address.
static void Operandimmfaraddr(t_imdata *im,ulong datasize,uchar *cmd,
  ulong cmdsize,t_operand *op) {
  int n;
  if (cmdsize<datasize+2) {
    im->da->errors|=DAE_CROSS;         // Command crosses end of memory block
    return; };
  op->features=OP_CONST|OP_SELECTOR;
  op->opsize=datasize+2;
  op->granularity=datasize;            // Attention, non-standard case!
  op->reg=REG_UNDEF;
  op->seg=SEG_UNDEF;
  im->immsize1=datasize;
  im->immsize2=2;
  if (datasize==2) {
    op->opconst=*(ushort *)cmd;
    im->da->warnings|=DAW_JMP16; }     // Practically unused in Win32 code
  else {
    op->opconst=*(ulong *)cmd;
    im->da->immfixup=im->mainsize; };
  op->selector=*(ushort *)(cmd+datasize);
  im->usesdatasize=1;
  // Decode address of destination to text, if requested.
  if (im->damode & DA_TEXT) {
    if (im->config->disasmmode==DAMODE_ATT) {
      op->text[0]=T('$'); n=1; }
    else
      n=Tcopycase(op->text,TEXTLEN,T("FAR "),im->config->lowercase);
    n+=Hexprint(2,op->text+n,op->selector,im,op->arg);
    op->text[n++]=T(':');
    if (im->config->disasmmode==DAMODE_ATT)
      op->text[n++]=T('$');
    Hexprint(4,op->text+n,op->opconst,im,op->arg);
  };
};

// Decodes immediate constant 1 used in shift operations.
static void Operandone(t_imdata *im,t_operand *op) {
  op->features=OP_CONST;
  op->opsize=op->granularity=1;        // Just to make it defined
  op->reg=REG_UNDEF;
  op->seg=SEG_UNDEF;
  op->opconst=1;
  if (im->damode & DA_TEXT) {
    if (im->config->disasmmode==DAMODE_ATT)
      Tstrcopy(op->text,TEXTLEN,T("$1"));
    else
      Tstrcopy(op->text,TEXTLEN,T("1"));
    ;
  };
};

// Decodes 8/16/32-bit immediate constant (possibly placed after ModRegRM-SIB-
// Disp combination). Constant is nbytes long in the command and extends to
// constsize bytes. If constant is a count, it deals with data of size datasize.
// ATTENTION, calling routine must set usesdatasize by itself!
static void Operandimmconst(t_imdata *im,ulong nbytes,ulong constsize,
  ulong datasize,uchar *cmd,ulong cmdsize,int issecond,t_operand *op) {
  int n;
  ulong u,mod;
  tchar label[TEXTLEN];
  if (cmdsize<im->modsize+im->dispsize+nbytes+(issecond?im->immsize1:0)) {
    im->da->errors|=DAE_CROSS;         // Command crosses end of memory block
    return; };
  op->features=OP_CONST;
  op->opsize=op->granularity=constsize;
  cmd+=im->modsize+im->dispsize;
  if (issecond==0)
    im->immsize1=nbytes;               // First constant
  else {
    im->immsize2=nbytes;               // Second constant (ENTER only)
    cmd+=im->immsize1; };
  op->reg=REG_UNDEF;
  op->seg=SEG_UNDEF;
  if (nbytes==4) {                     // 32-bit immediate constant
    op->opconst=*(ulong *)cmd;
    im->da->immfixup=im->mainsize+im->modsize+im->dispsize+
      (issecond?im->immsize1:0);
    ; }
  else if (nbytes==1)                  // 8-byte constant, maybe sign-extendable
    op->opconst=*(signed char *)cmd;
  else                                 // 16-bite immediate constant, rare
    op->opconst=*(ushort *)cmd;
  if (constsize==1)
    op->opconst&=0x000000FF;
  else if (constsize==2)
    op->opconst&=0x0000FFFF;
  switch (op->arg & B_MODMASK) {
    case B_BITCNT:                     // Constant is a bit count
      if ((datasize==4 && op->opconst>31) ||
        (datasize==1 && op->opconst>7) ||
        (datasize==2 && op->opconst>15)) im->da->warnings|=DAW_SHIFT;
      break;
    case B_SHIFTCNT:                   // Constant is a shift count
      if (op->opconst==0 ||
        (datasize==4 && op->opconst>31) || (datasize==1 && op->opconst>7) ||
        (datasize==2 && op->opconst>15)) im->da->warnings|=DAW_SHIFT;
      break;
    case B_STACKINC:                   // Stack increment must be DWORD-aligned
      if ((op->opconst & 0x3)!=0)
        im->da->warnings|=DAW_STACK;
      im->da->stackinc=op->opconst;
      break;
    default: break; };
  if (im->damode & DA_TEXT) {
    mod=op->arg & B_MODMASK;
    n=0;
    if (constsize==1) {                // 8-bit constant
      if (im->config->disasmmode==DAMODE_ATT)
        op->text[n++]=T('$');
      Hexprint(1,op->text+n,op->opconst,im,op->arg); }
    else if (constsize==4) {           // 32-bit constant
      if (im->decodeaddress!=NULL &&
        (mod==B_NONSPEC || mod==B_JMPCALL || mod==B_JMPCALLFAR) &&
        im->decodeaddress(label,op->opconst)!=0)
        Tstrcopy(op->text+n,TEXTLEN-n,label);
      else {
        if (im->config->disasmmode==DAMODE_ATT)
          op->text[n++]=T('$');
        if (mod!=B_UNSIGNED && mod!=B_BINARY && mod!=B_PORT &&
          (long)op->opconst<0 &&
          (mod==B_SIGNED || (long)op->opconst>NEGLIMIT)
        ) {
          op->text[n++]=T('-'); u=-(long)op->opconst; }
        else
          u=op->opconst;
        Hexprint(4,op->text+n,u,im,op->arg);
      }; }
    else {                             // 16-bit constant
      if (im->config->disasmmode==DAMODE_ATT)
        op->text[n++]=T('$');
      else if ((op->arg & B_SHOWSIZE)!=0) {
        n+=Tcopycase(op->text+n,TEXTLEN-n,sizename[constsize],
          im->config->lowercase);
        n+=Tstrcopy(op->text+n,TEXTLEN-n,T(" ")); };
      Hexprint(2,op->text+n,op->opconst,im,op->arg);
    };
  };
  return;
};

// Decodes contrtol register operands.
static void Operandcreg(t_imdata *im,int index,t_operand *op) {
  int n;
  op->features=OP_CREG;
  op->opsize=op->granularity=4;
  op->reg=index;
  op->seg=SEG_UNDEF;
  // Decode name of control register.
  if (im->damode & DA_TEXT) {
    n=0;
    if (im->config->disasmmode==DAMODE_ATT) op->text[n++]=T('%');
    Tcopycase(op->text+n,TEXTLEN-n,crname[index],im->config->lowercase); };
  // Some control registers are physically absent.
  if (index!=0 && index!=2 && index!=3 && index!=4)
    im->da->errors|=DAE_BADCR;
  ;
};

// Decodes debug register operands.
static void Operanddreg(t_imdata *im,int index,t_operand *op) {
  int n;
  op->features=OP_DREG;
  op->opsize=op->granularity=4;
  op->reg=index;
  op->seg=SEG_UNDEF;
  // Decode name of debug register.
  if (im->damode & DA_TEXT) {
    n=0;
    if (im->config->disasmmode==DAMODE_ATT) op->text[n++]=T('%');
    Tcopycase(op->text+n,TEXTLEN-n,drname[index],im->config->lowercase);
  };
};

// Decodes FPU status register FST.
static void Operandfst(t_imdata *im,t_operand *op) {
  op->features=OP_OTHERREG;
  op->opsize=op->granularity=2;
  op->reg=REG_UNDEF;
  op->seg=SEG_UNDEF;
  // Decode name of register.
  if (im->damode & DA_TEXT) {
    if (im->config->disasmmode==DAMODE_ATT)
      Tcopycase(op->text,TEXTLEN,T("%FST"),im->config->lowercase);
    else
      Tcopycase(op->text,TEXTLEN,T("FST"),im->config->lowercase);
    ;
  };
};

// Decodes FPU control register FCW.
static void Operandfcw(t_imdata *im,t_operand *op) {
  op->features=OP_OTHERREG;
  op->opsize=op->granularity=2;
  op->reg=REG_UNDEF;
  op->seg=SEG_UNDEF;
  // Decode name of register.
  if (im->damode & DA_TEXT) {
    if (im->config->disasmmode==DAMODE_ATT)
      Tcopycase(op->text,TEXTLEN,T("%FCW"),im->config->lowercase);
    else
      Tcopycase(op->text,TEXTLEN,T("FCW"),im->config->lowercase);
    ;
  };
};

// Decodes SSE control register MXCSR.
static void Operandmxcsr(t_imdata *im,t_operand *op) {
  op->features=OP_OTHERREG;
  op->opsize=op->granularity=4;
  op->reg=REG_UNDEF;
  op->seg=SEG_UNDEF;
  // Decode name of register.
  if (im->damode & DA_TEXT) {
    if (im->config->disasmmode==DAMODE_ATT)
      Tcopycase(op->text,TEXTLEN,T("%MXCSR"),im->config->lowercase);
    else
      Tcopycase(op->text,TEXTLEN,T("MXCSR"),im->config->lowercase);
    ;
  };
};


////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// DISASSEMBLER /////////////////////////////////

// Disassembles first command in the binary code of given length at given
// address. Assumes that address and data size attributes of all participating
// segments are 32 bit (flat model). Returns length of the command or 0 in case
// of severe error.
ulong Disasm(uchar const *cmd,ulong cmdsize,ulong ip,t_disasm *da,
  int damode,t_config *config,int (*decodeaddress)(tchar *s,ulong addr)) {
  int i,j,k,q,noperand,nout,enclose,vexreg,success,cfill,ofill;
  ulong m,n,u,prefix,prefixmask,code,arg,cmdtype,datasize;
  ulong type,vex,vexlead;
  t_imdata im;
  const t_chain *pchain;
  const t_bincmd *pcmd;
  const t_modrm *pmrm;
  t_operand *op,pseudoop;
  // Verify input parameters.
  if (cmd==NULL || cmdsize==0 || da==NULL || cmdchain==NULL)
    return 0;                          // Error in parameters or uninitialized
  // Initialize t_disasm structure that receives results of disassembly. This
  // structure is very large, memset() or several memset()'s would take much,
  // much longer.
  da->ip=ip;
  da->memfixup=da->immfixup=-1;
  da->errors=DAE_NOERR;
  da->warnings=DAW_NOWARN;
  da->uses=0;
  da->modifies=0;
  da->memconst=0;
  da->stackinc=0;
  for (i=0,op=da->op; i<NOPERAND; i++,op++) {
    op->features=0;
    op->arg=0;
    op->opsize=op->granularity=0;
    op->reg=REG_UNDEF;
    op->uses=0;
    op->modifies=0;
    op->seg=SEG_UNDEF;
    ((ulong *)op->scale)[0]=0;
    ((ulong *)op->scale)[1]=0;
    op->aregs=0;
    op->opconst=0;
    op->selector=0;
    op->text[0]=T('\0'); };
  da->dump[0]=T('\0');
  da->result[0]=T('\0');
  da->masksize=0;
  // Prepare intermediate data. This data allows to keep Disasm() reentrant
  // (thread-safe).
  im.da=da;
  im.damode=damode;
  if (config==NULL)
    im.config=config=&defconfig;       // Use default configuration
  else
    im.config=config;
  im.decodeaddress=decodeaddress;
  im.prefixlist=0;
  im.ssesize=16;                       // Default
  im.immsize1=im.immsize2=0;
  // Correct 80x86 command may contain up to 4 prefixes belonging to different
  // prefix groups. If Disasm() detects second prefix from the same group, it
  // flushes first prefix in the sequence as a pseudocommand. (This is not
  // quite true; all CPUs that I have tested accept repeating prefixes. Still,
  // who will place superfluous and possibly nonportable prefixes into the
  // code?)
  for (n=0; ; n++) {
    if (n>=cmdsize) {                  // Command crosses end of memory block
      n=0; im.prefixlist=0;            // Decode as standalone prefix
      break; };
    // Note that some CPUs treat REPx and LOCK as belonging to the same group.
    switch (cmd[n]) {
      case 0x26: prefix=PF_ES; prefixmask=PF_SEGMASK; break;
      case 0x2E: prefix=PF_CS; prefixmask=PF_SEGMASK; break;
      case 0x36: prefix=PF_SS; prefixmask=PF_SEGMASK; break;
      case 0x3E: prefix=PF_DS; prefixmask=PF_SEGMASK; break;
      case 0x64: prefix=PF_FS; prefixmask=PF_SEGMASK; break;
      case 0x65: prefix=PF_GS; prefixmask=PF_SEGMASK; break;
      case 0x66: prefix=prefixmask=PF_DSIZE; break;
      case 0x67: prefix=prefixmask=PF_ASIZE; break;
      case 0xF0: prefix=prefixmask=PF_LOCK; break;
      case 0xF2: prefix=PF_REPNE; prefixmask=PF_REPMASK; break;
      case 0xF3: prefix=PF_REP; prefixmask=PF_REPMASK; break;
      default: prefix=0; break; };
    if (prefix==0)
      break;
    if (im.prefixlist & prefixmask) {
      da->errors|=DAE_SAMEPREF;        // Two prefixes from the same group
      break; };
    im.prefixlist|=prefix;
  };
  // There may be VEX prefix preceding command body. Yes, VEX is supported in
  // the 32-bit mode! And even in the 16-bit, but who cares?
  vex=0; vexlead=0;
  if (cmdsize>=n+3 && (*(ushort *)(cmd+n) & 0xC0FE)==0xC0C4) {
    // VEX is not compatible with LOCK, 66, F2 and F3 prefixes. VEX is not
    // compatible with REX, too, but REX prefixes are missing in 32-bit mode.
    if (im.prefixlist & (PF_LOCK|PF_66|PF_F2|PF_F3))
      da->errors|=DAE_SAMEPREF;        // Incompatible prefixes
    else {
      if (cmd[n]==0xC5) {
        // 2-byte VEX prefix.
        im.prefixlist|=PF_VEX2;
        vex=cmd[n+1];
        vexlead=DX_VEX|DX_LEAD0F; n+=2; }
      else {
        // 3-byte VEX prefix.
        im.prefixlist|=PF_VEX3;
        vex=cmd[n+2]+(cmd[n+1]<<8);    // Note the order of the bytes!
        switch (vex & 0x1F00) {
          case 0x0100: vexlead=DX_VEX|DX_LEAD0F; n+=3; break;
          case 0x0200: vexlead=DX_VEX|DX_LEAD38; n+=3; break;
          case 0x0300: vexlead=DX_VEX|DX_LEAD3A; n+=3; break;
          default: vex=0; break;       // Unsupported VEX, decode as LES
        };
      };
      if (vex!=0) {
        // Get size of operands.
        if (vex & 0x0004)
          im.ssesize=32;               // 256-bit SSE operands
        // Get register encoded in VEX prefix.
        vexreg=(~vex>>3) & 0x07;
        // Check for SIMD prefix.
        switch (vex & 0x3) {
          case 0x0001: im.prefixlist|=PF_66; break;
          case 0x0002: im.prefixlist|=PF_F3; break;
          case 0x0003: im.prefixlist|=PF_F2; break;
        };
      };
    };
    if (n>=cmdsize) {                  // Command crosses end of memory block
      n=0; vex=0; im.prefixlist=0;     // Decode as LES
    };
  };
  // We have gathered all prefixes, including those that are integral part of
  // the SSE command.
  if (n>4 || (da->errors & DAE_SAMEPREF)!=0) {
    if (n>4) da->errors|=DAE_MANYPREF;
    n=0; im.prefixlist=0; };           // Decode as standalone prefix
  da->prefixes=im.prefixlist;
  da->nprefix=n;
  // Fetch first 4 bytes of the command and find start of command chain in the
  // command table.
  if (cmdsize>=n+sizeof(ulong))
    code=*(ulong *)(cmd+n);            // Optimization for most frequent case
  else {
    code=cmd[n];
    if (cmdsize>n+1) ((uchar *)&code)[1]=cmd[n+1];
    if (cmdsize>n+2) ((uchar *)&code)[2]=cmd[n+2];
    if (cmdsize>n+3) ((uchar *)&code)[3]=cmd[n+3]; };
  // Walk chain and search for matching command. Command is matched if:
  // (1) code bits allowed in mask coincide in command and descriptor;
  // (2) when command type contains D_MEMORY, ModRegRM byte must indicate
  //     memory, and when type contains D_REGISTER, Mod must indicate register;
  // (3) when bits D_DATAxx or D_ADDRxx are set, size of data and/or code must
  //     match these bits;
  // (4) field D_MUSTMASK must match gathered prefixes;
  // (5) presence or absence of VEX prefix must be matched by DX_VEX. If VEX
  //     is present, implied leading bytes must match vexlead and bit L must
  //     match DX_VLMASK;
  // (6) if short form of string commands is requested, bit D_LONGFORM must be
  //     cleared, or segment override prefix other that DS:, or address size
  //     prefix must be present;
  // (7) when bit D_POSTBYTE is set, byte after ModRegRM/SIB/offset must match
  //     postbyte. Note that all postbyted commands include memory address in
  //     ModRegRM form and do not include immediate constants;
  // (8) if alternative forms of conditional commands are requested, command
  //     is conditional, and it is marked as DX_ZEROMASK or DX_CARRYMASK,
  //     check whether these bits match damode. (Conditional branch on flag
  //     Z!=0 can be disassembled either as JZ or JE. First form is preferrable
  //     after SUB or DEC; second form is more natural after CMP);
  // (9) if command has mnemonics RETN but alternative form RET is expected,
  //     skip it - RET will follow.
  success=0;
  for (pchain=cmdchain+(code & CMDMASK); ; pchain=pchain->pnext) {
    if (pchain==NULL || pchain->pcmd==NULL)
      break;                           // End of chain, no match
    pcmd=pchain->pcmd;
    if (((code ^ pcmd->code) & pcmd->mask)!=0)
      continue;                        // (1) Different code bits
    cmdtype=pcmd->cmdtype;
    if ((damode & DA_TEXT)!=0) {
      if ((pcmd->exttype & DX_RETN)!=0 && config->useretform!=0)
        continue;                      // (9) RET form of near return expected
      if ((cmdtype & D_COND)!=0 &&
        (pcmd->exttype & (DX_ZEROMASK|DX_CARRYMASK))!=0
      ) {
        if ((damode & DA_JZ)!=0 && (pcmd->exttype & DX_ZEROMASK)==DX_JE)
          continue;                    // (8) Wait for DX_JZ
        if ((damode & DA_JC)!=0 && (pcmd->exttype & DX_CARRYMASK)==DX_JB)
          continue;                    // (8) Wait for DX_JC
        ;
      };
    };
    if ((pcmd->exttype & (DX_VEX|DX_LEADMASK))!=vexlead)
      continue;                        // (5) Unmatched VEX prefix
    if (pcmd->exttype & DX_VEX) {
      if (((pcmd->exttype & DX_VLMASK)==DX_LSHORT && (vex & 0x04)!=0) ||
        ((pcmd->exttype & DX_VLMASK)==DX_LLONG && (vex & 0x04)==0))
        continue;                      // (5) Unmatched VEX.L
      ;
    };
    if ((cmdtype &
      (D_MEMORY|D_REGISTER|D_LONGFORM|D_SIZEMASK|D_MUSTMASK|D_POSTBYTE))==0
    ) {
      success=1; break; };             // Optimization for most frequent case
    switch (cmdtype & D_MUSTMASK) {
      case D_MUST66:                   // (4) (SSE) Requires 66, no F2 or F3
        if ((im.prefixlist & (PF_66|PF_F2|PF_F3))!=PF_66) continue;
        break;
      case D_MUSTF2:                   // (4) (SSE) Requires F2, no 66 or F3
        if ((im.prefixlist & (PF_66|PF_F2|PF_F3))!=PF_F2) continue;
        break;
      case D_MUSTF3:                   // (4) (SSE) Requires F3, no 66 or F2
        if ((im.prefixlist & (PF_66|PF_F2|PF_F3))!=PF_F3) continue;
        break;
      case D_MUSTNONE:                 // (4) (MMX,SSE) Requires no 66, F2, F3
        if ((im.prefixlist & (PF_66|PF_F2|PF_F3))!=0) continue;
        break;
      case D_NEEDF2:                   // (4) (SSE) Requires F2, no F3
        if ((im.prefixlist & (PF_F2|PF_F3))!=PF_F2) continue;
        break;
      case D_NEEDF3:                   // (4) (SSE) Requires F3, no F2
        if ((im.prefixlist & (PF_F2|PF_F3))!=PF_F3) continue;
        break;
      case D_NOREP:                    // (4) Must not include F2 or F3
        if ((im.prefixlist & (PF_REP|PF_REPNE))!=0) continue;
        break;
      case D_MUSTREP:                  // (4) Must include F3 (REP)
      case D_MUSTREPE:                 // (4) Must include F3 (REPE)
        if ((im.prefixlist & PF_REP)==0) continue;
        break;
      case D_MUSTREPNE:                // (4) Must include F2 (REPNE)
        if ((im.prefixlist & PF_REPNE)==0) continue;
        break;
      default: break; };
    if ((cmdtype & D_DATA16)!=0 && (im.prefixlist & PF_DSIZE)==0)
      continue;                        // (3) 16-bit data expected
    if ((cmdtype & D_DATA32)!=0 && (im.prefixlist & PF_DSIZE)!=0)
      continue;                        // (3) 32-bit data expected
    if ((cmdtype & D_ADDR16)!=0 && (im.prefixlist & PF_ASIZE)==0)
      continue;                        // (3) 16-bit address expected
    if ((cmdtype & D_ADDR32)!=0 && (im.prefixlist & PF_ASIZE)!=0)
      continue;                        // (3) 32-bit address expected
    if ((cmdtype & D_LONGFORM)!=0 && config->shortstringcmds!=0 &&
      (im.prefixlist & (PF_ES|PF_CS|PF_SS|PF_FS|PF_GS|PF_ASIZE))==0)
      continue;                        // (6) Short form of string cmd expected
    if (cmdtype & D_MEMORY) {
      // (2) Command expects operand in memory (Mod in ModRegRM is not 11b).
      if (n+pcmd->length>=cmdsize)
        break;                         // Command longer than available code
      if ((cmd[n+pcmd->length] & 0xC0)==0xC0) continue; }
    else if (cmdtype & D_REGISTER) {
      // (2) Command expects operand in register (Mod in ModRegRM is 11b).
      if (n+pcmd->length>=cmdsize)
        break;                         // Command longer than available code
      if ((cmd[n+pcmd->length] & 0xC0)!=0xC0) continue; }
    if (cmdtype & D_POSTBYTE) {
      // Command expects postbyte after ModRegRM/SIB/offset as part of the
      // code. If command is longer than available code, immediately report
      // match - error will be reported elsewhere.
      m=n+pcmd->length;                // Offset to ModRegRM byte
      if (m>=cmdsize)
        break;                         // Command longer than available code
      if (im.prefixlist & PF_ASIZE)
        m+=modrm16[cmd[m]].size;       // 16-bit address
      else {
        pmrm=modrm32+cmd[m];
        if (pmrm->psib==NULL)          // 32-bit address without SIB
          m+=pmrm->size;
        else if (m+1>=cmdsize)         // Command longer than available code
          break;
        else                           // 32-bit address with SIB
          m+=pmrm->psib[cmd[m+1]].size;
        ;
      };
      if (m>=cmdsize)
        break;                         // Command longer than available code
      // Asterisk in SSE and AVX commands means comparison predicate. Check
      // for predefined range.
      if (cmd[m]==(uchar)pcmd->postbyte ||
        ((cmdtype & D_WILDCARD)!=0 && cmd[m]<(pcmd->exttype & DX_VEX?32:8)))
        im.immsize1=1;                 // (7) Interprete postbyte as imm const
      else
        continue;                      // (7)
      ;
    };
    success=1;
    break;                             // Perfect match, command found
  };
  // If command is bad but preceded with prefixes, decode first prefix as
  // standalone. In this case, list of command's prefixes is empty.
  if (success==0) {
    pcmd=NULL;
    if (im.prefixlist!=0) {
      n=0; da->nprefix=0; da->prefixes=im.prefixlist=0;
      code=cmd[n] & CMDMASK;
      for (pchain=cmdchain+code; ; pchain=pchain->pnext) {
        if (pchain==NULL || pchain->pcmd==NULL) {
          pcmd=NULL; break; };         // End of chain, no match
        pcmd=pchain->pcmd;
        if ((pcmd->cmdtype & D_CMDTYPE)!=D_PREFIX)
          continue;
        if (((code ^ pcmd->code) & pcmd->mask)==0) {
          cmdtype=pcmd->cmdtype;
          da->errors|=DAE_BADCMD;
          break;
        };
      };
    };
    // If matching command is still not found, report error and return one byte
    // as a command length.
    if (pcmd==NULL) {
      if (damode & DA_DUMP)
        Thexdump(da->dump,cmd,1,config->lowercase);
      if (damode & DA_TEXT) {
        if (config->disasmmode==DAMODE_HLA)
          j=Tcopycase(da->result,TEXTLEN,sizename[1],config->lowercase);
        else if (config->disasmmode==DAMODE_ATT)
          j=Tcopycase(da->result,TEXTLEN,sizeatt[1],config->lowercase);
        else
          j=Tcopycase(da->result,TEXTLEN,sizekey[1],config->lowercase);
        j+=Tstrcopy(da->result+j,TEXTLEN-j,T(" "));
        Thexdump(da->result+j,cmd,1,config->lowercase); };
      da->size=1;
      da->nprefix=0;
      da->prefixes=0;
      da->cmdtype=D_BAD;
      da->exttype=0;
      da->errors|=DAE_BADCMD;          // Unrecognized command
      if (damode & DA_HILITE) {
        da->masksize=tstrlen(da->result);
        memset(da->mask,DRAW_SUSPECT,da->masksize); };
      return 1;
    };
  };
  // Exclude prefixes that are integral part of the command from the list of
  // prefixes. First comparison optimizes for the most frequent case of no
  // obligatory prefixes.
  if (cmdtype & (D_SIZEMASK|D_MUSTMASK)) {
    switch (cmdtype & D_MUSTMASK) {
      case D_MUST66:                   // (SSE) Requires 66, no F2 or F3
      case D_MUSTF2:                   // (SSE) Requires F2, no 66 or F3
      case D_MUSTF3:                   // (SSE) Requires F3, no 66 or F2
        im.prefixlist&=~(PF_66|PF_F2|PF_F3); break;
      case D_NEEDF2:                   // (SSE) Requires F2, no F3
      case D_NEEDF3:                   // (SSE) Requires F3, no F2
        im.prefixlist&=~(PF_F2|PF_F3); break;
      default: break; };
    if (cmdtype & D_DATA16)            // Must include data size prefix
      im.prefixlist&=~PF_DSIZE;
    if (cmdtype & D_ADDR16)            // Must include address size prefix
      im.prefixlist&=~PF_ASIZE;
    ;
  };
  // Prepare for disassembling.
  im.modsize=0;                        // Size of ModRegRM/SIB bytes
  im.dispsize=0;                       // Size of address offset
  im.usesdatasize=0;
  im.usesaddrsize=0;
  im.usessegment=0;
  da->cmdtype=cmdtype;
  da->exttype=pcmd->exttype;
  n+=pcmd->length;                     // Offset of ModRegRM or imm constant
  if (n>cmdsize) {
    da->errors|=DAE_CROSS;             // Command crosses end of memory block
    goto error; };
  im.mainsize=n;                       // Size of command with prefixes
  // Set default data size (note that many commands and operands override it).
  if ((cmdtype & D_SIZE01)!=0 && (cmd[n-1] & 0x01)==0) {
    if (im.prefixlist & PF_DSIZE)
      da->warnings|=DAW_DATASIZE;      // Superfluous data size prefix
    datasize=1; }
  else if (im.prefixlist & PF_DSIZE)
    datasize=2;
  else
    datasize=4;
  // Process operands.
  noperand=0;
  for (i=0; i<NOPERAND; i++) {
    arg=pcmd->arg[i];
    if ((arg & B_ARGMASK)==B_NONE)
      break;                           // Optimization for most frequent case
    // If pseudooperands to be skipped, I process them nevertheless. Such
    // operands may contain important information.
    if ((arg & B_PSEUDO)!=0 && (damode & DA_PSEUDO)==0)
      op=&pseudoop;                    // Request to skip pseudooperands
    else
      op=da->op+noperand++;
    op->arg=arg;
    switch (arg & B_ARGMASK) {
      case B_AL:                       // Register AL
        Operandintreg(&im,1,REG_AL,op);
        break;
      case B_AH:                       // Register AH
        Operandintreg(&im,1,REG_AH,op);
        break;
      case B_AX:                       // Register AX
        Operandintreg(&im,2,REG_EAX,op);
        break;
      case B_CL:                       // Register CL
        Operandintreg(&im,1,REG_CL,op);
        break;
      case B_CX:                       // Register CX
        Operandintreg(&im,2,REG_ECX,op);
        break;
      case B_DX:                       // Register DX
        Operandintreg(&im,2,REG_EDX,op);
        break;
      case B_DXPORT:                   // Register DX as I/O port address
        Operandintreg(&im,2,REG_EDX,op);
        op->features|=OP_PORT;
        break;
      case B_EAX:                      // Register EAX
        Operandintreg(&im,4,REG_EAX,op);
        break;
      case B_EBX:                      // Register EBX
        Operandintreg(&im,4,REG_EBX,op);
        break;
      case B_ECX:                      // Register ECX
        Operandintreg(&im,4,REG_ECX,op);
        break;
      case B_EDX:                      // Register EDX
        Operandintreg(&im,4,REG_EDX,op);
        break;
      case B_ACC:                      // Accumulator (AL/AX/EAX)
        Operandintreg(&im,datasize,REG_EAX,op);
        im.usesdatasize=1;
        break;
      case B_STRCNT:                   // Register CX or ECX as REPxx counter
        Operandintreg(&im,(im.prefixlist & PF_ASIZE?2:4),REG_ECX,op);
        im.usesaddrsize=1;
        break;
      case B_DXEDX:                    // Register DX or EDX in DIV/MUL
        Operandintreg(&im,datasize,REG_EDX,op);
        im.usesdatasize=1;
        break;
      case B_BPEBP:                    // Register BP or EBP in ENTER/LEAVE
        Operandintreg(&im,datasize,REG_EBP,op);
        im.usesdatasize=1;
        break;
      case B_REG:                      // 8/16/32-bit register in Reg
        // Note that all commands that use B_REG have also another operand
        // that requires ModRM, so we don't need to set modsize here.
        if (n>=cmdsize)
          da->errors|=DAE_CROSS;       // Command crosses end of memory block
        else {
          Operandintreg(&im,datasize,(cmd[n]>>3) & 0x07,op);
          im.usesdatasize=1; };
        break;
      case B_REG16:                    // 16-bit register in Reg
        if (n>=cmdsize)
          da->errors|=DAE_CROSS;       // Command crosses end of memory block
        else
          Operandintreg(&im,2,(cmd[n]>>3) & 0x07,op);
        break;
      case B_REG32:                    // 32-bit register in Reg
        if (n>=cmdsize)
          da->errors|=DAE_CROSS;       // Command crosses end of memory block
        else
          Operandintreg(&im,4,(cmd[n]>>3) & 0x07,op);
        break;
      case B_REGCMD:                   // 16/32-bit register in last cmd byte
        Operandintreg(&im,datasize,cmd[n-1] & 0x07,op);
        im.usesdatasize=1;
        break;
      case B_REGCMD8:                  // 8-bit register in last cmd byte
        Operandintreg(&im,1,cmd[n-1] & 0x07,op);
        break;
      case B_ANYREG:                   // Reg field is unused, any allowed
        break;
      case B_INT:                      // 8/16/32-bit register/memory in ModRM
      case B_INT1632:                  // 16/32-bit register/memory in ModRM
        k=Operandmodrm(&im,datasize,cmd+n,cmdsize-n,op);
        if (k<0) break;                // Error in address
        if (k==0) Operandintreg(&im,datasize,cmd[n] & 0x07,op);
        im.usesdatasize=1;
        break;
      case B_INT8:                     // 8-bit register/memory in ModRM
        k=Operandmodrm(&im,1,cmd+n,cmdsize-n,op);
        if (k<0) break;                // Error in address
        if (k==0) Operandintreg(&im,1,cmd[n] & 0x07,op);
        break;
      case B_INT16:                    // 16-bit register/memory in ModRM
        k=Operandmodrm(&im,2,cmd+n,cmdsize-n,op);
        if (k<0) break;                // Error in address
        if (k==0) Operandintreg(&im,2,cmd[n] & 0x07,op);
        break;
      case B_INT32:                    // 32-bit register/memory in ModRM
        k=Operandmodrm(&im,4,cmd+n,cmdsize-n,op);
        if (k<0) break;                // Error in address
        if (k==0) Operandintreg(&im,4,cmd[n] & 0x07,op);
        break;
      case B_INT64:                    // 64-bit integer in ModRM, memory only
        k=Operandmodrm(&im,8,cmd+n,cmdsize-n,op);
        if (k<0) break;                // Error in address
        if (k==0) {
          // Register is not allowed, decode as 32-bit register and set error.
          Operandintreg(&im,4,cmd[n] & 0x07,op);
          op->features|=OP_INVALID;
          da->errors|=DAE_MEMORY; break; };
        break;
      case B_INT128:                   // 128-bit integer in ModRM, memory only
        k=Operandmodrm(&im,16,cmd+n,cmdsize-n,op);
        if (k<0) break;                // Error in address
        if (k==0) {
          // Register is not allowed, decode as 32-bit register and set error.
          Operandintreg(&im,4,cmd[n] & 0x07,op);
          op->features|=OP_INVALID;
          da->errors|=DAE_MEMORY; break; };
        break;
      case B_IMMINT:                   // 8/16/32-bit int at immediate addr
        Operandimmaddr(&im,datasize,cmd+n,cmdsize-n,op);
        im.usesdatasize=1;
        break;
      case B_INTPAIR:                  // Two signed 16/32 in ModRM, memory only
        k=Operandmodrm(&im,2*datasize,cmd+n,cmdsize-n,op);
        if (k<0) break;                // Error in address
        op->granularity=datasize;
        if (k==0) {
          // Register is not allowed, decode as register and set error.
          Operandintreg(&im,datasize,cmd[n] & 0x07,op);
          op->features|=OP_INVALID;
          da->errors|=DAE_MEMORY; break; };
        im.usesdatasize=1;
        break;
      case B_SEGOFFS:                  // 16:16/16:32 absolute address in memory
        k=Operandmodrm(&im,datasize+2,cmd+n,cmdsize-n,op);
        if (k<0) break;                // Error in address
        if (k==0) {
          // Register is not allowed, decode and set error.
          Operandintreg(&im,datasize,cmd[n] & 0x07,op);
          op->features|=OP_INVALID;
          da->errors|=DAE_MEMORY; break; };
        im.usesdatasize=1;
        break;
      case B_STRDEST:                  // 8/16/32-bit string dest, [ES:(E)DI]
        Operandindirect(&im,REG_EDI,1,SEG_ES,0,datasize,op);
        im.usesdatasize=1;
        break;
      case B_STRDEST8:                 // 8-bit string destination, [ES:(E)DI]
        Operandindirect(&im,REG_EDI,1,SEG_ES,0,1,op);
        break;
      case B_STRSRC:                   // 8/16/32-bit string source, [(E)SI]
        Operandindirect(&im,REG_ESI,1,SEG_UNDEF,0,datasize,op);
        im.usesdatasize=1;
        break;
      case B_STRSRC8:                  // 8-bit string source, [(E)SI]
        Operandindirect(&im,REG_ESI,1,SEG_UNDEF,0,1,op);
        break;
      case B_XLATMEM:                  // 8-bit memory in XLAT, [(E)BX+AL]
        Operandxlat(&im,op);
        break;
      case B_EAXMEM:                   // Reference to memory addressed by [EAX]
        Operandindirect(&im,REG_EAX,0,SEG_UNDEF,4,1,op);
        break;
      case B_LONGDATA:                 // Long data in ModRM, mem only
        k=Operandmodrm(&im,256,cmd+n,cmdsize-n,op);
        if (k<0) break;                // Error in address
        op->granularity=1;             // Just a trick
        if (k==0) {
          // Register is not allowed, decode and set error.
          Operandintreg(&im,4,cmd[n] & 0x07,op);
          op->features|=OP_INVALID;
          da->errors|=DAE_MEMORY; break; };
        im.usesdatasize=1;             // Caveat user
        break;
      case B_ANYMEM:                   // Reference to memory, data unimportant
        k=Operandmodrm(&im,1,cmd+n,cmdsize-n,op);
        if (k<0) break;                // Error in address
        if (k==0) {
          // Register is not allowed, decode and set error.
          Operandintreg(&im,4,cmd[n] & 0x07,op);
          op->features|=OP_INVALID;
          da->errors|=DAE_MEMORY; };
        break;
      case B_STKTOP:                   // 16/32-bit int top of stack
        Operandindirect(&im,REG_ESP,1,SEG_SS,0,datasize,op);
        im.usesdatasize=1;
        break;
      case B_STKTOPFAR:                // Top of stack (16:16/16:32 far addr)
        Operandindirect(&im,REG_ESP,1,SEG_SS,0,datasize*2,op);
        op->granularity=datasize;
        im.usesdatasize=1;
        break;
      case B_STKTOPEFL:                // 16/32-bit flags on top of stack
        Operandindirect(&im,REG_ESP,1,SEG_SS,0,datasize,op);
        im.usesdatasize=1;
        break;
      case B_STKTOPA:                  // 16/32-bit top of stack all registers
        Operandindirect(&im,REG_ESP,1,SEG_SS,0,datasize*8,op);
        op->granularity=datasize;
        op->modifies=da->modifies=0xFF;
        im.usesdatasize=1;
        break;
      case B_PUSH:                     // 16/32-bit int push to stack
      case B_PUSHRET:                  // 16/32-bit push of return address
      case B_PUSHRETF:                 // 16:16/16:32-bit push of far retaddr
      case B_PUSHA:                    // 16/32-bit push all registers
        Operandpush(&im,datasize,op);
        im.usesdatasize=1;
        break;
      case B_EBPMEM:                   // 16/32-bit int at [EBP]
        Operandindirect(&im,REG_EBP,1,SEG_SS,0,datasize,op);
        im.usesdatasize=1;
        break;
      case B_SEG:                      // Segment register in Reg
        if (n>=cmdsize)
          da->errors|=DAE_CROSS;       // Command crosses end of memory block
        else
          Operandsegreg(&im,(cmd[n]>>3) & 0x07,op);
        break;
      case B_SEGNOCS:                  // Segment register in Reg, but not CS
        if (n>=cmdsize)
          da->errors|=DAE_CROSS;       // Command crosses end of memory block
        else {
          k=(cmd[n]>>3) & 0x07;
          Operandsegreg(&im,k,op);
          if (k==SEG_SS)
            da->exttype|=DX_WONKYTRAP;
          if (k==SEG_CS) {
            op->features|=OP_INVALID;
            da->errors|=DAE_BADSEG;
          };
        };
        break;
      case B_SEGCS:                    // Segment register CS
        Operandsegreg(&im,SEG_CS,op);
        break;
      case B_SEGDS:                    // Segment register DS
        Operandsegreg(&im,SEG_DS,op);
        break;
      case B_SEGES:                    // Segment register ES
        Operandsegreg(&im,SEG_ES,op);
        break;
      case B_SEGFS:                    // Segment register FS
        Operandsegreg(&im,SEG_FS,op);
        break;
      case B_SEGGS:                    // Segment register GS
        Operandsegreg(&im,SEG_GS,op);
        break;
      case B_SEGSS:                    // Segment register SS
        Operandsegreg(&im,SEG_SS,op);
        break;
      case B_ST:                       // 80-bit FPU register in last cmd byte
        Operandfpureg(&im,cmd[n-1] & 0x07,op);
        break;
      case B_ST0:                      // 80-bit FPU register ST0
        Operandfpureg(&im,0,op);
        break;
      case B_ST1:                      // 80-bit FPU register ST1
        Operandfpureg(&im,1,op);
        break;
      case B_FLOAT32:                  // 32-bit float in ModRM, memory only
        k=Operandmodrm(&im,4,cmd+n,cmdsize-n,op);
        if (k<0) break;                // Error in address
        if (k==0) {
          // Register is not allowed, decode as FPU register and set error.
          Operandfpureg(&im,cmd[n] & 0x07,op);
          op->features|=OP_INVALID;
          da->errors|=DAE_MEMORY; };
        break;
      case B_FLOAT64:                  // 64-bit float in ModRM, memory only
        k=Operandmodrm(&im,8,cmd+n,cmdsize-n,op);
        if (k<0) break;                // Error in address
        if (k==0) {
          // Register is not allowed, decode as FPU register and set error.
          Operandfpureg(&im,cmd[n] & 0x07,op);
          op->features|=OP_INVALID;
          da->errors|=DAE_MEMORY; };
        break;
      case B_FLOAT80:                  // 80-bit float in ModRM, memory only
        k=Operandmodrm(&im,10,cmd+n,cmdsize-n,op);
        if (k<0) break;                // Error in address
        if (k==0) {
          // Register is not allowed, decode as FPU register and set error.
          Operandfpureg(&im,cmd[n] & 0x07,op);
          op->features|=OP_INVALID;
          da->errors|=DAE_MEMORY; };
        break;
      case B_BCD:                      // 80-bit BCD in ModRM, memory only
        k=Operandmodrm(&im,10,cmd+n,cmdsize-n,op);
        if (k<0) break;                // Error in address
        if (k==0) {
          // Register is not allowed, decode as FPU register and set error.
          Operandfpureg(&im,cmd[n] & 0x07,op);
          op->features|=OP_INVALID;
          da->errors|=DAE_MEMORY; };
        break;
      case B_MREG8x8:                  // MMX register as 8 8-bit integers
      case B_MREG16x4:                 // MMX register as 4 16-bit integers
      case B_MREG32x2:                 // MMX register as 2 32-bit integers
      case B_MREG64:                   // MMX register as 1 64-bit integer
        if (n>=cmdsize)
          da->errors|=DAE_CROSS;       // Command crosses end of memory block
        else {
          Operandmmxreg(&im,(cmd[n]>>3) & 0x07,op);
          op->granularity=Getgranularity(arg); };
        break;
      case B_MMX8x8:                   // MMX reg/memory as 8 8-bit integers
      case B_MMX16x4:                  // MMX reg/memory as 4 16-bit integers
      case B_MMX32x2:                  // MMX reg/memory as 2 32-bit integers
      case B_MMX64:                    // MMX reg/memory as 1 64-bit integer
        k=Operandmodrm(&im,8,cmd+n,cmdsize-n,op);
        if (k<0) break;                // Error in address
        if (k==0) Operandmmxreg(&im,cmd[n] & 0x07,op);
        op->granularity=Getgranularity(arg);
        break;
      case B_MMX8x8DI:                 // MMX 8 8-bit integers at [DS:(E)DI]
        Operandindirect(&im,REG_EDI,0,SEG_UNDEF,0,8,op);
        op->granularity=Getgranularity(arg);
        break;
      case B_3DREG:                    // 3DNow! register as 2 32-bit floats
        if (n>=cmdsize)
          da->errors|=DAE_CROSS;       // Command crosses end of memory block
        else {
          Operandnowreg(&im,(cmd[n]>>3) & 0x07,op);
          op->granularity=4; };
        break;
      case B_3DNOW:                    // 3DNow! reg/memory as 2 32-bit floats
        k=Operandmodrm(&im,8,cmd+n,cmdsize-n,op);
        if (k<0) break;                // Error in address
        if (k==0) Operandnowreg(&im,cmd[n] & 0x07,op);
        op->granularity=4;
        break;
      case B_SREGF32x4:                // SSE register as 4 32-bit floats
      case B_SREGF32L:                 // Low 32-bit float in SSE register
      case B_SREGF32x2L:               // Low 2 32-bit floats in SSE register
      case B_SREGF64x2:                // SSE register as 2 64-bit floats
      case B_SREGF64L:                 // Low 64-bit float in SSE register
        if (n>=cmdsize)
          da->errors|=DAE_CROSS;       // Command crosses end of memory block
        else {
          Operandssereg(&im,(cmd[n]>>3) & 0x07,op);
          op->granularity=Getgranularity(arg); };
        break;
      case B_SVEXF32x4:                // SSE reg in VEX as 4 32-bit floats
      case B_SVEXF32L:                 // Low 32-bit float in SSE in VEX
      case B_SVEXF64x2:                // SSE reg in VEX as 2 64-bit floats
      case B_SVEXF64L:                 // Low 64-bit float in SSE in VEX
        Operandssereg(&im,vexreg,op);
        op->granularity=Getgranularity(arg);
        break;
      case B_SSEF32x4:                 // SSE reg/memory as 4 32-bit floats
      case B_SSEF64x2:                 // SSE reg/memory as 2 64-bit floats
        k=Operandmodrm(&im,
          (arg & B_NOVEXSIZE?16:im.ssesize),cmd+n,cmdsize-n,op);
        if (k<0) break;                // Error in address
        if (k==0) Operandssereg(&im,cmd[n] & 0x07,op);
        op->granularity=Getgranularity(arg);
        break;
      case B_SSEF32L:                  // Low 32-bit float in SSE reg/memory
        k=Operandmodrm(&im,4,cmd+n,cmdsize-n,op);
        if (k<0) break;                // Error in address
        if (k==0)                      // Operand in SSE register
          Operandssereg(&im,cmd[n] & 0x07,op);
        op->granularity=4;
        break;
      case B_SSEF32x2L:                // Low 2 32-bit floats in SSE reg/memory
        k=Operandmodrm(&im,
          (arg & B_NOVEXSIZE?16:im.ssesize)/2,cmd+n,cmdsize-n,op);
        if (k<0) break;                // Error in address
        if (k==0)                      // Operand in SSE register
          Operandssereg(&im,cmd[n] & 0x07,op);
        op->granularity=4;
        break;
      case B_SSEF64L:                  // Low 64-bit float in SSE reg/memory
        k=Operandmodrm(&im,8,cmd+n,cmdsize-n,op);
        if (k<0) break;                // Error in address
        if (k==0)                      // Operand in SSE register
          Operandssereg(&im,cmd[n] & 0x07,op);
        op->granularity=8;
        break;
      case B_XMM0I32x4:                // XMM0 as 4 32-bit integers
      case B_XMM0I64x2:                // XMM0 as 2 64-bit integers
      case B_XMM0I8x16:                // XMM0 as 16 8-bit integers
        Operandssereg(&im,0,op);
        op->granularity=Getgranularity(arg);
        break;
      case B_SREGI8x16:                // SSE register as 16 8-bit sigints
      case B_SREGI16x8:                // SSE register as 8 16-bit sigints
      case B_SREGI32x4:                // SSE register as 4 32-bit sigints
      case B_SREGI64x2:                // SSE register as 2 64-bit sigints
      case B_SREGI32L:                 // Low 32-bit sigint in SSE register
      case B_SREGI32x2L:               // Low 2 32-bit sigints in SSE register
      case B_SREGI64L:                 // Low 64-bit sigint in SSE register
        if (n>=cmdsize)
          da->errors|=DAE_CROSS;       // Command crosses end of memory block
        else {
          Operandssereg(&im,(cmd[n]>>3) & 0x07,op);
          op->granularity=Getgranularity(arg); };
        break;
      case B_SVEXI8x16:                // SSE reg in VEX as 16 8-bit sigints
      case B_SVEXI16x8:                // SSE reg in VEX as 8 16-bit sigints
      case B_SVEXI32x4:                // SSE reg in VEX as 4 32-bit sigints
      case B_SVEXI64x2:                // SSE reg in VEX as 2 64-bit sigints
        Operandssereg(&im,vexreg,op);
        op->granularity=Getgranularity(arg);
        break;
      case B_SSEI8x16:                 // SSE reg/memory as 16 8-bit sigints
      case B_SSEI16x8:                 // SSE reg/memory as 8 16-bit sigints
      case B_SSEI32x4:                 // SSE reg/memory as 4 32-bit sigints
      case B_SSEI64x2:                 // SSE reg/memory as 2 64-bit sigints
        k=Operandmodrm(&im,
          (arg & B_NOVEXSIZE?16:im.ssesize),cmd+n,cmdsize-n,op);
        if (k<0) break;                // Error in address
        if (k==0)
          Operandssereg(&im,cmd[n] & 0x07,op);
        op->granularity=Getgranularity(arg);
        break;
      case B_SSEI8x8L:                 // Low 8 8-bit ints in SSE reg/memory
      case B_SSEI16x4L:                // Low 4 16-bit ints in SSE reg/memory
      case B_SSEI32x2L:                // Low 2 32-bit sigints in SSE reg/memory
        k=Operandmodrm(&im,
          (arg & B_NOVEXSIZE?16:im.ssesize)/2,cmd+n,cmdsize-n,op);
        if (k<0) break;                // Error in address
        if (k==0)
          Operandssereg(&im,cmd[n] & 0x07,op);
        op->granularity=Getgranularity(arg);
        break;
      case B_SSEI8x4L:                 // Low 4 8-bit ints in SSE reg/memory
      case B_SSEI16x2L:                // Low 2 16-bit ints in SSE reg/memory
        k=Operandmodrm(&im,4,cmd+n,cmdsize-n,op);
        if (k<0) break;                // Error in address
        if (k==0)
          Operandssereg(&im,cmd[n] & 0x07,op);
        op->granularity=Getgranularity(arg);
        break;
      case B_SSEI8x2L:                 // Low 2 8-bit ints in SSE reg/memory
        k=Operandmodrm(&im,2,cmd+n,cmdsize-n,op);
        if (k<0) break;                // Error in address
        if (k==0)
          Operandssereg(&im,cmd[n] & 0x07,op);
        op->granularity=Getgranularity(arg);
        break;
      case B_SSEI8x16DI:               // SSE 16 8-bit sigints at [DS:(E)DI]
        Operandindirect(&im,REG_EDI,0,SEG_UNDEF,0,
          (arg & B_NOVEXSIZE?16:im.ssesize),op);
        op->granularity=1;
        break;
      case B_EFL:                      // Flags register EFL
        Operandefl(&im,4,op);
        break;
      case B_FLAGS8:                   // Flags (low byte)
        Operandefl(&im,1,op);
        break;
      case B_OFFSET:                   // 16/32 const offset from next command
        Operandoffset(&im,datasize,datasize,cmd+n,cmdsize-n,da->ip+n,op);
        break;
      case B_BYTEOFFS:                 // 8-bit sxt const offset from next cmd
        Operandoffset(&im,1,datasize,cmd+n,cmdsize-n,da->ip+n,op);
        break;
      case B_FARCONST:                 // 16:16/16:32 absolute address constant
        Operandimmfaraddr(&im,datasize,cmd+n,cmdsize-n,op);
        break;
      case B_DESCR:                    // 16:32 descriptor in ModRM
        k=Operandmodrm(&im,6,cmd+n,cmdsize-n,op);
        if (k<0) break;                // Error in address
        if (k==0) {
          // Register is not allowed, decode as 32-bit register and set error.
          Operandintreg(&im,4,cmd[n] & 0x07,op);
          op->features|=OP_INVALID;
          da->errors|=DAE_MEMORY; };
        break;
      case B_1:                        // Immediate constant 1
        Operandone(&im,op);
        break;
      case B_CONST8:                   // Immediate 8-bit constant
        Operandimmconst(&im,1,1,datasize,cmd+n,cmdsize-n,0,op);
        if (arg & B_PORT) op->features|=OP_PORT;
        break;
      case B_SIMMI8x16:                // SSE reg in immediate 8-bit constant
        if (cmdsize-n<im.modsize+im.dispsize+1) {
          da->errors|=DAE_CROSS;       // Command crosses end of memory block
          break; };
        im.immsize1=1;
        Operandssereg(&im,(cmd[n+im.modsize+im.dispsize]>>4) & 0x07,op);
        op->granularity=Getgranularity(arg);
        break;
      case B_CONST8_2:                 // Immediate 8-bit const, second in cmd
        Operandimmconst(&im,1,1,datasize,cmd+n,cmdsize-n,1,op);
        break;
      case B_CONST16:                  // Immediate 16-bit constant
        Operandimmconst(&im,2,2,datasize,cmd+n,cmdsize-n,0,op);
        break;
      case B_CONST:                    // Immediate 8/16/32-bit constant
      case B_CONSTL:                   // Immediate 16/32-bit constant
        Operandimmconst(&im,datasize,datasize,datasize,cmd+n,cmdsize-n,0,op);
        im.usesdatasize=1;
        break;
      case B_SXTCONST:                 // Immediate 8-bit sign-extended to size
        Operandimmconst(&im,1,datasize,datasize,cmd+n,cmdsize-n,0,op);
        im.usesdatasize=1;
        break;
      case B_CR:                       // Control register in Reg
        Operandcreg(&im,(cmd[n]>>3) & 0x07,op);
        break;
      case B_CR0:                      // Control register CR0
        Operandcreg(&im,0,op);
        break;
      case B_DR:                       // Debug register in Reg
        Operanddreg(&im,(cmd[n]>>3) & 0x07,op);
        break;
      case B_FST:                      // FPU status register
        Operandfst(&im,op);
        break;
      case B_FCW:                      // FPU control register
        Operandfcw(&im,op);
        break;
      case B_MXCSR:                    // SSE media control and status register
        Operandmxcsr(&im,op);
        break;
      default:                         // Internal error
        da->errors|=DAE_INTERN;
      break; };
    if ((arg & B_32BITONLY)!=0 && op->opsize!=4)
      da->warnings|=DAW_NONCLASS;
    if ((arg & B_MODMASK)==B_JMPCALLFAR)
      da->warnings|=DAW_FARADDR;
    if (arg & B_PSEUDO) op->features|=OP_PSEUDO;
    if (arg & (B_CHG|B_UPD)) op->features|=OP_MOD;
  };
  if (im.prefixlist!=0) {                 // Optimization for most frequent case
    // If LOCK prefix is present, report error if prefix is not allowed by
    // command and warning otherwise. Application code usually doesn't need
    // atomic bus access.
    if ((im.prefixlist & PF_LOCK)!=0) {
      if ((cmdtype & D_LOCKABLE)==0) da->errors|=DAE_LOCK;
      else da->warnings|=DAW_LOCK; };
    // Warn if data size prefix is present but not used by command.
    if ((im.prefixlist & PF_DSIZE)!=0 && im.usesdatasize==0 &&
      (pcmd->exttype & DX_TYPEMASK)!=DX_NOP)
      da->warnings|=DAW_DATASIZE;
    // Warn if address size prefix is present but not used by command.
    if ((im.prefixlist & PF_ASIZE)!=0 && im.usesaddrsize==0)
      da->warnings|=DAW_ADDRSIZE;
    // Warn if segment override prefix is present but command doesn't access
    // memory. Prefixes CS: and DS: are also used as branch hints in
    // conditional branches.
    if ((im.prefixlist & PF_SEGMASK)!=0 && im.usessegment==0) {
      if ((cmdtype & D_BHINT)==0 || (im.prefixlist & PF_HINT)==0)
      da->warnings|=DAW_SEGPREFIX; };
    // Warn if REPxx prefix is present but not used by command. Attention,
    // Intel frequently uses these prefixes for different means!
    if (im.prefixlist & PF_REPMASK) {
      if (((im.prefixlist & PF_REP)!=0 && (cmdtype & D_MUSTMASK)!=D_MUSTREP &&
      (cmdtype & D_MUSTMASK)!=D_MUSTREPE) ||
      ((im.prefixlist & PF_REPNE)!=0 && (cmdtype & D_MUSTMASK)!=D_MUSTREPNE))
      da->warnings|=DAW_REPPREFIX;
    };
  };
  // Warn on unaligned stack, I/O and privileged commands.
  switch (cmdtype & D_CMDTYPE) {
    case D_PUSH:
      if (datasize==2) da->warnings|=DAW_STACK; break;
    case D_INT:
      da->warnings|=DAW_INTERRUPT; break;
    case D_IO:
      da->warnings|=DAW_IO; break;
    case D_PRIVILEGED:
      da->warnings|=DAW_PRIV;
    break; };
  // Warn on system, privileged  and undocumented commands.
  if ((cmdtype & D_USEMASK)!=0) {
    if ((cmdtype & D_USEMASK)==D_RARE || (cmdtype & D_USEMASK)==D_SUSPICIOUS)
      da->warnings|=DAW_RARE;
    if ((cmdtype & D_USEMASK)==D_UNDOC) da->warnings|=DAW_NONCLASS; };
  // If command implicitly changes ESP, it uses and modifies this register.
  if (cmdtype & D_CHGESP) {
    da->uses|=(1<<REG_ESP);
    da->modifies|=(1<<REG_ESP);
  };
error:
  // Prepare hex dump, if requested. As maximal size of command is limited to
  // MAXCMDSIZE=16 bytes, string can't overflow.
  if (damode & DA_DUMP) {
    if (da->errors & DAE_CROSS)        // Incomplete command
      Thexdump(da->dump,cmd,cmdsize,config->lowercase);
    else {
      j=0;
      // Dump prefixes. REPxx is treated as prefix and separated from command
      // with semicolon; prefixes 66, F2 and F3 that are part of SSE command
      // are glued with command's body - well, at least if there are no other
      // prefixes inbetween.
      for (u=0; u<da->nprefix; u++) {
        j+=Thexdump(da->dump+j,cmd+u,1,config->lowercase);
        if (cmd[u]==0x66 && (cmdtype & D_MUSTMASK)==D_MUST66) continue;
        if (cmd[u]==0xF2 && ((cmdtype & D_MUSTMASK)==D_MUSTF2 ||
          (cmdtype & D_MUSTMASK)==D_NEEDF2)) continue;
        if (cmd[u]==0xF3 && ((cmdtype & D_MUSTMASK)==D_MUSTF3 ||
          (cmdtype & D_MUSTMASK)==D_NEEDF3)) continue;
        if ((im.prefixlist & (PF_VEX2|PF_VEX3))!=0 && u==da->nprefix-2)
          continue;
        if ((im.prefixlist & PF_VEX3)!=0 && u==da->nprefix-3)
          continue;
        da->dump[j++]=T(':'); };
      // Dump body of the command, including ModRegRM and SIB bytes.
      j+=Thexdump(da->dump+j,cmd+u,im.mainsize+im.modsize-u,
        config->lowercase);
      // Dump displacement, if any, separated with space from command's body.
      if (im.dispsize>0) {
        da->dump[j++]=T(' ');
        j+=Thexdump(da->dump+j,cmd+im.mainsize+im.modsize,im.dispsize,
          config->lowercase);
        ;
      };
      // Dump immediate constants, if any.
      if (im.immsize1>0) {
        da->dump[j++]=T(' ');
        j+=Thexdump(da->dump+j,cmd+im.mainsize+im.modsize+im.dispsize,
          im.immsize1,config->lowercase)
        ;
      };
      if (im.immsize2>0) {
        da->dump[j++]=T(' ');
        Thexdump(da->dump+j,cmd+im.mainsize+im.modsize+im.dispsize+im.immsize1,
          im.immsize2,config->lowercase);
        ;
      };
    };
  };
  // Prepare disassembled command. There are many options that control look
  // and feel of disassembly, so the procedure is a bit, errr, boring.
  if (damode & DA_TEXT) {
    if (da->errors & DAE_CROSS) {      // Incomplete command
      q=Tstrcopy(da->result,TEXTLEN,T("???"));
      if (damode & DA_HILITE) {
        memset(da->mask,DRAW_SUSPECT,q);
        da->masksize=q;
      }; }
    else {
      j=0;
      // If LOCK and/or REPxx prefix is present, prepend it to the command.
      // Such cases are rare, first comparison makes small optimization.
      if (im.prefixlist & (PF_LOCK|PF_REPMASK)) {
        if (im.prefixlist & PF_LOCK)
          j=Tcopycase(da->result,TEXTLEN,T("LOCK "),config->lowercase);
        if (im.prefixlist & PF_REPNE)
          j+=Tcopycase(da->result+j,TEXTLEN-j,T("REPNE "),config->lowercase);
        else if (im.prefixlist & PF_REP) {
          if ((cmdtype & D_MUSTMASK)==D_MUSTREPE)
            j+=Tcopycase(da->result+j,TEXTLEN-j,T("REPE "),config->lowercase);
          else
            j+=Tcopycase(da->result+j,TEXTLEN-j,T("REP "),config->lowercase);
          ;
        };
      };
      // If there is a branch hint, prefix jump mnemonics with '+' (taken) or
      // '-' (not taken), or use pseudoprefixes BHT/BHNT. I don't know how MASM
      // indicates hints.
      if (cmdtype & D_BHINT) {
        if (config->jumphintmode==0) {
          if (im.prefixlist & PF_TAKEN)
            da->result[j++]=T('+');
          else if (im.prefixlist & PF_NOTTAKEN)
            da->result[j++]=T('-');
          ; }
        else {
          if (im.prefixlist & PF_TAKEN)
            j+=Tcopycase(da->result+j,TEXTLEN-j,T("BHT "),config->lowercase);
          else if (im.prefixlist & PF_NOTTAKEN)
            j+=Tcopycase(da->result+j,TEXTLEN-j,T("BHNT "),config->lowercase);
          ;
        };
      };
      // Get command mnemonics. If mnemonics contains asterisk, it must be
      // replaced by W, D or none according to sizesens. Asterisk in SSE and
      // AVX commands means comparison predicate.
      if (cmdtype & D_WILDCARD) {
        for (i=0; ; i++) {
          if (pcmd->name[i]==T('\0'))
            break;
          else if (pcmd->name[i]!=T('*'))
            da->result[j++]=pcmd->name[i];
          else if (cmdtype & D_POSTBYTE)
            j+=Tstrcopy(da->result+j,TEXTLEN-j,
            ssepredicate[cmd[im.mainsize+im.modsize+im.dispsize]]);
          else if (datasize==4 &&
            (config->sizesens==0 || config->sizesens==1))
            da->result[j++]=T('D');
          else if (datasize==2 &&
            (config->sizesens==1 || config->sizesens==2))
            da->result[j++]=T('W');
          ; };
        da->result[j]=T('\0');
        if (config->lowercase) tstrlwr(da->result); }
      else {
        j+=Tcopycase(da->result+j,TEXTLEN-j,pcmd->name,config->lowercase);
        if (config->disasmmode==DAMODE_ATT && im.usesdatasize!=0) {
          // AT&T mnemonics are suffixed with the operand's size.
          if ((cmdtype & D_CMDTYPE)!=D_CMD &&
            (cmdtype & D_CMDTYPE)!=D_MOV &&
            (cmdtype & D_CMDTYPE)!=D_MOVC &&
            (cmdtype & D_CMDTYPE)!=D_TEST &&
            (cmdtype & D_CMDTYPE)!=D_STRING &&
            (cmdtype & D_CMDTYPE)!=D_PUSH &&
            (cmdtype & D_CMDTYPE)!=D_POP) ;
          else if (datasize==1) j+=Tcopycase(da->result+j,TEXTLEN-j,
            T("B"),config->lowercase);
          else if (datasize==2) j+=Tcopycase(da->result+j,TEXTLEN-j,
            T("W"),config->lowercase);
          else if (datasize==4) j+=Tcopycase(da->result+j,TEXTLEN-j,
            T("L"),config->lowercase);
          else if (datasize==8) j+=Tcopycase(da->result+j,TEXTLEN-j,
            T("Q"),config->lowercase);
          ;
        };
      };
      if (damode & DA_HILITE) {
        type=cmdtype & D_CMDTYPE;
        if (da->errors!=0)
          cfill=DRAW_SUSPECT;
        else switch (cmdtype & D_CMDTYPE) {
          case D_JMP:                  // Unconditional near jump
          case D_JMPFAR:               // Unconditional far jump
            cfill=DRAW_JUMP; break;
          case D_JMC:                  // Conditional jump on flags
          case D_JMCX:                 // Conditional jump on (E)CX (and flags)
            cfill=DRAW_CJMP; break;
          case D_PUSH:                 // PUSH exactly 1 (d)word of data
          case D_POP:                  // POP exactly 1 (d)word of data
            cfill=DRAW_PUSHPOP; break;
          case D_CALL:                 // Plain near call
          case D_CALLFAR:              // Far call
          case D_INT:                  // Interrupt
            cfill=DRAW_CALL; break;
          case D_RET:                  // Plain near return from call
          case D_RETFAR:               // Far return or IRET
            cfill=DRAW_RET; break;
          case D_FPU:                  // FPU command
          case D_MMX:                  // MMX instruction, incl. SSE extensions
          case D_3DNOW:                // 3DNow! instruction
          case D_SSE:                  // SSE instruction
          case D_AVX:                  // AVX instruction
            cfill=DRAW_FPU; break;
          case D_IO:                   // Accesses I/O ports
          case D_SYS:                  // Legal but useful in system code only
          case D_PRIVILEGED:           // Privileged (non-Ring3) command
            cfill=DRAW_SUSPECT; break;
          default:
            cfill=DRAW_PLAIN;
          break; };
        memset(da->mask,cfill,j);
        da->masksize=j;
      };
      // Add decoded operands. In HLA mode, order of operands is inverted
      // except for comparison commands (marked with bit D_HLADIR) and
      // arguments are enclosed in parenthesis (except for immediate jumps).
      // In AT&T mode, order of operands is always inverted. Operands of type
      // B_PSEUDO are implicit and don't appear in text.
      if (config->disasmmode==DAMODE_HLA &&
        (pcmd->arg[0] & B_ARGMASK)!=B_OFFSET &&
        (pcmd->arg[0] & B_ARGMASK)!=B_BYTEOFFS &&
        (pcmd->arg[0] & B_ARGMASK)!=B_FARCONST)
        enclose=1;                     // Enclose operand list in parenthesis
      else
        enclose=0;
      if ((damode & DA_HILITE)!=0 && config->hiliteoperands!=0)
        cfill=DRAW_PLAIN;
      nout=0;
      for (i=0; i<noperand; i++) {
        if ((config->disasmmode==DAMODE_HLA && (cmdtype & D_HLADIR)==0) ||
          config->disasmmode==DAMODE_ATT)
          k=noperand-1-i;              // Inverted (HLA/AT&T) order of operands
        else
          k=i;                         // Direct (Intel's) order of operands
        arg=da->op[k].arg;
        if ((arg & B_ARGMASK)==B_NONE || (arg & B_PSEUDO)!=0)
          continue;                    // Empty or implicit operand
        q=j;
        if (nout==0) {
          // Spaces between mnemonic and first operand.
          da->result[j++]=T(' ');
          if (config->tabarguments) {
            for ( ; j<8; j++) da->result[j]=T(' '); };
          if (enclose) {
            da->result[j++]=T('(');
            if (config->extraspace) da->result[j++]=(' ');
          }; }
        else {
          // Comma and optional space between operands.
          da->result[j++]=T(',');
          if (config->extraspace) da->result[j++]=T(' ');
        };
        if (damode & DA_HILITE) {
          memset(da->mask+q,cfill,j-q);
          da->masksize=j; };
        // Operand itself.
        q=j;
        op=da->op+k;
        j+=Tstrcopy(da->result+j,TEXTLEN-j-10,op->text);
        if (damode & DA_HILITE) {
          if (config->hiliteoperands==0)
            ofill=cfill;
          else if (op->features & OP_REGISTER)
            ofill=DRAW_IREG;
          else if (op->features & (OP_FPUREG|OP_MMXREG|OP_3DNOWREG|OP_SSEREG))
            ofill=DRAW_FREG;
          else if (op->features & (OP_SEGREG|OP_CREG|OP_DREG))
            ofill=DRAW_SYSREG;
          else if (op->features & OP_MEMORY) {
            if (op->scale[REG_ESP]!=0 || op->scale[REG_EBP]!=0)
              ofill=DRAW_STKMEM;
            else
              ofill=DRAW_MEM;
            ; }
          else if (op->features & OP_CONST)
            ofill=DRAW_CONST;
          else
            ofill=cfill;
          memset(da->mask+q,ofill,j-q);
          da->masksize=j;
        };
        nout++;
      };
      // All arguments added, close list.
      if (enclose && nout!=0) {
        q=j;
        if (config->extraspace) da->result[j++]=T(' ');
        da->result[j++]=T(')');
        if (damode & DA_HILITE) {
          memset(da->mask+q,cfill,j-q);
          da->masksize=j;
        };
      };
      da->result[j]=T('\0');
    };
  };
  // Calculate total size of command.
  if (da->errors & DAE_CROSS)          // Incomplete command
    n=cmdsize;
  else
    n+=im.modsize+im.dispsize+im.immsize1+im.immsize2;
  da->size=n;
  return n;
};

// Given error and warning lists, returns pointer to the string describing
// relatively most severe error or warning, or NULL if there are no errors or
// warnings.
tchar *Geterrwarnmessage(ulong errors,ulong warnings) {
  tchar *ps;
  if (errors==0 && warnings==0)
    ps=NULL;
  else if (errors & DAE_BADCMD)
    ps=T("Unknown command");
  else if (errors & DAE_CROSS)
    ps=T("Command crosses end of memory block");
  else if (errors & DAE_MEMORY)
    ps=T("Illegal use of register");
  else if (errors & DAE_REGISTER)
    ps=T("Memory address is not allowed");
  else if (errors & DAE_LOCK)
    ps=T("LOCK prefix is not allowed");
  else if (errors & DAE_BADSEG)
    ps=T("Invalid segment register");
  else if (errors & DAE_SAMEPREF)
    ps=T("Two prefixes from the same group");
  else if (errors & DAE_MANYPREF)
    ps=T("More than 4 prefixes");
  else if (errors & DAE_BADCR)
    ps=T("Invalid CR register");
  else if (errors & DAE_INTERN)
    ps=T("Internal OllyDbg error");
  else if (warnings & DAW_DATASIZE)
    ps=T("Superfluous operand size prefix");
  else if (warnings & DAW_ADDRSIZE)
    ps=T("Superfluous address size prefix");
  else if (warnings & DAW_SEGPREFIX)
    ps=T("Superfluous segment override prefix");
  else if (warnings & DAW_REPPREFIX)
    ps=T("Superfluous REPxx prefix");
  else if (warnings & DAW_DEFSEG)
    ps=T("Explicit default segment register");
  else if (warnings & DAW_JMP16)
    ps=T("16-bit jump, call or return");
  else if (warnings & DAW_FARADDR)
    ps=T("Far jump or call");
  else if (warnings & DAW_SEGMOD)
    ps=T("Modification of segment register");
  else if (warnings & DAW_PRIV)
    ps=T("Privileged instruction");
  else if (warnings & DAW_IO)
    ps=T("I/O command");
  else if (warnings & DAW_SHIFT)
    ps=T("Shift out of range");
  else if (warnings & DAW_LOCK)
    ps=T("Command uses (valid) LOCK prefix");
  else if (warnings & DAW_STACK)
    ps=T("Unaligned stack operation");
  else if (warnings & DAW_NOESP)
    ps=T("Suspicious use of stack pointer");
  else if (warnings & DAW_NONCLASS)
    ps=T("Undocumented instruction or encoding");
  else
    ps=NULL;
  return ps;
};
#pragma warning (default : 4090 4245 4701 4703 4996)

