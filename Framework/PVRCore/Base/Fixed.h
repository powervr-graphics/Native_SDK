/*!
\brief Software implementation of a fixed point number
\file PVRCore/Base/Fixed.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/Base/Assert_.h"

/// <summary>ICC30 s15Fixed16Number representation for 32-bit signed fixed values, and u16Fixed16Number for 32-bit
/// unsigned fixed values.</summary>
/// <typeparam name="IntegerType">The representation type.</typeparam>
/// <typeparam name="IsSigned">This instance is signed.</typeparam>
/// <remarks>Always uses unsigned values for its storage type. Uses sign, bias and magnitude for signed values.
/// </remarks>
template <typename IntegerType, bool IsSigned>
class FixedPoint
{
public:
	FixedPoint();
	FixedPoint(const FixedPoint& value);
	FixedPoint(const double value);

	// Creation function to construct from a known fixed point value.
	static FixedPoint createFromFixedInteger(IntegerType fixedPointValue);

	// Operators
	FixedPoint& operator=(const FixedPoint& rhs);
	FixedPoint& operator=(const double rhs);

	// Operators to return a 32-bit double, allowing most double functionality to be shared.
	operator double();
	operator const double() const;

	// Arithmetic Operators with l-values
	FixedPoint& operator+=(FixedPoint rhs);
	FixedPoint& operator-=(FixedPoint rhs);
	FixedPoint& operator/=(FixedPoint rhs);
	FixedPoint& operator*=(FixedPoint rhs);
	FixedPoint& operator+=(double rhs);
	FixedPoint& operator-=(double rhs);
	FixedPoint& operator/=(double rhs);
	FixedPoint& operator*=(double rhs);
	FixedPoint& operator++(int);
	FixedPoint& operator--(int);
	FixedPoint& operator++();
	FixedPoint& operator--();

	// Arithmetic Operators
	const FixedPoint operator+(const FixedPoint rhs) const;
	const FixedPoint operator-(const FixedPoint rhs) const;
	const FixedPoint operator/(const FixedPoint rhs) const;
	const FixedPoint operator*(const FixedPoint rhs) const;
	const FixedPoint operator+(const double rhs) const;
	const FixedPoint operator-(const double rhs) const;
	const FixedPoint operator/(const double rhs) const;
	const FixedPoint operator*(const double rhs) const;

	// Logic Operators
	bool operator! () const;
	operator bool  () const;
	bool operator!=(const FixedPoint rhs) const;
	bool operator==(const FixedPoint rhs) const;
	bool operator< (const double rhs) const;
	bool operator> (const double rhs) const;
	bool operator<=(const double rhs) const;
	bool operator>=(const double rhs) const;

	// Utility functions
	static inline const FixedPoint getMaximumAbsoluteValue();
	static inline const FixedPoint getMinimumAbsoluteValue();
	static inline const FixedPoint getLowestValue();

	// Bit Manipulation
	inline const void setFixedIntegerValue(IntegerType fixedPointValue);
	inline const IntegerType getFixedIntegerValue();
		
private:
	IntegerType _value;
	
	// A bit field structure that acts as a way of quickly splitting the integral and fractional parts.
	struct BitFields
	{
		IntegerType fractionalPart : (sizeof(IntegerType) * (8 / 2) );
		IntegerType integralPart   : (sizeof(IntegerType) * (8 / 2) );
	};

	// Signed variant of the bit field structure
	struct SignedBitFields
	{
		IntegerType fractionalPart : (sizeof(IntegerType) * (8 / 2) );
		IntegerType integralPart   : (sizeof(IntegerType) * (8 / 2) - 1);
		IntegerType signBit        : (1);
	};

	inline void packDouble(double value);
	inline double unpackDouble() const;
	
	// Integer maximum
	static const IntegerType c_integerMaximum = static_cast<IntegerType>(-1);

	// The multiplier is the value when the bits in the least significant half are all one, or the maximum value if it's normalised
	static const IntegerType c_multiplier = static_cast<IntegerType>(1) << (sizeof(IntegerType) * (8 / 2) );
};

#include <cmath>

template <typename IntegerType, bool IsSigned>
FixedPoint<IntegerType, IsSigned>::FixedPoint()
{
	PVR_STATIC_ASSERT( !(static_cast<IntegerType>(-1) < static_cast<IntegerType>(0) ), cannotUseSignedValues);
}

template <typename IntegerType, bool IsSigned>
FixedPoint<IntegerType, IsSigned>::FixedPoint(const FixedPoint& value) : _value(value._value) 
{
	PVR_STATIC_ASSERT( !(static_cast<IntegerType>(-1) < static_cast<IntegerType>(0) ), cannotUseSignedValues);
}

template <typename IntegerType, bool IsSigned>
FixedPoint<IntegerType, IsSigned>::FixedPoint(const double value)
{
	PVR_STATIC_ASSERT( !(static_cast<IntegerType>(-1) < static_cast<IntegerType>(0) ), cannotUseSignedValues);
	packDouble(value);
}

template <typename IntegerType, bool IsSigned>
FixedPoint<IntegerType, IsSigned> FixedPoint<IntegerType, IsSigned>::createFromFixedInteger( IntegerType fixedPointValue )
{
	PVR_STATIC_ASSERT( !(static_cast<IntegerType>(-1) < static_cast<IntegerType>(0) ), cannotUseSignedValues);
	FixedPoint<IntegerType, IsSigned> newFixed; 
	newFixed.setFixedIntegerValue(fixedPointValue); 
	return newFixed;
}

template <typename IntegerType, bool IsSigned>
FixedPoint<IntegerType, IsSigned>& FixedPoint<IntegerType, IsSigned>::operator=(const FixedPoint& rhs)
{
	_value=rhs._value; 
	return *this;
}

template <typename IntegerType, bool IsSigned>
FixedPoint<IntegerType, IsSigned>& FixedPoint<IntegerType, IsSigned>::operator=(const double rhs)
{
	packDouble(rhs);
	return *this;
}

template <typename IntegerType, bool IsSigned>
FixedPoint<IntegerType, IsSigned>::operator double()
{
	return unpackDouble();
}

template <typename IntegerType, bool IsSigned>
FixedPoint<IntegerType, IsSigned>::operator const double() const
{
	return unpackDouble();
}

template <typename IntegerType, bool IsSigned>
FixedPoint<IntegerType, IsSigned>& FixedPoint<IntegerType, IsSigned>::operator+= (FixedPoint<IntegerType, IsSigned> rhs)
{
	_value += rhs._value;
	return *this;
}

template <typename IntegerType, bool IsSigned>
FixedPoint<IntegerType, IsSigned>& FixedPoint<IntegerType, IsSigned>::operator-= (FixedPoint<IntegerType, IsSigned> rhs)
{
	_value -= rhs._value;
	return *this;
}

template <typename IntegerType, bool IsSigned>
FixedPoint<IntegerType, IsSigned>& FixedPoint<IntegerType, IsSigned>::operator/= (FixedPoint<IntegerType, IsSigned> rhs)
{
	return *this /= rhs.unpackDouble();
}

template <typename IntegerType, bool IsSigned>
FixedPoint<IntegerType, IsSigned>& FixedPoint<IntegerType, IsSigned>::operator*= (FixedPoint<IntegerType, IsSigned> rhs)
{
	return *this *= rhs.unpackDouble();
}

template <typename IntegerType, bool IsSigned>
FixedPoint<IntegerType, IsSigned>& FixedPoint<IntegerType, IsSigned>::operator+= (double rhs)
{
	_value += (rhs * c_multiplier);
	return *this;
}

template <typename IntegerType, bool IsSigned>
FixedPoint<IntegerType, IsSigned>& FixedPoint<IntegerType, IsSigned>::operator-= (double rhs)
{
	_value -= (rhs * c_multiplier);
	return *this;
}

template <typename IntegerType, bool IsSigned>
FixedPoint<IntegerType, IsSigned>& FixedPoint<IntegerType, IsSigned>::operator/= (double rhs)
{
	_value /= rhs;
	return *this;
}

template <typename IntegerType, bool IsSigned>
FixedPoint<IntegerType, IsSigned>& FixedPoint<IntegerType, IsSigned>::operator*= (double rhs)
{
	_value *= rhs;
	return *this;
}

template <typename IntegerType, bool IsSigned>
FixedPoint<IntegerType, IsSigned>& FixedPoint<IntegerType, IsSigned>::operator++ (int)
{
	_value += c_multiplier;
	return *this;
}

template <typename IntegerType, bool IsSigned>
FixedPoint<IntegerType, IsSigned>& FixedPoint<IntegerType, IsSigned>::operator-- (int)
{
	_value -= c_multiplier;
	return *this;
}

template <typename IntegerType, bool IsSigned>
FixedPoint<IntegerType, IsSigned>& FixedPoint<IntegerType, IsSigned>::operator++ ()
{
	_value += c_multiplier;
	return *this;
}

template <typename IntegerType, bool IsSigned>
FixedPoint<IntegerType, IsSigned>& FixedPoint<IntegerType, IsSigned>::operator-- ()
{
	_value -= c_multiplier;
	return *this;
}

template <typename IntegerType, bool IsSigned>
const FixedPoint<IntegerType, IsSigned> FixedPoint<IntegerType, IsSigned>::operator+ (FixedPoint<IntegerType, IsSigned> rhs) const
{
	return createFromFixedInteger(_value + rhs._value);
}

template <typename IntegerType, bool IsSigned>
const FixedPoint<IntegerType, IsSigned> FixedPoint<IntegerType, IsSigned>::operator- (FixedPoint<IntegerType, IsSigned> rhs) const
{
	return createFromFixedInteger(_value - rhs._value);
}

template <typename IntegerType, bool IsSigned>
const FixedPoint<IntegerType, IsSigned> FixedPoint<IntegerType, IsSigned>::operator/ (FixedPoint<IntegerType, IsSigned> rhs) const
{
	return createFromFixedInteger(_value / rhs.unpackDouble());
}

template <typename IntegerType, bool IsSigned>
const FixedPoint<IntegerType, IsSigned> FixedPoint<IntegerType, IsSigned>::operator* (FixedPoint<IntegerType, IsSigned> rhs) const
{
	return createFromFixedInteger(_value * rhs.unpackDouble());
}

template <typename IntegerType, bool IsSigned>
const FixedPoint<IntegerType, IsSigned> FixedPoint<IntegerType, IsSigned>::operator+ (double rhs) const
{
	return createFromFixedInteger(_value + (rhs * c_multiplier) );
}

template <typename IntegerType, bool IsSigned>
const FixedPoint<IntegerType, IsSigned> FixedPoint<IntegerType, IsSigned>::operator- (double rhs) const
{
	return createFromFixedInteger(_value - (rhs * c_multiplier) );
}

template <typename IntegerType, bool IsSigned>
const FixedPoint<IntegerType, IsSigned> FixedPoint<IntegerType, IsSigned>::operator/ (double rhs) const
{
	return createFromFixedInteger(_value / rhs);
}

template <typename IntegerType, bool IsSigned>
const FixedPoint<IntegerType, IsSigned> FixedPoint<IntegerType, IsSigned>::operator* (double rhs) const
{
	return createFromFixedInteger(_value * rhs);
}

template <typename IntegerType, bool IsSigned>
bool FixedPoint<IntegerType, IsSigned>::operator!() const
{
	return _value == 0;
}

template <typename IntegerType, bool IsSigned>
FixedPoint<IntegerType, IsSigned>::operator bool() const
{
	return _value != 0;
}

template <typename IntegerType, bool IsSigned>
bool FixedPoint<IntegerType, IsSigned>::operator!=( const FixedPoint rhs ) const
{
	return _value != rhs._value;
}

template <typename IntegerType, bool IsSigned>
bool FixedPoint<IntegerType, IsSigned>::operator==( const FixedPoint rhs ) const
{
	return _value == rhs._value;
}

template <typename IntegerType, bool IsSigned>
bool FixedPoint<IntegerType, IsSigned>::operator<( const double rhs ) const
{
	return _value < rhs._value;
}

template <typename IntegerType, bool IsSigned>
bool FixedPoint<IntegerType, IsSigned>::operator>( const double rhs ) const
{
	return _value > rhs._value;
}

template <typename IntegerType, bool IsSigned>
bool FixedPoint<IntegerType, IsSigned>::operator<=( const double rhs ) const
{
	return _value <= rhs._value;
}

template <typename IntegerType, bool IsSigned>
bool FixedPoint<IntegerType, IsSigned>::operator>=( const double rhs ) const
{
	return _value >= rhs._value;
}

template <typename IntegerType, bool IsSigned>
const FixedPoint<IntegerType, IsSigned> FixedPoint<IntegerType, IsSigned>::getMaximumAbsoluteValue()
{
	return createFromFixedInteger(IsSigned ? (c_integerMaximum >> 1) : c_integerMaximum );
}

template <typename IntegerType, bool IsSigned>
const FixedPoint<IntegerType, IsSigned> FixedPoint<IntegerType, IsSigned>::getMinimumAbsoluteValue()
{
	return createFromFixedInteger(1);
}

template <typename IntegerType, bool IsSigned>
const FixedPoint<IntegerType, IsSigned> FixedPoint<IntegerType, IsSigned>::getLowestValue()
{
	return createFromFixedInteger(IsSigned ? c_integerMaximum : 0);
}

template <typename IntegerType, bool IsSigned>
const void FixedPoint<IntegerType, IsSigned>::setFixedIntegerValue( IntegerType fixedPointValue )
{
	_value = fixedPointValue;
}

template <typename IntegerType, bool IsSigned>
const IntegerType FixedPoint<IntegerType, IsSigned>::getFixedIntegerValue()
{
	return _value;
}

template <typename IntegerType, bool IsSigned>
void FixedPoint<IntegerType, IsSigned>::packDouble(double value)
{
	// Check if the value needs to be clamped.
	if (value >= getMaximumAbsoluteValue().unpackDouble() )
	{
		_value = getMaximumAbsoluteValue()._value;
	}
	else if (value <= getLowestValue().unpackDouble() )
	{
		_value = getLowestValue()._value;
	}
	else
	{
		if (IsSigned)
		{
			// Access the bit fields of the storage type directly.
			SignedBitFields& bitFields = reinterpret_cast<SignedBitFields&>(_value);

			double absoluteValue = fabs(value);

			bitFields.signBit = (value < 0) ? 1 : 0;
			bitFields.integralPart = (static_cast<IntegerType>(absoluteValue) - bitFields.signBit);
			bitFields.fractionalPart = static_cast<IntegerType>( (absoluteValue - floor(absoluteValue) ) * static_cast<double>(c_multiplier) );
		}
		else
		{
			// Access the bit fields of the storage type directly.
			BitFields& bitFields = reinterpret_cast<BitFields&>(_value);

			bitFields.fractionalPart = static_cast<IntegerType>( (value - floor(value) ) * static_cast<double>(c_multiplier) );
		}
	}
}

template <typename IntegerType, bool IsSigned>
double FixedPoint<IntegerType, IsSigned>::unpackDouble() const
{
	// If it's signed, need special behaviour to handle the sign bit.
	if (IsSigned)
	{
		SignedBitFields bitFields = reinterpret_cast<const SignedBitFields&>(_value);

		double result = static_cast<double>(bitFields.fractionalPart) / static_cast<double>(c_multiplier);

		result += static_cast<double>(bitFields.integralPart);

		result += bitFields.signBit;

		result *= bitFields.signBit ? -1.0 : 1.0;

		return result;
	}
	else
	{
		BitFields bitFields = reinterpret_cast<const BitFields&>(_value);

		double result = static_cast<double>(bitFields.fractionalPart) / static_cast<double>(c_multiplier);

		result += static_cast<double>(bitFields.integralPart);

		return result;
	}
}