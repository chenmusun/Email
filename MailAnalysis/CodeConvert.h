#include <string>
using namespace std;
class CCodeConvert
{
public:
	static enum {
		UTF8_TO_GB2312,
		GB2312_TO_UTF8,
	};
	static void Convert(LPCTSTR srcfile, LPCTSTR destfile, DWORD dwFlag = UTF8_TO_GB2312);
	static void UTF_8ToGB2312(string& pOut, char *pText, int pLen);
	static void GB2312ToUTF_8(string& pOut, char *pText, int pLen);
	// Unicode 转换成UTF-8
	static void UnicodeToUTF_8(char* pOut, WCHAR* pText);
	// GB2312 转换成　Unicode
	static void Gb2312ToUnicode(WCHAR* pOut, char *gbBuffer);
	// 把Unicode 转换成 GB2312
	static void UnicodeToGB2312(char* pOut, WCHAR uData);
	// 把UTF-8转换成Unicode
	// 修改函数的返回类型，表示处理的UTF-8字节数
	// Added by Wangwg 20091103
	static int UTF_8ToUnicode(WCHAR* pOut, char* pText);
	// 把UTF-8转换成Unicode
	static void ISO2002JPToUnicode(WCHAR* pOut, char* pText);

	CCodeConvert();
	virtual ~CCodeConvert();

};

BYTE GetB64Code(char cChar);
std::string base64_encode(const string& src);
std::string base64_decode(const string & src);
std::string quotedprintable_encode(const string&src);
std::string quotedprintable_decode(const string&src);
BOOL base64_decode(const char* pcSrcData, long lDataSize, char* pDestData, long &lDestSize);
BOOL quotedprintable_decode(const char* pcSrcData, long lDataSize, char* pDestData, long &lDestSize);