/** @defgroup CommFunc 常用扩展函数,基于VCL,只能用于BCB(CommFunc.h)
 *  @author  eggxp
 *  @version 1.0
 *  @date    2005.02.20
 *  @{
 */

//---------------------------------------------------------------------------

#ifndef StrUtilH
#define StrUtilH

#include <Classes.hpp>

//---------------------------------------------------------------------------

/** 二进制数据转换成16进制字符串 \n
 *  结果字符串中间分隔符可以设定 ,汇编优化版
 *  @param[in] DesText    目标缓冲区（0结尾字符串)
 *  @param[in] SrcBuffer  源缓冲区(二进制数据)
 *  @param[in] SrcBufSize        源缓冲区长度
 *  @param[in] SplitChar         //自定义分隔字符，0为不使用分隔符
 *  @return 无返回值
 *  @note
 *  @par 示例:
 *  @code
	  char   DesText[100];
	  memset(DesText,0,sizeof(DesText));
	  char   SrcBuffer[3]={0xAA,0xBB,0x10};
	  WY_BinToHex(DesText,SrcBuffer,sizeof(SrcBuffer),',');
	  ShowMessge(DesText);
	  //结果:
	   DesText = "AA,BB,10";
 *  @endcode
 *  @see WY_HexToBin
 *  @deprecated 无
 */
void __fastcall WY_BinToHex(char * DesText,const char * SrcBuffer,int SrcBufSize,const char SplitChar=' ');

/** 二进制数据转换成16进制字符串 \n
 *  包装,调用 WY_BinToHex函数，以便于返回String,易于使用
 *  @param[in] SrcBuffer    源缓冲区(二进制数据)
 *  @param[in] SrcLen        源缓冲区长度
 *  @return 无返回值
 *  @note  默认值是用' '作为分割符
 *  @par 示例:
 *  @code
	  char   SrcBuffer[3]={0xAA,0xBB,0x10};
	  String  DesText = BinToStr(SrcBuffer,sizeof(SrcBuffer));
	  ShowMessge(DesText);
	  //结果:
	   DesText = "AA,BB,10";
 *  @endcode
 *  @see WY_BinToHex
 *  @deprecated 无
 */
String BinToStr(char* SrcBuffer, int SrcLen);

/** 分割字符串 \n
 *  将一个字符串根据某个子串分割成字符串列表
 *  @param[in] SrcStr    源字符串,待分割的字符串
 *  @param[in] SplitChar 分割符,可以是多个字符
 *  @param[in] Str       输出字符串列表,保存分割后的结果
 *  @return 无返回值
 *  @note 分割好的字符串保存在TStrings中
 *  @par 示例:
 *  @code
    TStringList * OutputString = new TStringList;
	String SrcString = "徐,沈,申";
    SplitStr(SrcString,",",OutputString);
	//OutputString结果:
	//OutputString->String[0] == "徐"
	//OutputString->String[1] == "沈"
	//OutputString->String[2] == "申"
 *  @endcode
 *  @see 无
 *  @deprecated 无
 */
void SplitStr(AnsiString SrcStr,AnsiString SplitChar,TStrings *Str);

#endif
