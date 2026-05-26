#pragma once

// return a Unicode character from two UTF-16 characters
	// (if the Unicode character is >0xffff two UTF-16 chars are needed to represent it)
	inline static unsigned long FromUTF16(const wchar_t c0, const wchar_t c1)
	{
		if (c0 >= 0xd800 && c0<0xdc00)
		{
			//surrogate pair indicated
			//convert back to long char
			unsigned long c4 = c0;
			unsigned long c5 = c1;

			c4 -= 0xd800;		//upper 10 bits
			c5 -= 0xdc00;		//lower 10 bits
			c4 &= 0x3ff;
			c5 &= 0x3ff;
			c4 <<= 10;
			c5 |= c4;
			c5 += 0x10000;	//c5 is the original unicode char
			return(c5);
		}
		else
		{
			//src is 16 bits anyway - return
			return(c0);
		}

	}

static unsigned long ToUTF8(unsigned char* buffer, unsigned long buffer_size, const wchar_t* src, const unsigned int num_chars)
	{
		unsigned long i = 0;
		unsigned long destpos = 0;
		wchar_t c0;
		wchar_t c1;
		unsigned long c;
		while (i<num_chars)
		{
			c0 = src[i];					//read wide char

			//convert FROM UTF-16
			if (i < num_chars - 1)
			{
				//read the possible second surrogate pair character (if char is > 0xffff)

				c1 = src[i + 1];
				//and convert surrogate pair (if applicable) to Unicode character
				c = FromUTF16(c0, c1);
			}
			else
			{
				//the last character, so cannot be a surrogate pair
				c = c0;
			}




			if (c <= 0x7f)
			{
				//char 0-7f single byte
				if (destpos<buffer_size)
				{
					buffer[destpos] = (unsigned char)c;
				}
				destpos++;
			}
			else if (c <= 0x7ff)
			{
				//U+0080 to U+07FF
				//00000yyy yyxxxxxx encodes to 110yyyyy 10xxxxxx
				unsigned char sx = (unsigned char)(c & 0x3f) | 0x80;	//store bits 0-5
				c >>= 6;
				c |= 0xC0;

				if (destpos<buffer_size - 1)
				{
					buffer[destpos + 1] = sx;
					buffer[destpos + 0] = (unsigned char)c;
				}
				destpos += 2;

			}
			else if (c <= 0xffff)
			{
				// zzzzyyyy yyxxxxxx encodes to 1110zzzz 10yyyyyy 10xxxxxx
				unsigned char sx = (unsigned char)(c & 0x3f) | 0x80;	//store bits 0-5
				c >>= 6;
				unsigned char sx1 = (unsigned char)(c & 0x3f) | 0x80;	//store bits 6-11
				c >>= 6;
				unsigned char sx2 = (unsigned char)(c & 0xf) | 0xE0;	//finally bits 12-15

				if (destpos<buffer_size - 2)
				{
					buffer[destpos + 2] = sx;
					buffer[destpos + 1] = sx1;
					buffer[destpos + 0] = sx2;
				}
				destpos += 3;

			}
			else if (c>0xffff)
			{
				//supplementary src pair of 16 bit chars making unicode>0xffff
				//now convert to utf-8

				// 0wwwzz zzzzyyyy yyxxxxxx
				//
				// 11110www
				// 10zzzzzz
				// 10yyyyyy
				// 10xxxxxx
				unsigned long v8_3 = c & 0x3f;
				v8_3 |= 0x80;

				unsigned long v8_2 = c >> 6;
				v8_2 &= 0x3f;
				v8_2 |= 0x80;

				unsigned long v8_1 = c >> 12;
				v8_1 &= 0x3f;
				v8_1 |= 0x80;

				unsigned long v8_0 = c >> 18;
				v8_0 &= 0x7;
				v8_0 |= 0xF0;

				//store 4 bytes
				if (destpos<buffer_size - 3)
				{
					buffer[destpos + 0] = (unsigned char)v8_0;
					buffer[destpos + 1] = (unsigned char)v8_1;
					buffer[destpos + 2] = (unsigned char)v8_2;
					buffer[destpos + 3] = (unsigned char)v8_3;
				}
				//skip a source char
				i++;
				//always stored as 4 chars
				destpos += 4;

			}
			i++;
		}

		return destpos;
	}


