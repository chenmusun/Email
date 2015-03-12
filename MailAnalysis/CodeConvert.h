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
	// Unicode ת����UTF-8
	static void UnicodeToUTF_8(char* pOut, WCHAR* pText);
	// GB2312 ת���ɡ�Unicode
	static void Gb2312ToUnicode(WCHAR* pOut, char *gbBuffer);
	// ��Unicode ת���� GB2312
	static void UnicodeToGB2312(char* pOut, WCHAR uData);
	// ��UTF-8ת����Unicode
	// �޸ĺ����ķ������ͣ���ʾ�����UTF-8�ֽ���
	// Added by Wangwg 20091103
	static int UTF_8ToUnicode(WCHAR* pOut, char* pText);
	// ��UTF-8ת����Unicode
	static void ISO2002JPToUnicode(WCHAR* pOut, char* pText);

	CCodeConvert();
	virtual ~CCodeConvert();

};

std::string base64_encode(const string& src);
std::string base64_decode(const string & src);
std::string quotedprintable_encode(const string&src);
std::string quotedprintable_decode(const string&src);