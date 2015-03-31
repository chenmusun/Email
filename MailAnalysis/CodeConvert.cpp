#include "stdafx.h"
#include "CodeConvert.h"
#include "../ReceiveEmail/public.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCodeConvert::CCodeConvert()
{

}

CCodeConvert::~CCodeConvert()
{

}


int  CCodeConvert::UTF_8ToUnicode(WCHAR* pOut, char *pText)
{
	char* uchar = (char *)pOut;

	// 获取UTF-8字符的字节数
	// Added by Wangwg 20100119
	int iByteCnt = 0;
	BYTE byFirstByte = pText[0];
	while ((byFirstByte & 0x80) == 0x80)
	{
		iByteCnt++;
		byFirstByte = (byFirstByte << 1);
	};

	// 增加对有一个或两个字节组成的UTF-8字符到Unicode字符的转换
	// Added by Wangwg 20091103
	if ((pText[0] & 0xF0) == 0xE0)
	{
		uchar[0] = ((pText[1] & 0x03) << 6) + (pText[2] & 0x3F);
		uchar[1] = ((pText[0] & 0x0F) << 4) + ((pText[1] & 0x3F) >> 2);
	}
	else if ((pText[0] & 0xE0) == 0xC0)
	{
		uchar[0] = ((pText[0] & 0x03) << 6) + (pText[1] & 0x3F);
		uchar[1] = ((pText[0] & 0x1F) >> 2);
	}
	else if ((pText[0] & 0xC0) == 0x80)
	{
		uchar[0] = pText[0] & 0x3F;
		uchar[1] = 0x00;
	}
	else
	{
		uchar[0] = uchar[1] = 0x00;
		iByteCnt = 1;
	}

	return iByteCnt;
}

void CCodeConvert::UnicodeToGB2312(char* pOut, WCHAR uData)
{
	WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, &uData, 1, pOut, sizeof(WCHAR), NULL, NULL);
	return;
}

void CCodeConvert::Gb2312ToUnicode(WCHAR* pOut, char *gbBuffer)
{
	::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, gbBuffer, 2, pOut, 1);
	return;
}

void CCodeConvert::UnicodeToUTF_8(char* pOut, WCHAR* pText)
{
	// 注意 WCHAR高低字的顺序,低字节在前，高字节在后
	char* pchar = (char *)pText;

	pOut[0] = (0xE0 | ((pchar[1] & 0xF0) >> 4));
	pOut[1] = (0x80 | ((pchar[1] & 0x0F) << 2)) + ((pchar[1] & 0xC0) >> 6);
	pOut[2] = (0x80 | (pchar[0] & 0x3F));

	return;
}

void CCodeConvert::GB2312ToUTF_8(string& pOut, char *pText, int pLen)
{
	char buf[4];
	char* rst = new char[pLen + (pLen >> 2) + 2];

	memset(buf, 0, 4);
	memset(rst, 0, pLen + (pLen >> 2) + 2);

	int i = 0;
	int j = 0;
	while (i < pLen)
	{
		//如果是英文直接复制就可以
		if (*(pText + i) >= 0)
		{
			rst[j++] = pText[i++];
		}
		else
		{
			WCHAR pbuffer;
			Gb2312ToUnicode(&pbuffer, pText + i);

			UnicodeToUTF_8(buf, &pbuffer);

			unsigned short int tmp = 0;
			tmp = rst[j] = buf[0];
			tmp = rst[j + 1] = buf[1];
			tmp = rst[j + 2] = buf[2];


			j += 3;
			i += 2;
		}
	}
	rst[j] = '\0';

	//返回结果
	pOut = rst;
	MSAFE_DELETE(rst);

	return;
}

void CCodeConvert::UTF_8ToGB2312(string &pOut, char *pText, int pLen)
{
	//TRACE("\r\nCCodeConvert::UTF_8ToGB2312\r\n");
	char * newBuf = new char[pLen + 1];
	memset(newBuf, 0, pLen);
	//newBuf[pLen]=0x00;

	char Ctemp[4];
	memset(Ctemp, 0, 4);

	int i = 0;
	int j = 0;
	int iByteCnt = 0;

	while (i < pLen)
	{
		if (pText[i] > 0)
		{
			if ((pText[i] < 0x20)) 
			{
				/*if (pText[i] == '\r' || pText[i] =='\n')
					newBuf[j++] = pText[i++];
					else
					*/newBuf[j++] = '='; i++;
			}
			else 
				newBuf[j++] = pText[i++];
		}
		else
		{
			WCHAR Wtemp;
			iByteCnt = UTF_8ToUnicode(&Wtemp, pText + i);
			i += iByteCnt;

			if (Wtemp == 0x20AC) // 对欧元符号的特殊处理
			{
				newBuf[j++] = (char)0x80;
			}
			else
			{
				// 避过四字节及以上UTF-8字符，不做处理
				// Added by Wangwg 20100119
				UnicodeToGB2312(Ctemp, Wtemp);
				if (iByteCnt <= 3 && Ctemp[0] < 0)
				{
					newBuf[j++] = Ctemp[0];
					newBuf[j++] = Ctemp[1];
				}
			}
		}
	}
	newBuf[j] = '\0';

	pOut = newBuf;
	MSAFE_DELETE(newBuf);

	return;
}

void CCodeConvert::ISO2002JPToUnicode(WCHAR* pOut, char* pText)
{
	int iCodePage = GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_USE_CP_ACP, NULL, 0);
	int iSrcSize = ::MultiByteToWideChar(iCodePage, MB_PRECOMPOSED, pText, -1, NULL, 0);
	int iDestSize = ::MultiByteToWideChar(iCodePage, MB_PRECOMPOSED, pText, iSrcSize, pOut, 0);

	::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pText, iSrcSize, pOut, iDestSize);
}

void CCodeConvert::Convert(LPCTSTR srcfile, LPCTSTR destfile, DWORD dwFlag)
{
	TRACE("\r\nCCodeConvert::Convert");
	CFile file(srcfile, CFile::modeReadWrite);
	DWORD nlen = (DWORD)file.GetLength();
	char *s = new char[nlen + 1];
	s[nlen] = 0x00;
	file.Read(s, nlen);
	file.Close();

	switch (dwFlag)
	{
	case 0: //UTF-8 => GB2312
	{
				CFile newfile(destfile, CFile::modeCreate | CFile::modeWrite);
				string str;
				UTF_8ToGB2312(str, s, nlen);
				newfile.Write(str.c_str(), str.length());
				newfile.Close();
	}
		break;
	}

	MSAFE_DELETE(s);
}

static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";


static const char base64_table[] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

BYTE GetB64Code(char cChar)
{
	for (BYTE byCode = 0; byCode < sizeof(base64_table); byCode++)
	{
		if (cChar == base64_table[byCode]) return byCode;
	}
	return -1;
}


static inline bool is_base64(unsigned char c) {
	return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_encode(const string& decoded_string)
{
	const char base64_pad = '=';
	char end = '\0';
	string out;
	int inlen = decoded_string.size();
	int inlen1, inlen2 = inlen % 3;
	inlen1 = inlen - inlen2;
	int i;
	if (inlen1 > 0)
	for (i = 0; i < inlen1; i += 3)
	{
		out += base64_table[(decoded_string[i] & 0xFF) >> 2];

		out += base64_table[((decoded_string[i] & 0x3) << 4) | ((decoded_string[i + 1] & 0xF0) >> 4)];
		out += base64_table[((decoded_string[i + 1] & 0xF) << 2) | ((decoded_string[i + 2] & 0xC0) >> 6)];
		out += base64_table[decoded_string[i + 2] & 0x3f];
	}
	if (inlen2 > 0)
	{
		unsigned char fragment;
		out += base64_table[(decoded_string[inlen1] & 0xFF) >> 2];
		fragment = (decoded_string[inlen1] & 0x3) << 4;
		if (inlen2 > 1)
			fragment |= (decoded_string[inlen1 + 1] & 0xF0) >> 4;
		out += base64_table[fragment];
		out += (inlen2 < 2) ? base64_pad : base64_table[(decoded_string[inlen1 + 1] & 0xF) << 2];
		out += base64_pad;
	}
	out += end;
	return out;
}

std::string base64_decode(std::string const& encoded_string)
{
	int in_len = encoded_string.size();
	int i = 0;
	int j = 0;
	int in_ = 0;
	unsigned char char_array_4[4], char_array_3[3];
	std::string ret;

	while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
		char_array_4[i++] = encoded_string[in_]; in_++;
		if (i == 4) {
			for (i = 0; i < 4; i++)
				char_array_4[i] = base64_chars.find(char_array_4[i]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++)
				ret += char_array_3[i];
			i = 0;
		}
	}

	if (i) {
		for (j = i; j < 4; j++)
			char_array_4[j] = 0;

		for (j = 0; j < 4; j++)
			char_array_4[j] = base64_chars.find(char_array_4[j]);

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
	}

	return ret;
}

std::string	quotedprintable_encode(const string& src)
{
	int line_len;
	int max_line_len = 70;
	char tmp[8];
	// 输出的行长度计数         
	line_len = 0;
	char end = '\0';
	string dst;
	for (size_t i = 0; i < src.size(); i++)
	{         // ASCII 33-60, 62-126原样输出，其余的需编码         
		if ((src[i] >= '!') && (src[i] <= '~') && (src[i] != '='))
		{
			dst += src[i];
			line_len++;
		}
		else
		{
			char c1 = 0x0F & (src[i] >> 4);
			char c2 = 0x0F & src[i];

			dst += '=';
			dst += (c1 < 0xA) ? (c1 + 48) : (c1 + 55);
			dst += (c2 < 0xA) ? (c2 + 48) : (c2 + 55);
			line_len += 3;
		}
		if (line_len >= max_line_len)
		{
			sprintf_s(tmp, 8, "=\r\n");
			dst += tmp;
			line_len = 0;
		}
	}       // 输出加个结束符     
	dst += end;
	return dst;
}

std::string quotedprintable_decode(const string& src)
{
	// 输出的字符计数     
	size_t i = 0;
	string dst;
	char tmp = 0, end = '\0';
	while (i < src.size())
	{
		if (src[i] == '=')        // 是编码字节     
		{
			if (i<src.size() - 2)
			{
				char ch1 = src[i + 1];
				char ch2 = src[i + 2];
				if ((src[i + 1] == '/r') || (src[i + 1] == '/n')) continue;
				tmp = ((ch1>'9') ? (ch1 - 'A' + 10) : (ch1 - '0')) * 16 + ((ch2>'9') ? (ch2 - 'A' + 10) : (ch2 - '0'));
				dst += tmp;
				i += 3;
			}
		}
		else        // 非编码字节       
		{
			dst += src[i];
			i++;
		}

	}       // 输出加个结束符     
	dst += end;
	return dst;
}


BOOL base64_decode(const char* pcSrcData, long lDataSize, char* pDestData, long &lDestSize)
{
	if (pcSrcData == NULL || pDestData == NULL || lDestSize <= 0) return FALSE;

	long lSrcSize = 0;
	lSrcSize = (lDataSize <= 0 ? strlen(pcSrcData) : lDataSize);
	if (lDestSize < lSrcSize) return FALSE;

	memset(pDestData, 0, sizeof(pDestData));

	BYTE bySrcSeg[4] = { 0 }, byDestSeg[3] = { 0 };
	long lSrcOff, lDestOff, lIdx;
	lSrcOff = lDestOff = lIdx = 0;

	try
	{
		do
		{
			memset(byDestSeg, 0, sizeof(byDestSeg));
			for (lIdx = 0; lIdx < 4; lIdx++)
			{
				while (pcSrcData[lSrcOff] == 0x09 || pcSrcData[lSrcOff] == 0x0A ||
					pcSrcData[lSrcOff] == 0x0D || pcSrcData[lSrcOff] == 0x20)
				{
					//跳过回车换行空格字符
					lSrcOff++;
					if (lSrcOff >= lSrcSize) break;
				}
				if (lSrcOff >= lSrcSize) break;

				bySrcSeg[lIdx] = GetB64Code(pcSrcData[lSrcOff++]);
				if (lSrcOff >= lSrcSize) break;
			}
			if (bySrcSeg[0] == 64 || bySrcSeg[1] == 64) break;
			byDestSeg[0] = ((bySrcSeg[0] << 2) & 0xFC) + ((bySrcSeg[1] >> 4) & 0x03);
			if (bySrcSeg[2] == 64) //判断是否是'='
			{
				memcpy(pDestData + lDestOff, byDestSeg, 1);
				lDestOff++;
				break;
			}

			byDestSeg[1] = ((bySrcSeg[1] << 4) & 0xF0) + ((bySrcSeg[2] >> 2) & 0x0F);
			if (bySrcSeg[3] == 64) //判断是否是'='
			{
				memcpy(pDestData + lDestOff, byDestSeg, 2);
				lDestOff += 2;
				break;
			}

			byDestSeg[2] = ((bySrcSeg[2] << 6) & 0xC0) + ((bySrcSeg[3]) & 0x03F);
			memcpy(pDestData + lDestOff, byDestSeg, 3);
			lDestOff += 3;
		} while (lSrcOff < lSrcSize);

		// 回车换行
		memcpy(pDestData + lDestOff, "\r\n", 2);

		lDestSize = lDestOff + 2;
		return TRUE;
	}
	catch (...)
	{
	}

	return FALSE;
}


BOOL quotedprintable_decode(const char* pcSrcData, long lDataSize, char* pDestData, long &lDestSize)
{
	if (lDataSize > lDestSize)
		return FALSE;
	long LEN = lDataSize;
	int step = 0;
	try
	{
		char temp[3];
		long pbefore = 0;
		long offset = 0;
		do
		{
			if (pcSrcData[offset] != '=')
			{
				offset++;
				continue;
			}

			memcpy(pDestData + step, pcSrcData + pbefore, offset - pbefore);
			step += offset - pbefore;
			pbefore = offset + 1;

			offset++;
			if ((pcSrcData[offset] == 0x0D) || (pcSrcData[offset] == 0x0A))
			{
				pbefore = offset + 1;
				offset++;

				if ((pcSrcData[offset] == 0x0D) || (pcSrcData[offset] == 0x0A))
				{
					pbefore = offset + 1;
					offset++;
				}
				continue;
			}

			temp[0] = pcSrcData[offset];
			temp[0] = temp[0] < 0x41 ? temp[0] - 0x30 : (temp[0] < 0x61 ? temp[0] - 0x37 : temp[0] - 0x57);

			offset++;
			temp[1] = pcSrcData[offset];
			temp[1] = temp[1] < 0x41 ? temp[1] - 0x30 : (temp[1] < 0x61 ? temp[1] - 0x37 : temp[1] - 0x57);

			temp[2] = (temp[0] << 4) | (temp[1] & 0x0F);

			memcpy(pDestData + step, temp + 2, 1);
			step += 1;

			pbefore = offset + 1;
			offset++;

		} while (offset < LEN);

		if (pbefore < LEN)
		{
			memcpy(pDestData + step, pcSrcData + pbefore, LEN - pbefore);
			step += LEN - pbefore;
		}
	}
	catch (...)
	{
		pDestData[step] = 0x00;
		return FALSE;
	}

	pDestData[step] = 0x00;
	return TRUE;
}
