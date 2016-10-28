/*!*********************************************************************************************************************
\file         PVRAssets/Helper.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief Internal helper classes
***********************************************************************************************************************/
#pragma once

#include "PVRAssets/Model/Mesh.h"
namespace pvr {
/*!*********************************************************************************************************************
\brief Read vertex data into float32 buffer.
\param[in] data Data to read from
\param[in] type Data type of the vertex to read 
\param[in] count Number of vertices to read
\param[out] out Array of vertex read
***********************************************************************************************************************/
inline void VertexRead(const byte* const data, const types::DataType type, uint32 count, float32* out)
{
	uint32	i;

	out[0] = 0;

	if (count > 1)
	{
		out[1] = 0;
	}

	if (count > 2)
	{
		out[2] = 0;
	}

	if (count > 3)
	{
		out[3] = 1;
	}

	switch (type)
	{
	default:
		assertion(false);
		break;

    case types::DataType::Float32:
		for (i = 0; i < count; ++i)
		{
			out[i] = ((float32*)data)[i];
		}
		break;

    case types::DataType::Fixed16_16:
		for (i = 0; i < count; ++i)
		{
			out[i] = ((int32*)data)[i] * 1.0f / (float32)(1 << 16);
		}
		break;

    case types::DataType::Int32:
		for (i = 0; i < count; ++i)
		{
			out[i] = (float32)((int32*)data)[i];
		}
		break;

    case types::DataType::UInt32:
		for (i = 0; i < count; ++i)
		{
			out[i] = (float32)((uint32*)data)[i];
		}
		break;

    case types::DataType::Int8:
		for (i = 0; i < count; ++i)
		{
			out[i] = (float32)((char8*)data)[i];
		}
		break;

    case types::DataType::Int8Norm:
		for (i = 0; i < count; ++i)
		{
			out[i] = (float32)((char8*)data)[i] / (float32)((1 << 7) - 1);
		}
		break;

    case types::DataType::UInt8:
		for (i = 0; i < count; ++i)
		{
			out[i] = (float32)((byte*)data)[i];
		}
		break;

    case types::DataType::UInt8Norm:
		for (i = 0; i < count; ++i)
		{
			out[i] = (float32)((byte*)data)[i] / (float32)((1 << 8) - 1);
		}
		break;

    case types::DataType::Int16:
		for (i = 0; i < count; ++i)
		{
			out[i] = (float32)((int16*)data)[i];
		}
		break;

    case types::DataType::Int16Norm:
		for (i = 0; i < count; ++i)
		{
			out[i] = (float32)((int16*)data)[i] / (float32)((1 << 15) - 1);
		}
		break;

    case types::DataType::UInt16:
		for (i = 0; i < count; ++i)
		{
			out[i] = (float32)((uint16*)data)[i];
		}
		break;

	//case PVRDataType::Int16Norm:
	//	for(i = 0; i < count; ++i)
	//		out[i] = (float32)((uint16*)data)[i] / (float32)((1 << 16)-1);
	//	break;

    case types::DataType::RGBA:
	{
		uint32 dwVal = *(uint32*)data;
		byte v[4];

		v[0] = (byte)(dwVal >> 24);
		v[1] = (byte)(dwVal >> 16);
		v[2] = (byte)(dwVal >>  8);
		v[3] = (byte)(dwVal >>  0);

		for (i = 0; i < 4; ++i)
		{
			out[i] = 1.0f / 255.0f * (float32)v[i];
		}
	}
	break;

    case types::DataType::ABGR:
	{
		uint32 dwVal = *(uint32*)data;
		byte v[4];

		v[0] = (byte)(dwVal >> 0);
		v[1] = (byte)(dwVal >> 8);
		v[2] = (byte)(dwVal >> 16);
		v[3] = (byte)(dwVal >> 24);

		for (i = 0; i < 4; ++i)
		{
			out[i] = 1.0f / 255.0f * (float32)v[i];
		}
	}
	break;

    case types::DataType::ARGB:
    case types::DataType::D3DCOLOR:
	{
		uint32 dwVal = *(uint32*)data;
		byte v[4];

		v[0] = (byte)(dwVal >> 16);
		v[1] = (byte)(dwVal >>  8);
		v[2] = (byte)(dwVal >>  0);
		v[3] = (byte)(dwVal >> 24);

		for (i = 0; i < 4; ++i)
		{
			out[i] = 1.0f / 255.0f * (float32)v[i];
		}
	}
	break;

    case types::DataType::UBYTE4:
	{
		uint32 dwVal = *(uint32*)data;
		byte v[4];

		v[0] = (byte)(dwVal >>  0);
		v[1] = (byte)(dwVal >>  8);
		v[2] = (byte)(dwVal >> 16);
		v[3] = (byte)(dwVal >> 24);

		for (i = 0; i < 4; ++i)
		{
			out[i] = v[i];
		}
	}
	break;

    case types::DataType::DEC3N:
	{
		int32 dwVal = *(int32*)data;
		int32 v[4];

		v[0] = (dwVal << 22) >> 22;
		v[1] = (dwVal << 12) >> 22;
		v[2] = (dwVal <<  2) >> 22;
		v[3] = 0;

		for (i = 0; i < 3; ++i)
		{
			out[i] = (float32)v[i] * (1.0f / 511.0f);
		}
	}
	break;
	}
}

/*!*********************************************************************************************************************
\brief Read vertex index data into uin32 buffer.
\param[in] data Data to read from
\param[in] type Index type to read
\param[out] out of index data read
***********************************************************************************************************************/
inline void VertexIndexRead(const byte* const data, const types::IndexType type, uint32* const out)
{
	switch (type)
	{
	default:
		assertion(false);
		break;

    case types::IndexType::IndexType16Bit:
		*out = *(uint16*)data;
		break;

    case types::IndexType::IndexType32Bit:
		*out = *(uint32*)data;
		break;
	}
}
}
