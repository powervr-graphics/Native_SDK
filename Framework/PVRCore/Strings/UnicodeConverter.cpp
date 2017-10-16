/*!
\brief Implementations for the functions from UnicodeConverter.h
\file PVRCore/Strings/UnicodeConverter.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include <cstring>
#include "PVRCore/Strings/UnicodeConverter.h"
using std::vector;

namespace pvr {
namespace utils {

#define VALID_ASCII 0x80
#define TAIL_MASK 0x3F
#define BYTES_PER_TAIL 6

#define UTF16_SURG_H_MARK 0xD800
#define UTF16_SURG_H_END  0xDBFF
#define UTF16_SURG_L_MARK 0xDC00
#define UTF16_SURG_L_END  0xDFFF

#define UNICODE_NONCHAR_MARK 0xFDD0
#define UNICODE_NONCHAR_END  0xFDEF
#define UNICODE_RESERVED	 0xFFFE
#define UNICODE_MAX			 0x10FFFF

// A table which allows quick lookup to determine the number of bytes of a UTF8 code point.
static const utf8 c_utf8TailLengths[256] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0,
};

// A table which allows quick lookup to determine whether a UTF8 sequence is 'overlong'.
static const utf32 c_utf32MinimumValues[4] =
{
	0x00000000,		// 0 tail bytes
	0x00000080,		// 1 tail bytes
	0x00000800,		// 2 tail bytes
	0x00010000,		// 3 tail bytes
};

uint32 UnicodeConverter::unicodeCount(const utf8* unicodeString)
{
	const utf8* currentCharacter = unicodeString;

	uint32 characterCount = 0;
	while (*currentCharacter)
	{
		// Quick optimisation for ASCII characters
		while (*currentCharacter && ((*currentCharacter) > 0))
		{
			characterCount++;
			currentCharacter++;
		}

		// Done
#if defined(USE_UTF8)
		if (*currentCharacter)
		{
			utf8 tailMask = *currentCharacter & 0xF0;
			switch (tailMask)
			{
			case 0xF0:
				currentCharacter++;
			case 0xE0:
				currentCharacter++;
			case 0xC0:
				currentCharacter++;
				break;
			default:
				//Invalid tail byte
				return 0;
			}

			currentCharacter++;
			characterCount++;
		}
#endif
	}

	return characterCount;
}

uint32 UnicodeConverter::unicodeCount(const utf16* unicodeString)
{
	const utf16* currentCharacter = unicodeString;
	uint32 characterCount = 0;
	while (*currentCharacter)
	{
		if (currentCharacter[0] >= UTF16_SURG_H_MARK && currentCharacter[0] <= UTF16_SURG_H_END &&
		    currentCharacter[1] >= UTF16_SURG_L_MARK && currentCharacter[1] <= UTF16_SURG_L_END)
		{
			currentCharacter += 2;
		}
		else
		{
			currentCharacter += 1;
		}

		characterCount++;
	}

	return characterCount;
}

uint32 UnicodeConverter::unicodeCount(const utf32* unicodeString)
{
	uint32 characterCount = 0;
	const utf32* currentCharacter = unicodeString;
	while (*currentCharacter != 0)
	{
		++characterCount;
		++currentCharacter;
	}

	return characterCount;
}

Result UnicodeConverter::convertAsciiToUnicode(const char8* asciiString, vector<utf8>& unicodeString)
{
	Result result = Result::Success;
	uint32 stringLength = static_cast<uint32>(strlen(asciiString));

	if (isAsciiChar(asciiString) == true)
	{
		//Make sure to include the NULL terminator.
		unicodeString.resize(stringLength + 1);
		if (result == Result::Success)
		{
			for (uint32 i = 0; i < stringLength; ++i)
			{
				unicodeString[i] = asciiString[i];
			}
		}
	}
	else
	{
		result = Result::InvalidArgument;
	}

	return result;
}

Result UnicodeConverter::convertAsciiToUnicode(const char8* asciiString, vector<utf16>& unicodeString)
{
	Result result = Result::Success;

	if (isAsciiChar(asciiString))
	{
		result = convertUTF8ToUTF16((utf8*)asciiString, unicodeString);
	}
	else
	{
		result = Result::InvalidArgument;
	}

	return result;
}

Result UnicodeConverter::convertAsciiToUnicode(const char8* asciiString, vector<utf32>& unicodeString)
{
	Result result = Result::Success;

	if (isAsciiChar(asciiString))
	{
		result = convertUTF8ToUTF32((utf8*)asciiString, unicodeString);
	}
	else
	{
		result = Result::InvalidArgument;
	}

	return result;
}

Result UnicodeConverter::convertUTF8ToUTF16(const utf8* /*unicodeString*/, vector<utf16>& /*unicodeStringOut*/)
{
	assertion(false ,  "UTF8 to UTF16 conversion not implmented");
	return Result::UnknownError;
}

Result UnicodeConverter::convertUTF8ToUTF32(const utf8* unicodeString, vector<utf32>& unicodeStringOut)
{
	Result result = Result::Success;

	uint32 stringLength = static_cast<uint32>(strlen((const char*)unicodeString));
	const utf8* currentCharacter = unicodeString;
	while (*currentCharacter && result == Result::Success)
	{
		// Quick optimisation for ASCII characters
		while (*currentCharacter && isAsciiChar(*currentCharacter))
		{
			unicodeStringOut.push_back(*currentCharacter);
			++currentCharacter;
		}

		//Check that we haven't reached the end.
		if (*currentCharacter != 0)
		{
			//Get the tail length and current character
			utf32 codePoint = *currentCharacter;
			uint32 tailLength = c_utf8TailLengths[codePoint];

			//Increment the character
			++currentCharacter;

			// Check for invalid tail length. Maximum 4 bytes for each UTF8 character.
			// Also check to make sure the tail length is inside the provided buffer.
			if (tailLength != 0 && (currentCharacter + tailLength <= unicodeString + stringLength))
			{
				//Get the data out of the first byte. This depends on the length of the tail.
				codePoint &= (TAIL_MASK >> tailLength);

				//Get the data out of each tail byte
				for (uint32 i = 0; i < tailLength; ++i)
				{
					// Check for invalid tail bytes
					if ((currentCharacter[i] & 0xC0) != 0x80)
					{
						//Invalid tail byte.
						result = Result::InvalidArgument;
					}

					codePoint = (codePoint << BYTES_PER_TAIL) + (currentCharacter[i] & TAIL_MASK);
				}

				if (result == Result::Success)
				{
					currentCharacter += tailLength;

					// Check overlong values.
					if (codePoint >= c_utf32MinimumValues[tailLength])
					{
						if (isValidCodePoint(codePoint))
						{
							unicodeStringOut.push_back(codePoint);
						}
						else
						{
							result = Result::InvalidArgument;
						}
					}
					else
					{
						//Overlong!
						result = Result::InvalidArgument;
					}
				}
			}
			else
			{
				result = Result::OutOfBounds;
			}


		}
	}

	return result;
}

Result UnicodeConverter::convertUTF16ToUTF8(const utf16* /*unicodeString*/, vector<utf8>& /*unicodeStringOut*/)
{
	assertion(false ,  "UTF16 to UTF8 conversion not implmented");
	return Result::UnknownError;
}

Result UnicodeConverter::convertUTF16ToUTF32(const utf16* unicodeString, vector<utf32>& unicodeStringOut)
{
	Result result = Result::Success;

	const uint16* currentCharacter = unicodeString;

	// Determine the number of shorts
	while (*++currentCharacter);
	unsigned int uiBufferLen = (unsigned int)(currentCharacter - unicodeString);

	// Reset to start.
	currentCharacter = unicodeString;

	while (*currentCharacter && result == Result::Success)
	{
		// Straight copy. We'll check for surrogate pairs next...
		uint32 codePoint = *currentCharacter;
		++currentCharacter;

		// Check for a surrogate pair indicator.
		if (codePoint >= UTF16_SURG_H_MARK && codePoint <= UTF16_SURG_H_END)
		{
			// Make sure the next 2 bytes are in range...
			if (currentCharacter + 1 < unicodeString + uiBufferLen && *currentCharacter != 0)
			{
				//Check that the next value is in the low surrogate range.
				if (*currentCharacter >= UTF16_SURG_L_MARK && *currentCharacter <= UTF16_SURG_L_END)
				{
					codePoint = ((codePoint - UTF16_SURG_H_MARK) << 10) + (*currentCharacter - UTF16_SURG_L_MARK) + 0x10000;
					++currentCharacter;
				}
				else
				{
					result = Result::InvalidArgument;
				}
			}
			else
			{
				result = Result::OutOfBounds;
			}
		}

		if (result == Result::Success)
		{
			//Check that the code point is valid
			if (isValidCodePoint(codePoint))
			{
				unicodeStringOut.push_back(codePoint);
			}
			else
			{
				result = Result::InvalidArgument;
			}
		}
	}

	return result;
}

Result UnicodeConverter::convertUTF32ToUTF8(const utf32* /*unicodeString*/, vector<utf8>& /*unicodeStringOut*/)
{
	assertion(false ,  "UTF32 to UTF8 conversion not implmented");
	return Result::UnknownError;
}

Result UnicodeConverter::convertUTF32ToUTF16(const utf32* /*unicodeString*/, vector<utf16>& /*unicodeStringOut*/)
{
	assertion(false ,  "UTF32 to UTF16 conversion not implmented");
	return Result::UnknownError;
}

bool UnicodeConverter::isAsciiChar(char8 asciiChar)
{
	//Make sure that the ascii string is limited to encodings with the first 7 bits. Any outside of this range are part of the system's locale.
	if ((asciiChar & VALID_ASCII) != 0)
	{
		return false;
	}

	return true;
}

bool UnicodeConverter::isAsciiChar(const char8* asciiString)
{
	uint32 stringLength = static_cast<uint32>(strlen(asciiString));

	for (uint32 i = 0; i < stringLength; ++i)
	{
		if (!isAsciiChar(asciiString[i]))
		{
			return false;
		}
	}

	return true;
}

bool UnicodeConverter::isValidUnicode(const utf8* unicodeString)
{
	uint32 stringLength = unicodeCount(unicodeString);
	const utf8* currentCharacter = unicodeString;
	while (*currentCharacter != 0)
	{
		//Quick optimisation for ASCII characters - these are always valid.
		while (*currentCharacter && isAsciiChar((char8)*currentCharacter))
		{
			currentCharacter++;
		}

		//Check that we haven't hit the end with the previous loop.
		if (*currentCharacter != 0)
		{
			//Get the code point
			utf32 codePoint = *currentCharacter;

			//Move to the next character
			++currentCharacter;

			//Get the tail length for this codepoint
			uint32 tailLength = c_utf8TailLengths[codePoint];

			//Check for invalid tail length, characters - there are only a few possibilities here due to the lookup table.
			//Also check to make sure the tail length is inside the provided buffer.
			if (tailLength == 0 || (currentCharacter + tailLength > unicodeString + stringLength))
			{
				return false;
			}

			// Get the data out of the first byte. This depends on the length of the tail.
			codePoint &= (TAIL_MASK >> tailLength);

			// Get the data out of each tail byte, making sure that the tail is valid unicode.
			for (uint32 i = 0; i < tailLength; ++i)
			{
				// Check for invalid tail bytes
				if ((currentCharacter[i] & 0xC0) != 0x80)
				{
					return false;
				}

				codePoint = (codePoint << BYTES_PER_TAIL) + (currentCharacter[i] & TAIL_MASK);
			}
			currentCharacter += tailLength;

			//Check for 'overlong' values - i.e. values which have a tail but don't actually need it..
			if (codePoint < c_utf32MinimumValues[tailLength])
			{
				return false;
			}

			//Check that it's a valid code point
			if (!isValidCodePoint(codePoint))
			{
				return false;
			}
		}
	}

	return true;
}

bool UnicodeConverter::isValidUnicode(const utf16* unicodeString)
{
	const uint16* currentCharacter = unicodeString;

	// Determine the number of shorts
	while (*++currentCharacter);
	unsigned int uiBufferLen = (unsigned int)(currentCharacter - unicodeString);

	// Reset to start.
	currentCharacter = unicodeString;

	while (*currentCharacter)
	{
		// Straight copy. We'll check for surrogate pairs next...
		uint32 codePoint = *currentCharacter;
		++currentCharacter;

		// Check for a surrogate pair indicator.
		if (codePoint >= UTF16_SURG_H_MARK && codePoint <= UTF16_SURG_H_END)
		{
			// Make sure the next 2 bytes are in range...
			if (currentCharacter + 1 < unicodeString + uiBufferLen && *currentCharacter != 0)
			{
				//Check that the next value is in the low surrogate range.
				if (*currentCharacter >= UTF16_SURG_L_MARK && *currentCharacter <= UTF16_SURG_L_END)
				{
					codePoint = ((codePoint - UTF16_SURG_H_MARK) << 10) + (*currentCharacter - UTF16_SURG_L_MARK) + 0x10000;
					++currentCharacter;
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}

		//Check that the code point is valid
		if (!isValidCodePoint(codePoint))
		{
			return false;
		}
	}

	return true;
}

bool UnicodeConverter::isValidUnicode(const utf32* unicodeString)
{
	uint32 stringLength = unicodeCount(unicodeString);

	for (uint32 i = 0; i < stringLength; ++i)
	{
		if (!isValidCodePoint(unicodeString[i]))
		{
			return false;
		}
	}

	return true;
}

 bool UnicodeConverter::isValidCodePoint(utf32 codePoint)
{
	// Check that this value isn't a UTF16 surrogate mask.
	if (codePoint >= UTF16_SURG_H_MARK && codePoint <= UTF16_SURG_L_END)
	{
		return false;
	}

	// Check non-char values
	if (codePoint >= UNICODE_NONCHAR_MARK && codePoint <= UNICODE_NONCHAR_END)
	{
		return false;
	}

	// Check reserved values
	if ((codePoint & UNICODE_RESERVED) == UNICODE_RESERVED)
	{
		return false;
	}

	// Check max value.
	if (codePoint > UNICODE_MAX)
	{
		return false;
	}

	return true;
}
}
}
//!\endcond