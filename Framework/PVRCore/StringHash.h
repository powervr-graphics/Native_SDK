/*!*********************************************************************************************************************
\file         PVRCore\StringHash.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        A hashed string with functionality for fast compares.
***********************************************************************************************************************/
#pragma once
#include "PVRCore/CoreIncludes.h"
#include "PVRCore/Hash_.h"
#include "PVRCore/StringFunctions.h"
#include <functional>

namespace pvr {
/*!*********************************************************************************************
\brief Implementation of a hashed string with functionality for fast compares.
\description In most cases, can be used as a drop-in replacement for std::strings to take advantage
		of fast hashed  comparisons. On debug builds, tests for hash collisions are performed
		(Assertion + error log if collision found)
 ***********************************************************************************************/
class StringHash
{
	typedef hash<std::string> HashFn;
public:
	/*!*********************************************************************************************
	\param[in]			str	A c-style string
	**********************************************************************************************/
	StringHash(const char* str) : m_String(str), m_Hash(HashFn()(m_String)) { }

	/*!*********************************************************************************************
	\param[in]			right	A string
	**********************************************************************************************/
	StringHash(const std::string& right) : m_String(right), m_Hash(HashFn()(m_String)) { }

	/*!*********************************************************************************************
	\brief Convert to string
	\return A string representatation of this hash
	************************************************************************************************/
	operator const std::string& () const { return m_String; }

	/*!*********************************************************************************************
	\brief ctor. Default constructor
	************************************************************************************************/
	StringHash() : m_String(""), m_Hash(HashFn()(m_String)) {}

	/*!*********************************************************************************************
	\brief      		Appends a string to the end of this StringHash, recalculates hash.
	\param[in]			ptr	A string
	\return 			This
	*************************************************************************/
	StringHash& append(const char* ptr)
	{
		m_String.append(ptr);
		m_Hash = HashFn()(m_String);
		return *this;
	}

	/*!**********************************************************************************************
	\brief      		Appends a string.
	\param[in]			str	A string
	\return 			Updated string
	************************************************************************************************/
	StringHash& append(const string& str)
	{
		m_String.append(str);
		m_Hash = HashFn()(m_String);
		return *this;
	}

	/*!**********************************************************************************************
	\brief      		Assigns the string to the string ptr.
	\param[in]			ptr A string
	\return 			Updated string
	************************************************************************************************/
	StringHash& assign(const char* ptr)
	{
		m_String.assign(ptr);
		m_Hash = HashFn()(m_String);
		return *this;
	}

	/*!**********************************************************************************************
	\brief      		Assigns the string to the string str.
	\param[in]			str A string
	\return 			Updated string
	************************************************************************************************/
	StringHash& assign(const string& str)
	{
		m_String = str;
		m_Hash = HashFn()(m_String);
		return *this;
	}

	/*!**********************************************************************************************
	\brief      		Return the length of this string hash
	\return 			Length of this string hash
	************************************************************************************************/
	size_t size() const { return m_String.size(); }

	/*!**********************************************************************************************
	\brief      		Return the length of this string hash
	\return 			Length of this string hash
	************************************************************************************************/
	size_t length() const { return m_String.length(); }

	/*!**********************************************************************************************
	\brief      		Return if the string is empty
	\return 			true if the string is emtpy (length=0), false otherwise
	************************************************************************************************/
	bool empty() const { return m_String.empty(); }

    /*!
       \brief Clear this string hash
     */
	void clear() { assign(string());  }

	/*!**********************************************************************************************
	\brief      	== Operator. This function compares the hash values of the StringHashes.
	\param[in]		str 	A hashed string to compare with
	\return 		True if they match
	************************************************************************************************/
	bool operator==(const StringHash& str) const
	{
#ifdef DEBUG //Collision detection
		if (m_Hash == str.getHash() && m_String != str.m_String)
		{
			Log(Log.Critical, "***** STRING HASH COLLISION DETECTED ********************");
			Log(Log.Critical, "** String [%s] collides with string [%s] ", m_String.c_str(), str.m_String.c_str());
			Log(Log.Critical, "*********************************************************");
			assertion(0 ,  "StringHash COLLISION FOUND");
		}
#endif

#ifndef PVR_STRING_HASH_STRONG_COMPARISONS
		return (m_Hash == str.getHash());
#else
		return (m_Hash == str.getHash()) && m_String == str.m_String;
#endif
	}

	/*!**********************************************************************************************
	\brief      	== Operator. This function performs a strcmp() as it's more efficient to strcmp
	                than to hash the string for every comparison.
	\param[in]		str 	A string to compare with
	\return 		True if they match
	************************************************************************************************/
	bool operator==(const char* str) const {	return (m_String.compare(str) == 0); }

	/*!**********************************************************************************************
	\brief      	== Operator. This function performs a strcmp()
	as it's more efficient to strcmp than to hash the string
	for every comparison.
	\param[in]		str 	A string to compare with
	\return 		True if they match
	************************************************************************************************/
	bool operator==(const std::string& str) const {	return m_String == str;	}

	/*!***********************************************************************
	\brief      	!= Operator. Compares hash values.
	\param[in]		str 	A StringHash to compare with
	\return 		True if they don't match
	*************************************************************************/
	bool operator!=(const StringHash& str) const { return !(*this == str);	}

	bool operator<(const StringHash& str)const
	{
#ifndef DEBUG //Collision detection
        if(m_Hash == str.getHash() && m_String != str.m_String)
        {
            assertion(false, strings::createFormatted("HASH COLLISION DETECTED with %s and %s",m_String.c_str(),str.m_String.c_str()).c_str());
        }

#endif
		return m_Hash < str.getHash() || (m_Hash == str.getHash() && m_String < str.m_String);
	}

    /*!
       \brief operator >
       \param str String hash to compare
       \return Return true if this hash is greater
     */
	bool operator>(const StringHash& str)const
	{
		return str < *this;
	}

    /*!
       \brief operator <=
       \param str String hash to compare
       \return Return true if this hash lessthan or equalto
     */
	bool operator<=(const StringHash& str)const
	{
		return !(str > *this);
	}

    /*!
       \brief operator >=
       \param str String hash to compare
       \return Return true if this hash greaterthan or equalto
     */
	bool operator>=(const StringHash& str)const
	{
		return !(str < *this);
	}

	/*!***********************************************************************
	\return 		The base string object contained.
	*************************************************************************/
	const string& str() const {	return m_String;	}

	/*!***********************************************************************
	\return 		The hash value of this StringHash.
	*************************************************************************/
    std::size_t getHash() const {	return m_Hash;	}

	/*!***************************************************************************
	\return			A c-string of the contained string.
	*****************************************************************************/
	const char* c_str() const {	return m_String.c_str(); }

private:
	std::string m_String;
	std::size_t m_Hash;
};
}
