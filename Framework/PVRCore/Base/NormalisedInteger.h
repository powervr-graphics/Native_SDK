/*!
\brief Implementation of a Normalised Integer (an integer representing a fixed point value from 0..1).
\file PVRCore/Base/NormalisedInteger.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include <limits>

namespace pvr
{
/// <summary>Implementation of a Normalised Integer (an integer representing a fixed point value from 0..1). Contains
/// functions to construct from Integer values, to convert to and from floating point values, arithmetic operators
/// and similar.</summary>
/// <typeparam name="IntegerType">The underlying type that will be integer. This class will utilise the full
/// precision of the underlying integer to represent the 0..1 range.</typeparam>
template <typename IntegerType>
class NormalisedInteger
{
public:
	// Constructors
	/// <summary>Default constructor. Undefined value.</summary>
	NormalisedInteger() {}

	/// <summary>Constructor from a floating point value. Assumed to be in 0..1 range.</summary>
	/// <param name="value">The value to pack into the integer. Final value is undefined if not in the 0..1 range.
	/// </param>
	NormalisedInteger(const double value) { packDouble(value); }

	/// <summary>Creates from an integer that represent a normalised value.</summary>
	static NormalisedInteger createFromIntegerValue(IntegerType normalisedValue);

	/// <summary>Assignment from double assumed to contain a value in the 0..1 range.</summary>
	NormalisedInteger& operator=(const double rhs);

	/// <summary>Convert into double in the 0..1 range.</summary>
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

	/// <summary>Set the Normalised Integer from an integer reinterpreted to a normalised value. The value is utilised
	/// as-is, which means 0 will represent 0 and its maximum value will represent 1.</summary>
	/// <param name="normalisedValue">Set the Normalised Integer from an integer reinterpreted to a normalised value
	/// </param>
	inline const void setIntegerValue(IntegerType normalisedValue);
	/// <summary>get the internal (integer) representation of the integer.</summary>
	/// <returns>An integer containing the normalised value (i.e. ReturnValue = RepresentedValue * IntegerTypeMaxSize
	/// </returns>
	inline const IntegerType getNormalisedIntegerValue();

private:
	IntegerType _value;

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
	_value += rhs._value;
	return *this;
}

template <typename IntegerType>
NormalisedInteger<IntegerType>& NormalisedInteger<IntegerType>::operator-= (NormalisedInteger<IntegerType> rhs)
{
	_value -= rhs._value;
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
	_value += static_cast<IntegerType>(rhs * static_cast<double>(std::numeric_limits<IntegerType>::max()));
	return *this;
}

template <typename IntegerType>
NormalisedInteger<IntegerType>& NormalisedInteger<IntegerType>::operator-= (double rhs)
{
	_value -= static_cast<IntegerType>(rhs * static_cast<double>(std::numeric_limits<IntegerType>::max()));
	return *this;
}

template <typename IntegerType>
NormalisedInteger<IntegerType>& NormalisedInteger<IntegerType>::operator/= (double rhs)
{
	_value /= rhs;
	return *this;
}

template <typename IntegerType>
NormalisedInteger<IntegerType>& NormalisedInteger<IntegerType>::operator*= (double rhs)
{
	_value *= rhs;
	return *this;
}

template <typename IntegerType>
NormalisedInteger<IntegerType>& NormalisedInteger<IntegerType>::operator++ (int)
{
	_value += std::numeric_limits<IntegerType>::max();
	return *this;
}

template <typename IntegerType>
NormalisedInteger<IntegerType>& NormalisedInteger<IntegerType>::operator-- (int)
{
	_value -= std::numeric_limits<IntegerType>::max();
	return *this;
}

template <typename IntegerType>
NormalisedInteger<IntegerType>& NormalisedInteger<IntegerType>::operator++ ()
{
	_value += std::numeric_limits<IntegerType>::max();
	return *this;
}

template <typename IntegerType>
NormalisedInteger<IntegerType>& NormalisedInteger<IntegerType>::operator-- ()
{
	_value -= std::numeric_limits<IntegerType>::max();
	return *this;
}

template <typename IntegerType>
const NormalisedInteger<IntegerType> NormalisedInteger<IntegerType>::operator+ (NormalisedInteger<IntegerType> rhs) const
{
	return createFromIntegerValue(_value + rhs._value);
}

template <typename IntegerType>
const NormalisedInteger<IntegerType> NormalisedInteger<IntegerType>::operator- (NormalisedInteger<IntegerType> rhs) const
{
	return createFromIntegerValue(_value - rhs._value);
}

template <typename IntegerType>
const NormalisedInteger<IntegerType> NormalisedInteger<IntegerType>::operator/ (NormalisedInteger<IntegerType> rhs) const
{
	return createFromIntegerValue(_value / rhs.unpackDouble());
}

template <typename IntegerType>
const NormalisedInteger<IntegerType> NormalisedInteger<IntegerType>::operator* (NormalisedInteger<IntegerType> rhs) const
{
	return createFromIntegerValue(_value * rhs.unpackDouble());
}

template <typename IntegerType>
const NormalisedInteger<IntegerType> NormalisedInteger<IntegerType>::operator+ (double rhs) const
{
	return createFromIntegerValue(_value + static_cast<IntegerType>(rhs * static_cast<double>(std::numeric_limits<IntegerType>::max())));
}

template <typename IntegerType>
const NormalisedInteger<IntegerType> NormalisedInteger<IntegerType>::operator- (double rhs) const
{
	return createFromIntegerValue(_value - static_cast<IntegerType>(rhs * static_cast<double>(std::numeric_limits<IntegerType>::max())));
}

template <typename IntegerType>
const NormalisedInteger<IntegerType> NormalisedInteger<IntegerType>::operator/ (double rhs) const
{
	return createFromIntegerValue(_value / rhs);
}

template <typename IntegerType>
const NormalisedInteger<IntegerType> NormalisedInteger<IntegerType>::operator* (double rhs) const
{
	return createFromIntegerValue(_value * rhs);
}

/////////////////////////////////// Logic Operators ///////////////////////////////////
template <typename IntegerType>
bool NormalisedInteger<IntegerType>::operator!=( const NormalisedInteger rhs ) const
{
	return _value != rhs._value;
}

template <typename IntegerType>
bool NormalisedInteger<IntegerType>::operator==( const NormalisedInteger rhs ) const
{
	return _value == rhs._value;
}

template <typename IntegerType>
bool NormalisedInteger<IntegerType>::operator<( const NormalisedInteger rhs ) const
{
	return _value < rhs._value;
}

template <typename IntegerType>
bool NormalisedInteger<IntegerType>::operator>( const NormalisedInteger rhs ) const
{
	return _value > rhs._value;
}

template <typename IntegerType>
bool NormalisedInteger<IntegerType>::operator<=( const NormalisedInteger rhs ) const
{
	return _value <= rhs._value;
}

template <typename IntegerType>
bool NormalisedInteger<IntegerType>::operator>=( const NormalisedInteger rhs ) const
{
	return _value >= rhs._value;
}

// Type Queries
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

// Bit Manipulation
template <typename IntegerType>
const void NormalisedInteger<IntegerType>::setIntegerValue( IntegerType normalisedValue )
{
	_value = normalisedValue;
}

template <typename IntegerType>
const IntegerType NormalisedInteger<IntegerType>::getNormalisedIntegerValue()
{
	return _value;
}

// Internal Functions
template <typename IntegerType>
void NormalisedInteger<IntegerType>::packDouble(double value)
{
	// Check if the value needs to be clamped.
	if (value > 1.0)
	{
		_value = std::numeric_limits<IntegerType>::max();
	}
	else if (value <= -1.0)
	{
		_value = std::numeric_limits<IntegerType>::min();
	}
	else
	{
		_value = static_cast<IntegerType>(value * static_cast<double>(std::numeric_limits<IntegerType>::max()));
	}
}

template <typename IntegerType>
double NormalisedInteger<IntegerType>::unpackDouble() const
{
	double value = (static_cast<double>(_value) / static_cast<double>(std::numeric_limits<IntegerType>::max()));
	
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