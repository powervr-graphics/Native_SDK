/*!
\brief Software implementation of a 16 bit floating point number.
\file PVRCore/Base/HalfFloat.h
\author PowerVR by Imagination, Developer Technology Team.
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
//Half float class
#if defined(_WIN32)
#pragma pack(push,1)
#endif
namespace pvr {
/// <summary>Software implementation of a 16 bit floating point number.</summary>
class HalfFloat
{
public:
	// Constructors
	HalfFloat() {}
	HalfFloat(const HalfFloat& value) : _value(value._value) {}
	HalfFloat(float value) { packFloat(value); }

	// Operators
	HalfFloat& operator= (HalfFloat rhs)
	{
		_value = rhs._value;
		return *this;
	}

	HalfFloat& operator= (float rhs)
	{
		packFloat(rhs);
		return *this;
	}


	// Operators to return a 32-bit float, allowing most float functionality to be shared.
	operator float() const { return unpackFloat(); }

	HalfFloat& operator+= (HalfFloat rhs)
	{
		return ((*this) += rhs.unpackFloat());
	}

	HalfFloat& operator-= (HalfFloat rhs)
	{
		return ((*this) -= rhs.unpackFloat());
	}


	HalfFloat& operator/= (HalfFloat rhs)
	{
		return ((*this) /= rhs.unpackFloat());
	}

	HalfFloat& operator*= (HalfFloat rhs)
	{
		return ((*this) *= rhs.unpackFloat());
	}

	HalfFloat& operator+= (float rhs)
	{
		packFloat(unpackFloat() + rhs);
		return *this;
	}
	HalfFloat& operator-= (float rhs)
	{
		packFloat(unpackFloat() - rhs);
		return *this;
	}

	HalfFloat& operator/= (float rhs)
	{
		packFloat(unpackFloat() / rhs);
		return *this;
	}
	HalfFloat& operator*= (float rhs)
	{
		packFloat(unpackFloat() * rhs);
		return *this;
	}

	static float getMaximumAbsoluteValue() { return 65504.0f; }
	static float getMinimumAbsoluteValue() { return 0.0000000596f; }
	static float getLowestValue() { return - ((float)getMaximumAbsoluteValue()); }

private:
	struct HalfFloatBits
	{
		unsigned int mantissa: 10;
		unsigned int exponent: 5;
		unsigned int sign: 1;
	};

	struct FloatBits
	{
		unsigned int mantissa: 23;
		unsigned int exponent: 8;
		unsigned int sign: 1;
	};

	union FloatData
	{
		float      value;
		FloatBits bits;
	};

	union HalfFloatData
	{
		short    value;
		HalfFloatBits bits;
#if defined(__GNUC__)
	} __attribute__((packed));
#else
	};
#endif //__GNUC__

	short _value;

	static const int c_halfExponentBias = 15;
	static const int c_floatExponentBias = 127;

	static const int c_halfExponentNaN = 31;
	static const int c_floatExponentNaN = 255;

	void packFloat(const float& value)
	{
		HalfFloatData halfData = { _value };
		//Get the raw bits of the floating point value.
		const FloatData floatFields = { value };

		//Work out the new exponent.
		int newExponent = floatFields.bits.exponent - c_floatExponentBias + c_halfExponentBias;

		//Set a sort of "default state" for the half float value
		halfData.value = 0;
		halfData.bits.sign = floatFields.bits.sign;

		//Check for NaNs, Infinities, Overflows and Underflows.

		//Anything above or equal to halfExponentNaN is going to be either too large to represent or a NaN.
		if (newExponent >= c_halfExponentNaN)
		{
			//Any exponent greater than the representable range will return Infinity.
			halfData.bits.exponent = c_halfExponentNaN;

			//Ensure that any NaNs are still a NaN, and not accidentally turned into an infinity.
			if (floatFields.bits.exponent == c_floatExponentNaN && floatFields.bits.mantissa != 0)
			{
				halfData.bits.mantissa = 0x200;
			}
		}
		//Underflows may be representable with a loss of accuracy
		else if (newExponent <= 0)
		{
			//Attempt to deal with it if we can.
			if (newExponent >= -10)
			{
				//Calculate the new mantissa, masking off the top bit.
				unsigned int newMantissa = floatFields.bits.mantissa | 0x800000;
				halfData.bits.mantissa = newMantissa >> (14 - newExponent);

				//Check if it needs to be rounded up
				if ((newMantissa >> (13 - newExponent)) & 1)
				{
					//It might overflow to the exponent bit, but this is fine due to the way floats work.
					halfData.value++;
				}
			}
		}
		else
		{
			//Simple case, just set the exponent and mantissa.
			halfData.bits.exponent = newExponent;
			halfData.bits.mantissa = floatFields.bits.mantissa >> 13;

			//Check if it needs to be rounded up
			if (floatFields.bits.mantissa & 0x1000)
			{
				//It might overflow to the exponent bit, but this is fine due to the way floats work.
				halfData.value++;
			}
		}
		_value = halfData.value;
	}

	float unpackFloat() const
	{
		//Get the raw bits of the half float, and turn them into a float.
		FloatData floatFields;
		const HalfFloatData halfData = { _value };
		//Set a default value for the float
		floatFields.bits.sign = halfData.bits.sign;
		floatFields.bits.exponent = 0;
		floatFields.bits.mantissa = 0;

		// Check for anything that isn't signed zero
		if (halfData.bits.exponent != 0 || halfData.bits.mantissa != 0)
		{
			//Some de-normalised half floats can be normalised - due to the increased mantissa of a full float.
			//This is a kind of slow operation, but means that the floating point values can be easier to work on in hardware and thus faster.
			if (halfData.bits.exponent == 0)
			{
				// Adjust the exponent and mantissa to try to normalise the new value.
				unsigned int exponentAdjustment = 0;
				unsigned int newMantissa = halfData.bits.mantissa << 1;

				//Loop through until the mantissa hits its maximum
				while ((newMantissa & 0x400) == 0)
				{
					exponentAdjustment++;
					newMantissa <<= 1;
				}

				//Set the new mantissa and exponent values
				floatFields.bits.mantissa = (newMantissa & 0x3ff) << 13;
				floatFields.bits.exponent = (c_floatExponentBias - c_halfExponentBias) - exponentAdjustment;
			}
			//Handle infinities and NaNs
			else if (halfData.bits.exponent == c_halfExponentNaN)
			{
				//Expand the mantissa up to the original size, and set the NaN value for the exponent.
				floatFields.bits.exponent = c_floatExponentNaN;
				floatFields.bits.mantissa = halfData.bits.mantissa << 13;
			}
			//Otherwise it's a simple normalised number
			else
			{
				floatFields.bits.exponent = halfData.bits.exponent + (c_floatExponentBias - c_halfExponentBias);
				floatFields.bits.mantissa = halfData.bits.mantissa << 13;
			}
		}

		return floatFields.value;
	}

#if defined(_WIN32)
};
#pragma pack(pop)
#elif defined(__GNUC__)
} __attribute__((packed));
#else
};
#endif //_WIN32
}