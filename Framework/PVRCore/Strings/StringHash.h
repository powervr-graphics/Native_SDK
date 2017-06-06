/*!
\brief A hashed string with functionality for fast compares.
\file PVRCore/Strings/StringHash.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/CoreIncludes.h"
#include "PVRCore/Base/Hash_.h"
#include "PVRCore/StringFunctions.h"
#include <functional>

namespace pvr {
/// <summary>Implementation of a hashed string with functionality for fast compares.</summary>
/// <remarks>In most cases, can be used as a drop-in replacement for std::strings to take advantage of fast
/// hashed comparisons. On debug builds, tests for hash collisions are performed (Assertion + error log if
/// collision found)</remarks>
class StringHash
{
	typedef hash<std::string> HashFn;
public:
	/// <summary>Constructor. Initialize with c-style string, which will be copied. Automatically calculates hash.</summary>
	/// <param name="str">A c-style string. Will be copied internaly.</param>
	StringHash(const char* str) : _String(str), _Hash(HashFn()(_String)) { }

	/// <summary>Constructor. Initialize with c++style string, which will be copied. Automatically calculates hash.</summary>
	/// <param name="right">The string to initialize with.</param>
	StringHash(const std::string& right) : _String(right), _Hash(HashFn()(_String)) { }

	/// <summary>Conversion to string reference. No-op.</summary>
	/// <returns>A string representatation of this hash</returns>
	operator const std::string& () const { return _String; }

	/// <summary>ctor. Default constructor. Empty string.</summary>
	StringHash() : _String(""), _Hash(HashFn()(_String)) {}

	/// <summary>Appends a string to the end of this StringHash, recalculates hash.</summary>
	/// <param name="ptr">A string</param>
	/// <returns>this (post the operation)</returns>
	StringHash& append(const char* ptr)
	{
		_String.append(ptr);
		_Hash = HashFn()(_String);
		return *this;
	}

	/// <summary>Appends a string.</summary>
	/// <param name="str">A string</param>
	/// <returns>this (post the operation)</returns>
	StringHash& append(const string& str)
	{
		_String.append(str);
		_Hash = HashFn()(_String);
		return *this;
	}

	/// <summary>Assigns the string to the string ptr.</summary>
	/// <param name="ptr">A string</param>
	/// <returns>this (post the operation)</returns>
	StringHash& assign(const char* ptr)
	{
		_String.assign(ptr);
		_Hash = HashFn()(_String);
		return *this;
	}

	/// <summary>Assigns the string to the string str.</summary>
	/// <param name="str">A string</param>
	/// <returns>this (post the operation)</returns>
	StringHash& assign(const string& str)
	{
		_String = str;
		_Hash = HashFn()(_String);
		return *this;
	}

	/// <summary>Return the length of this string hash</summary>
	/// <returns>Length of this string hash</returns>
	size_t size() const { return _String.size(); }

	/// <summary>Return the length of this string hash</summary>
	/// <returns>Length of this string hash</returns>
	size_t length() const { return _String.length(); }

	/// <summary>Return if the string is empty</summary>
	/// <returns>true if the string is emtpy (length=0), false otherwise</returns>
	bool empty() const { return _String.empty(); }

    /// <summary>Clear this string hash</summary>
	void clear() { assign(string());  }

	/// <summary>== Operator. Compares hash values. Extremely fast.</summary>
	/// <param name="str">A hashed string to compare with</param>
	/// <returns>True if the strings have the same hash</returns>
	/// <remarks>On debug builds, this operation does a deep check for hash collisions. You may also define
	/// PVR_STRING_HASH_STRONG_COMPARISONS if you want to force deep checks (in which) case IFF hashes are equal the
	/// the strings will be compared char-per-char as well. Only consider this if you think that for some reason you
	/// have an extremely high probability of hash collisions.</remarks>
	bool operator==(const StringHash& str) const
	{
#ifdef DEBUG //Collision detection
		if (_Hash == str.getHash() && _String != str._String)
		{
			Log(Log.Critical, "***** STRING HASH COLLISION DETECTED ********************");
			Log(Log.Critical, "** String [%s] collides with string [%s] ", _String.c_str(), str._String.c_str());
			Log(Log.Critical, "*********************************************************");
			assertion(0 ,  "StringHash COLLISION FOUND");
		}
#endif

#ifndef PVR_STRING_HASH_STRONG_COMPARISONS
		return (_Hash == str.getHash());
#else
		return (_Hash == str.getHash()) && _String == str._String;
#endif
	}

	/// <summary>Equality Operator. This function performs a strcmp(), so it is orders of magnitude slower than comparing
	/// to another StringHash, but still much faster than creating a temporary StringHash for one comparison.
	/// </summary>
	/// <param name="str">A string to compare with</param>
	/// <returns>True if they are the same.</returns>
	bool operator==(const char* str) const {	return (_String.compare(str) == 0); }

	/// <summary>Equality Operator. This function performs a string comparison so it is orders of magnitude slower than
	/// comparing to another StringHash, but still much faster than creating a temporary StringHash for one
	/// comparison.</summary>
	/// <param name="str">A string to compare with</param>
	/// <returns>True if they are the same.</returns>
	bool operator==(const std::string& str) const {	return _String == str; }

	/// <summary>Inequality Operator. Compares hash values. Extremely fast.</summary>
	/// <param name="str">A StringHash to compare with</param>
	/// <returns>True if they don't match</returns>
	bool operator!=(const StringHash& str) const { return !(*this == str); }

	bool operator<(const StringHash& str)const
	{
#ifndef DEBUG //Collision detection
        if(_Hash == str.getHash() && _String != str._String)
        {
            assertion(false, strings::createFormatted("HASH COLLISION DETECTED with %s and %s",_String.c_str(),str._String.c_str()).c_str());
        }

#endif
		return _Hash < str.getHash() || (_Hash == str.getHash() && _String < str._String);
	}

    /// <summary>Greater-than operator</summary>
    /// <param name="str">Right hand side of the operator (StringHash)</param>
    /// <returns>Return true if left-hand side is greater than right-hand side</returns>
	bool operator>(const StringHash& str)const
	{
		return str < *this;
	}

    /// <summary>Less-than or equal operator</summary>
    /// <param name="str">Right hand side of the operator (StringHash)</param>
    /// <returns>Return true if left-hand side is less than or equal to the right-hand side</returns>
	bool operator<=(const StringHash& str)const
	{
		return !(str > *this);
	}

    /// <summary>Greater-than or equal operator</summary>
    /// <param name="str">Right hand side of the operator (StringHash)</param>
    /// <returns>Return true if left-hand side is greater-than or equal to the right-hand side</returns>
	bool operator>=(const StringHash& str)const
	{
		return !(str < *this);
	}

	/// <summary>Get the base string object used by this StringHash object</summary>
	/// <returns>The base string object contained in this string hash.</returns>
	const string& str() const { return _String; }

	/// <summary>Get the base string object used by this StringHash object</summary>
	/// <returns>The hash value of this StringHash.</returns>
    std::size_t getHash() const { return _Hash; }

	/// <summary>Get the base string object used by this StringHash object</summary>
	/// <returns>A c-string representation of the contained string.</returns>
	const char* c_str() const { return _String.c_str(); }

private:
	std::string _String;
	std::size_t _Hash;
};
}