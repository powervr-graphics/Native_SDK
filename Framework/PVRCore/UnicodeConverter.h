/*!*********************************************************************************************************************
\file         PVRCore\UnicodeConverter.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Utility functions for handling Unicode.
***********************************************************************************************************************/
#pragma once
#include "PVRCore/CoreIncludes.h"

namespace pvr {
namespace utils {
/*!*********************************************************************************************************************
\brief  Use as a namespace (static functions only). Contains functionality to work with Unicode strings: Conversions between
ASCII/UTF8/UTF16/UTF32, parse/count characters in Multibyte systems, validity queries.
***********************************************************************************************************************/
class UnicodeConverter
{
public:
	/*!*********************************************************************************************************************
	\brief		Count the number of characters in a unicode UTF-8 string
	\param      unicodeString A UTF-8 string.
	\return     The number of characters unicodeString represents.
	  ***********************************************************************************************************************/
	static uint32 unicodeCount(const utf8* unicodeString);

	/*!*********************************************************************************************************************
	\brief		Count the number of characters in a unicode UTF-16 string
	\param      unicodeString A UTF-16 string.
	\return     The number of characters unicodeString represents.
	  ***********************************************************************************************************************/
	static uint32 unicodeCount(const utf16* unicodeString);

	/*!*********************************************************************************************************************
	\brief		Count the number of characters in a unicode UTF-32 string
	\param      unicodeString A UTF-32 string.
	\return     The number of characters unicodeString represents.
	  ***********************************************************************************************************************/
	static uint32 unicodeCount(const utf32* unicodeString);

	/*!*********************************************************************************************************************
	\param[in]   asciiString An ASCII string
	\param[out]  unicodeString The resulting UTF-8 string stored as an std::vector<utf8>
	\return      pvr::Result Success or error code.
	  ***********************************************************************************************************************/
	static Result convertAsciiToUnicode(const char8* asciiString, std::vector<utf8>& unicodeString);

	/*!*********************************************************************************************************************
	\param[in]   asciiString An ASCII string
	\param[out]  unicodeString The resulting UTF-16 string stored as an std::vector<utf16>
	\return      pvr::Result Success or error code.
	  ***********************************************************************************************************************/
	static Result convertAsciiToUnicode(const char8* asciiString, std::vector<utf16>& unicodeString);

	/*!*********************************************************************************************************************
	\param[in]   asciiString An ASCII string
	\param[out]  utf32StringOut The resulting UTF-32 string stored as an std::vector<utf32>
	\return      pvr::Result Success or error code.
	  ***********************************************************************************************************************/
	static Result convertAsciiToUnicode(const char8* asciiString, std::vector<utf32>& utf32StringOut);

	/*!*********************************************************************************************************************
	\param[in]   utf8String A UTF-8 string
	\param[out]  utf16StringOut The resulting UTF-16 string stored as an std::vector<utf16>
	\return      pvr::Result Success or error code.
	  ***********************************************************************************************************************/
	static Result convertUTF8ToUTF16(const utf8* utf8String, std::vector<utf16>& utf16StringOut);

	/*!*********************************************************************************************************************
	\param[in]   utf8String A UTF-8 string
	\param[out]  utf32StringOut The resulting UTF-32 string stored as an std::vector<utf32>
	\return      pvr::Result Success or error code.
	  ***********************************************************************************************************************/
	static Result convertUTF8ToUTF32(const utf8* utf8String, std::vector<utf32>& utf32StringOut);

	/*!*********************************************************************************************************************
	\param[in]   utf16String A UTF-16 string
	\param[out]  utf8StringOut The resulting UTF-8 string stored as an std::vector<utf8>
	\return      pvr::Result Success or error code.
	  ***********************************************************************************************************************/
	static Result convertUTF16ToUTF8(const utf16* utf16String, std::vector<utf8>& utf8StringOut);

	/*!*********************************************************************************************************************
	\param[in]   utf16String A UTF-16 string
	\param[out]  utf32StringOut The resulting UTF-32 string stored as an std::vector<utf32>
	\return      pvr::Result Success or error code.
	  ***********************************************************************************************************************/
	static Result convertUTF16ToUTF32(const utf16* utf16String, std::vector<utf32>& utf32StringOut);

	/*!*********************************************************************************************************************
	\param[in]   utf32String A UTF-32 string
	\param[out]  utf8StringOut The resulting UTF-8 string stored as an std::vector<utf8>
	\return      pvr::Result Success or error code.
	  ***********************************************************************************************************************/
	static Result convertUTF32ToUTF8(const utf32* utf32String, std::vector<utf8>& utf8StringOut);

	/*!*********************************************************************************************************************
	\param[in]   utf32String A UTF-32 string
	\param[out]  utf16StringOut The resulting UTF-16 string stored as an std::vector<utf16>
	\return      pvr::Result Success or error code.
	  ***********************************************************************************************************************/
	static Result convertUTF32ToUTF16(const utf32* utf32String, std::vector<utf16>& utf16StringOut);

	/*!*********************************************************************************************************************
	\brief  Check if a string contains only valid UTF-8 characters
	  ***********************************************************************************************************************/
	static bool isValidUnicode(const utf8* unicodeString);

	/*!*********************************************************************************************************************
	\brief  Check if a string contains only valid UTF-16 characters
	  ***********************************************************************************************************************/
	static bool isValidUnicode(const utf16* unicodeString);

	/*!*********************************************************************************************************************
	\brief  Check if a string contains only valid UTF-32 characters
	  ***********************************************************************************************************************/
	static bool isValidUnicode(const utf32* unicodeString);

	/*!*********************************************************************************************************************
	\brief  Check if a string only contains valid Ascii-7 characters
	  ***********************************************************************************************************************/
	static bool isAsciiChar(const char8* asciiString);

	/*!*********************************************************************************************************************
	\brief  Check if a character is valid Ascii-7
	  ***********************************************************************************************************************/
	static bool isAsciiChar(char8 asciiChar);

private:
	static bool isValidCodePoint(utf32 codePoint);
};
}
}