#include <stdio.h>
#include <conio.h>
#include <windows.h>
#pragma hdrstop

#include "disasm.h"

int Decodeaddress(tchar *s,ulong addr) {
  if (addr<1024 || addr>50000) return 0;
  return sprintf(s,"LABEL_%x",addr); 
};

int main(int argc,tchar* argv[]) {
  int m,n,len,cmdlen,pos,ncmd;
  uchar *cmd;
  tchar *pmsg,s[512];
  ulong t;
  t_disasm da;
  FILE *f,*g;

  Preparedisasm();

  f=fopen("disasm.exe","rb");

  fseek(f,0,SEEK_END);
  len=ftell(f);
  fseek(f,0,SEEK_SET);
  cmd=(uchar *)malloc(len);
  fread(cmd,len,1,f);
  fclose(f);

  // First loop: determine length of the command, no text.
  printf("Determine command length and get information:\n");
  pos=0;
  ncmd=0;
  t=GetTickCount();
  while (pos<len) {
    cmdlen=Disasm(cmd+pos,len-pos,pos,&da,0,NULL,NULL);
    if (cmdlen==0) break;
    pos+=cmdlen; ncmd++;
  };
  t=GetTickCount()-t;
  printf("  %i commands in %i milliseconds, %i ns per commnand\n\n",
    ncmd,t,(__int64)(t*1000000)/max(1,ncmd));

  // Second loop: disassembling to text.
  printf("Pure disassembling to text:\n");
  pos=0;
  ncmd=0;
  t=GetTickCount();
  while (pos<len) {
    cmdlen=Disasm(cmd+pos,len-pos,pos,&da,DA_TEXT,NULL,NULL);
    if (cmdlen==0) break;
    pos+=cmdlen; ncmd++;
  };
  t=GetTickCount()-t;
  printf("  %i commands in %i milliseconds, %i ns per commnand\n\n",
    ncmd,t,(__int64)(t*1000000)/max(1,ncmd));

  // Third loop: disassembling to text, creation of dump and highlighting
  // of operands.
  printf("Disassembly, dump and highlighting:\n");
  pos=0;
  ncmd=0;
  t=GetTickCount();
  while (pos<len) {
    cmdlen=Disasm(cmd+pos,len-pos,pos,&da,DA_TEXT|DA_DUMP|DA_HILITE,NULL,NULL);
    if (cmdlen==0) break;
    pos+=cmdlen; ncmd++;
  };
  t=GetTickCount()-t;
  printf("  %i commands in %i milliseconds, %i ns per commnand\n\n",
    ncmd,t,(__int64)(t*1000000)/max(1,ncmd));

  // Fourth loop: creation of file with addresses, dump, disassembly and
  // comments.

  printf("Creation of file:\n");
  pos=0;
  ncmd=0;
  g=fopen(T("a.asm"),T("wt"));
  while (pos<len) {
    cmdlen=Disasm(cmd+pos,len-pos,0x00000+pos,&da,DA_TEXT|DA_DUMP,NULL,Decodeaddress);
    if (cmdlen==0) break;
    n=tsprintf(s,T("%08X  %s "),da.ip,da.dump);
    while (n<30) s[n++]=T(' ');
    n+=tsprintf(s+n,T("%s"),da.result);
    pmsg=Geterrwarnmessage(da.errors,da.warnings);
    if (pmsg!=NULL) {
      while (n<56) s[n++]=T(' ');
      tsprintf(s+n,T("; %s"),pmsg); };
    fprintf(g,T("%s\n"),s);
    if (da.masksize>0) {
      for (n=0; n<30; n++) s[n]=T(' ');
      for (m=0; m<da.masksize; m++) s[n++]=da.mask[m];
      s[n]=T('\0');
      fprintf(g,T("%s\n"),s);
    };
    pos+=cmdlen; ncmd++;
  };
  fclose(g);
  printf("  File a.asm created.\n\n");

  free(cmd);
  Finishdisasm();

  printf("Press any key to finish...\n");
  getch();
  return 0;
};

