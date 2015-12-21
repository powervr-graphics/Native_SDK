/*!*********************************************************************************************************************
\file         PVRCore\NormalisedInteger.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Implementation of a Normalised Integer (an integer representing a fixed point value from 0..1).
***********************************************************************************************************************/
#pragma once
#include <limits>

namespace pvr
{
/*!****************************************************************************************************************
\brief     Implementation of a Normalised Integer (an integer representing a fixed point value from 0..1). Contains 
           functions to construct from Integer values, to convert to and from floating point values, arithmetic
		   operators and similar.
*******************************************************************************************************************/
template <typename IntegerType>
class NormalisedInteger
{
public:
	// Constructors
	/*!****************************************************************************************************************
	\brief     Default constructor. Undefined value.
	*******************************************************************************************************************/
	NormalisedInteger() {}

	/*!****************************************************************************************************************
	\brief     Constructor from a floating point value assumed to be in 0..1 range.
	*******************************************************************************************************************/
	NormalisedInteger(const double value) { packDouble(value); }

	/*!****************************************************************************************************************
	\brief     Creates from an integer that represent a normalised value.
	*******************************************************************************************************************/
	static NormalisedInteger createFromIntegerValue(IntegerType normalisedValue);

	/*!****************************************************************************************************************
	\brief     Assignment from double assumed to contain a value in the 0..1 range.
	*******************************************************************************************************************/
	NormalisedInteger& operator=(const double rhs);

	/*!****************************************************************************************************************
	\brief     Convert into double in the 0..1 range.
	*******************************************************************************************************************/
	operator double() const;

	NormalisedInteger& operator+=(NormalisedInteger rhs);
	NormalisedInteger& operator-=(NormalisedInteger rhs);
	NormalisedInteger& operator/=(NormalisedInteger rhs);
	NormalisedInteger& operator*=(NormalisedInteger rhs);
	NormalisedInteger& operator+=(double rhs);
	NormalisedInteger& operator-=(double rhs);
	NormalisedInteger& operator/=(double rhs);
	NormalisedInteger& operator*=(double rhs);
	NormalisedInteger& operator++(int);
	NormalisedInteger& operator--(int);
	NormalisedInteger& operator++();
	NormalisedInteger& operator--();

	// Arithmetic Operators
	
	const NormalisedInteger operator+(const NormalisedInteger rhs) const;
	const NormalisedInteger operator-(const NormalisedInteger rhs) const;
	const NormalisedInteger operator/(const NormalisedInteger rhs) const;
	const NormalisedInteger operator*(const NormalisedInteger rhs) const;
	const NormalisedInteger operator+(const double rhs) const;
	const NormalisedInteger operator-(const double rhs) const;
	const NormalisedInteger operator/(const double rhs) const;
	const NormalisedInteger operator*(const double rhs) const;

	// Logic Operators
	
	bool operator!=(const NormalisedInteger rhs) const;
	bool operator==(const NormalisedInteger rhs) const;
	bool operator< (const NormalisedInteger rhs) const;
	bool operator> (const NormalisedInteger rhs) const;
	bool operator<=(const NormalisedInteger rhs) const;
	bool operator>=(const NormalisedInteger rhs) const;

	// Utility functions
	static inline const NormalisedInteger max();
	static inline const NormalisedInteger min();

	// Bit Manipulation
	/*!****************************************************************************************************************
	\brief     Set the Normalised Integer from an integer reinterpreted to a normalised value.
	\param normalisedValue Set the Normalised Integer from an integer reinterpreted to a normalised value
	*******************************************************************************************************************/
	inline const void setIntegerValue(IntegerType normalisedValue);
	/*!****************************************************************************************************************
	\return    An integer containing the normalised value (i.e. ReturnValue = RepresentedValue * IntegerTypeMaxSize
	*******************************************************************************************************************/
	inline const IntegerType getNormalisedIntegerValue();

private:
	IntegerType m_value;

	inline void packDouble(double value);
	inline double unpackDouble() const;
};


template <typename IntegerType>
NormalisedInteger<IntegerType> NormalisedInteger<IntegerType>::createFromIntegerValue( IntegerType normalisedValue )
{
	NormalisedInteger<IntegerType> newNormalised; 
	newNormalised.setIntegerValue(normalisedValue); 
	return newNormalised;
}


template <typename IntegerType>
NormalisedInteger<IntegerType>& NormalisedInteger<IntegerType>::operator=(const double rhs)
{
	packDouble(rhs);
	return *this;
}

template <typename IntegerType>
NormalisedInteger<IntegerType>::operator double() const
{
	return unpackDouble();
}

template <typename IntegerType>
NormalisedInteger<IntegerType>& NormalisedInteger<IntegerType>::operator+= (NormalisedInteger<IntegerType> rhs)
{
	m_value += rhs.m_value;
	return *this;
}

template <typename IntegerType>
NormalisedInteger<IntegerType>& NormalisedInteger<IntegerType>::operator-= (NormalisedInteger<IntegerType> rhs)
{
	m_value -= rhs.m_value;
	return *this;
}

template <typename IntegerType>
NormalisedInteger<IntegerType>& NormalisedInteger<IntegerType>::operator/= (NormalisedInteger<IntegerType> rhs)
{
	return *this /= rhs.unpackDouble();
}

template <typename IntegerType>
NormalisedInteger<IntegerType>& NormalisedInteger<IntegerType>::operator*= (NormalisedInteger<IntegerType> rhs)
{
	return *this *= rhs.unpackDouble();
}

template <typename IntegerType>
NormalisedInteger<IntegerType>& NormalisedInteger<IntegerType>::operator+= (double rhs)
{
	m_value += static_cast<IntegerType>(rhs * static_cast<double>(std::numeric_limits<IntegerType>::max()));
	return *this;
}

template <typename IntegerType>
NormalisedInteger<IntegerType>& NormalisedInteger<IntegerType>::operator-= (double rhs)
{
	m_value -= static_cast<IntegerType>(rhs * static_cast<double>(std::numeric_limits<IntegerType>::max()));
	return *this;
}

template <typename IntegerType>
NormalisedInteger<IntegerType>& NormalisedInteger<IntegerType>::operator/= (double rhs)
{
	m_value /= rhs;
	return *this;
}

template <typename IntegerType>
NormalisedInteger<IntegerType>& NormalisedInteger<IntegerType>::operator*= (double rhs)
{
	m_value *= rhs;
	return *this;
}

template <typename IntegerType>
NormalisedInteger<IntegerType>& NormalisedInteger<IntegerType>::operator++ (int)
{
	m_value += std::numeric_limits<IntegerType>::max();
	return *this;
}

template <typename IntegerType>
NormalisedInteger<IntegerType>& NormalisedInteger<IntegerType>::operator-- (int)
{
	m_value -= std::numeric_limits<IntegerType>::max();
	return *this;
}

template <typename IntegerType>
NormalisedInteger<IntegerType>& NormalisedInteger<IntegerType>::operator++ ()
{
	m_value += std::numeric_limits<IntegerType>::max();
	return *this;
}

template <typename IntegerType>
NormalisedInteger<IntegerType>& NormalisedInteger<IntegerType>::operator-- ()
{
	m_value -= std::numeric_limits<IntegerType>::max();
	return *this;
}

template <typename IntegerType>
const NormalisedInteger<IntegerType> NormalisedInteger<IntegerType>::operator+ (NormalisedInteger<IntegerType> rhs) const
{
	return createFromIntegerValue(m_value + rhs.m_value);
}

template <typename IntegerType>
const NormalisedInteger<IntegerType> NormalisedInteger<IntegerType>::operator- (NormalisedInteger<IntegerType> rhs) const
{
	return createFromIntegerValue(m_value - rhs.m_value);
}

template <typename IntegerType>
const NormalisedInteger<IntegerType> NormalisedInteger<IntegerType>::operator/ (NormalisedInteger<IntegerType> rhs) const
{
	return createFromIntegerValue(m_value / rhs.unpackDouble());
}

template <typename IntegerType>
const NormalisedInteger<IntegerType> NormalisedInteger<IntegerType>::operator* (NormalisedInteger<IntegerType> rhs) const
{
	return createFromIntegerValue(m_value * rhs.unpackDouble());
}

template <typename IntegerType>
const NormalisedInteger<IntegerType> NormalisedInteger<IntegerType>::operator+ (double rhs) const
{
	return createFromIntegerValue(m_value + static_cast<IntegerType>(rhs * static_cast<double>(std::numeric_limits<IntegerType>::max())));
}

template <typename IntegerType>
const NormalisedInteger<IntegerType> NormalisedInteger<IntegerType>::operator- (double rhs) const
{
	return createFromIntegerValue(m_value - static_cast<IntegerType>(rhs * static_cast<double>(std::numeric_limits<IntegerType>::max())));
}

template <typename IntegerType>
const NormalisedInteger<IntegerType> NormalisedInteger<IntegerType>::operator/ (double rhs) const
{
	return createFromIntegerValue(m_value / rhs);
}

template <typename IntegerType>
const NormalisedInteger<IntegerType> NormalisedInteger<IntegerType>::operator* (double rhs) const
{
	return createFromIntegerValue(m_value * rhs);
}

/////////////////////////////////// Logic Operators ///////////////////////////////////
template <typename IntegerType>
bool NormalisedInteger<IntegerType>::operator!=( const NormalisedInteger rhs ) const
{
	return m_value != rhs.m_value;
}

template <typename IntegerType>
bool NormalisedInteger<IntegerType>::operator==( const NormalisedInteger rhs ) const
{
	return m_value == rhs.m_value;
}

template <typename IntegerType>
bool NormalisedInteger<IntegerType>::operator<( const NormalisedInteger rhs ) const
{
	return m_value < rhs.m_value;
}

template <typename IntegerType>
bool NormalisedInteger<IntegerType>::operator>( const NormalisedInteger rhs ) const
{
	return m_value > rhs.m_value;
}

template <typename IntegerType>
bool NormalisedInteger<IntegerType>::operator<=( const NormalisedInteger rhs ) const
{
	return m_value <= rhs.m_value;
}

template <typename IntegerType>
bool NormalisedInteger<IntegerType>::operator>=( const NormalisedInteger rhs ) const
{
	return m_value >= rhs.m_value;
}

/*!*********************************************************************************************************************
 Type Queries
***********************************************************************************************************************/
template <typename IntegerType>
const NormalisedInteger<IntegerType> NormalisedInteger<IntegerType>::max()
{
	return createFromIntegerValue(std::numeric_limits<IntegerType>::max());
}

template <typename IntegerType>
const NormalisedInteger<IntegerType> NormalisedInteger<IntegerType>::min()
{
	return createFromIntegerValue(std::numeric_limits<IntegerType>::min());
}

/*!*********************************************************************************************************************
 Bit Manipulation
***********************************************************************************************************************/
template <typename IntegerType>
const void NormalisedInteger<IntegerType>::setIntegerValue( IntegerType normalisedValue )
{
	m_value = normalisedValue;
}

template <typename IntegerType>
const IntegerType NormalisedInteger<IntegerType>::getNormalisedIntegerValue()
{
	return m_value;
}

/*!*********************************************************************************************************************
 Internal Functions
***********************************************************************************************************************/
template <typename IntegerType>
void NormalisedInteger<IntegerType>::packDouble(double value)
{
	// Check if the value needs to be clamped.
	if (value > 1.0)
	{
		m_value = std::numeric_limits<IntegerType>::max();
	}
	else if (value <= -1.0)
	{
		m_value = std::numeric_limits<IntegerType>::min();
	}
	else
	{
		m_value = static_cast<IntegerType>(value * static_cast<double>(std::numeric_limits<IntegerType>::max()));
	}
}

template <typename IntegerType>
double NormalisedInteger<IntegerType>::unpackDouble() const
{
	double value = (static_cast<double>(m_value) / static_cast<double>(std::numeric_limits<IntegerType>::max()));
	
	if (value > 1.0)
	{
		return 1.0;
	}
	else if (value < -1.0)
	{
		return -1.0;
	}
	else
	{
		return value;
	}
}

}