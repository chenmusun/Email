#ifndef UTF8_HPP
#define UTF8_HPP

#include <string>

class Utf8 {
public:
	static bool Decode(const std::string &utf8, std::wstring& wcs) {
		unsigned int cursor = 0;
		unsigned int offset;
		unsigned char* p;
		unsigned int e;
		wcs.clear();
		while (cursor < utf8.size()) {
			p = (unsigned char*)&utf8[cursor];
			if (*p < 0xC0) {
				offset = 0;
				e = p[0];
			}
			else if (*p < 0xE0) {	//11000000 < 11100000
				offset = 1;
				if (!Check(cursor, offset, utf8.size())) return false;
				e = (p[0] & 0x1f) << 6;
				e |= (p[1] & 0x3f);
			}
			else if (*p < 0xF0) {	//11100000 < 11110000
				offset = 2;
				if (!Check(cursor, offset, utf8.size())) return false;
				e = (p[0] & 0x0f) << 12;
				e |= (p[1] & 0x3f) << 6;
				e |= (p[2] & 0x3f);
			}
			else if (*p < 0xF8) {	//11110000 < 11111000
				offset = 3;
				if (!Check(cursor, offset, utf8.size())) return false;
				e = (p[0] & 0x07) << 18;
				e |= (p[1] & 0x3f) << 12;
				e |= (p[2] & 0x3f) << 6;
				e |= (p[3] & 0x3f);
			}
			else if (*p < 0xFC) {	//11111000 < 11111100
				offset = 4;
				if (!Check(cursor, offset, utf8.size())) return false;
				e = (p[0] & 0x03) << 24;
				e |= (p[1] & 0x3f) << 18;
				e |= (p[2] & 0x3f) << 12;
				e |= (p[3] & 0x3f) << 6;
				e |= (p[4] & 0x3f);
			}
			else {					//11111100 < 11111110
				offset = 5;
				if (!Check(cursor, offset, utf8.size())) return false;
				e = (p[0] & 0x01) << 30;
				e |= (p[1] & 0x3f) << 24;
				e |= (p[2] & 0x3f) << 18;
				e |= (p[3] & 0x3f) << 12;
				e |= (p[4] & 0x3f) << 6;
				e |= (p[5] & 0x3f);
			}
			wcs.push_back((wchar_t)e);
			cursor += offset + 1;
		}
		return true;
	}

	static void Encode(const std::wstring& wcs, std::string &utf8) {
		unsigned int cursor = 0;
		unsigned int offset;
		unsigned int u;
		utf8.clear();
		while (cursor < wcs.size()) {
			u = wcs[cursor];
			if (u < 0x80) {				//01111111
				offset = 0;
				utf8.push_back(u);
			}
			else if (u < 0x800) {		//11111 111111
				offset = 1;
				utf8.push_back(((u >> 6) & 0x1f) | 0xc0);
				utf8.push_back((u & 0x3f) | 0x80);
			}
			else if (u < 0x10000) {		//1111 111111 111111
				offset = 2;
				utf8.push_back(((u >> 12) & 0x0f) | 0xe0);
				utf8.push_back(((u >> 6) & 0x3f) | 0x80);
				utf8.push_back((u & 0x3f) | 0x80);
			}
			else if (u < 0x200000) {		//111 111111 111111 111111
				offset = 3;
				utf8.push_back(((u >> 18) & 0x07) | 0xf0);
				utf8.push_back(((u >> 12) & 0x3f) | 0x80);
				utf8.push_back(((u >> 6) & 0x3f) | 0x80);
				utf8.push_back((u & 0x3f) | 0x80);
			}
			else if (u < 0x4000000) {	//11 111111 111111 111111 111111
				offset = 4;
				utf8.push_back(((u >> 24) & 0x03) | 0xf8);
				utf8.push_back(((u >> 18) & 0x3f) | 0x80);
				utf8.push_back(((u >> 12) & 0x3f) | 0x80);
				utf8.push_back(((u >> 6) & 0x3f) | 0x80);
				utf8.push_back((u & 0x3f) | 0x80);
			}
			else {						//1 111111 111111 111111 111111 111111
				offset = 5;
				utf8.push_back(((u >> 30) & 0x01) | 0xfc);
				utf8.push_back(((u >> 24) & 0x3f) | 0x80);
				utf8.push_back(((u >> 18) & 0x3f) | 0x80);
				utf8.push_back(((u >> 12) & 0x3f) | 0x80);
				utf8.push_back(((u >> 6) & 0x3f) | 0x80);
				utf8.push_back((u & 0x3f) | 0x80);
			}
			cursor++;
		}
	}

private:
	static bool Check(unsigned int start, unsigned int offset, unsigned int end) {
		if (start + offset >= end) {
			return false;
		}
		return true;
	}
};

#endif
