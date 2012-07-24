//---------------------------------------------------------------------------
#pragma hdrstop

#include "StrUtil.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

//---------------------------------------------------------------------------


void __fastcall WY_BinToHex(char * DesText,const char * SrcBuffer,int SrcBufSize,const char SplitChar)
{
 //asm优化版
 //Bluely 完成于 2005.12.29

  static char  Convert[16] = "0123456789ABCDEF";
__asm
{
                    mov     edi, SrcBufSize
                    mov     esi, DesText
                    mov     bl, SplitChar
                    xor     edx, edx
                    mov     eax, SrcBuffer
                    cmp     edi, edx
                    jle     short FUNC_END

    LOOP_BEGIN:
                    xor     ecx, ecx
                    mov     cl, [eax]
                    sar     ecx, 4
                    mov     cl, Convert[ecx]
                    mov     [esi], cl
                    xor     ecx, ecx
                    mov     cl, [eax]
                    and     ecx, 0Fh
                    test    bl, bl
                    mov     cl, Convert[ecx]
                    mov     [esi+1], cl
                    jnz     short HAS_SPLIT_CHAR
                    add     esi, 2
                    jmp     short ONE_LOOP

    HAS_SPLIT_CHAR:
                    mov     [esi+2], bl
                    add     esi, 3

    ONE_LOOP:
                    inc     edx
                    inc     eax
                    cmp     edi, edx
                    jg      short LOOP_BEGIN

    FUNC_END:
                    mov     byte ptr [esi], 0 //返回缓冲区以0为结尾
}

}

//-----------------------------------------------------------------------------
String BinToStr(char* SrcBuffer, int SrcLen)
{
  AnsiString StrResult;
  StrResult.SetLength(SrcLen*3);
  WY_BinToHex(StrResult.c_str(),SrcBuffer,SrcLen,' ');
  return StrResult;
}

//-----------------------------------------------------------------------------
int GetCharPos(String Data,char SplitChar)
{
  TCHAR ch;
  TCHAR *pch;
  bool IsFind=false;
  int i=0;
  String InStr;
  if (!Data.Length())
    return 0;
  InStr = Data;
  pch = InStr.c_str();
  do
  {
    ch  =*pch;
    if (ch == SplitChar)
    {
      i++;
      IsFind = true;
      break;
    }
    i++;
    if(IsDBCSLeadByte(ch))
    {
      i++;
      ch = pch[1];
      ch  =*pch;
    }
    pch =CharNext(pch);
  }
  while (ch != '\n' && ch != '\0');
  if (!IsFind)
    return 0;
  else
    return i;
}

void SplitStr(AnsiString SrcStr,AnsiString SplitChar,TStrings *Str)
{
  int i,j;
  String tmpStr;
  Str->Clear();
  j=SplitChar.Length();
  i=GetCharPos(SrcStr,*SplitChar.c_str());
  while (i > 0)
  {
    tmpStr = SrcStr.SubString(1,i-1);
    //if (tmpStr != "")
    Str->Add(tmpStr);
    SrcStr.Delete(1,i+j-1);
    i=GetCharPos(SrcStr,*SplitChar.c_str());
  }
  Str->Add(SrcStr);
}