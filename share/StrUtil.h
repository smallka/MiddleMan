/** @defgroup CommFunc ������չ����,����VCL,ֻ������BCB(CommFunc.h)
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

/** ����������ת����16�����ַ��� \n
 *  ����ַ����м�ָ��������趨 ,����Ż���
 *  @param[in] DesText    Ŀ�껺������0��β�ַ���)
 *  @param[in] SrcBuffer  Դ������(����������)
 *  @param[in] SrcBufSize        Դ����������
 *  @param[in] SplitChar         //�Զ���ָ��ַ���0Ϊ��ʹ�÷ָ���
 *  @return �޷���ֵ
 *  @note
 *  @par ʾ��:
 *  @code
	  char   DesText[100];
	  memset(DesText,0,sizeof(DesText));
	  char   SrcBuffer[3]={0xAA,0xBB,0x10};
	  WY_BinToHex(DesText,SrcBuffer,sizeof(SrcBuffer),',');
	  ShowMessge(DesText);
	  //���:
	   DesText = "AA,BB,10";
 *  @endcode
 *  @see WY_HexToBin
 *  @deprecated ��
 */
void __fastcall WY_BinToHex(char * DesText,const char * SrcBuffer,int SrcBufSize,const char SplitChar=' ');

/** ����������ת����16�����ַ��� \n
 *  ��װ,���� WY_BinToHex�������Ա��ڷ���String,����ʹ��
 *  @param[in] SrcBuffer    Դ������(����������)
 *  @param[in] SrcLen        Դ����������
 *  @return �޷���ֵ
 *  @note  Ĭ��ֵ����' '��Ϊ�ָ��
 *  @par ʾ��:
 *  @code
	  char   SrcBuffer[3]={0xAA,0xBB,0x10};
	  String  DesText = BinToStr(SrcBuffer,sizeof(SrcBuffer));
	  ShowMessge(DesText);
	  //���:
	   DesText = "AA,BB,10";
 *  @endcode
 *  @see WY_BinToHex
 *  @deprecated ��
 */
String BinToStr(char* SrcBuffer, int SrcLen);

/** �ָ��ַ��� \n
 *  ��һ���ַ�������ĳ���Ӵ��ָ���ַ����б�
 *  @param[in] SrcStr    Դ�ַ���,���ָ���ַ���
 *  @param[in] SplitChar �ָ��,�����Ƕ���ַ�
 *  @param[in] Str       ����ַ����б�,����ָ��Ľ��
 *  @return �޷���ֵ
 *  @note �ָ�õ��ַ���������TStrings��
 *  @par ʾ��:
 *  @code
    TStringList * OutputString = new TStringList;
	String SrcString = "��,��,��";
    SplitStr(SrcString,",",OutputString);
	//OutputString���:
	//OutputString->String[0] == "��"
	//OutputString->String[1] == "��"
	//OutputString->String[2] == "��"
 *  @endcode
 *  @see ��
 *  @deprecated ��
 */
void SplitStr(AnsiString SrcStr,AnsiString SplitChar,TStrings *Str);

#endif
