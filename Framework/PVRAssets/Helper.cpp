#include "PVRAssets/Helper.h"
namespace pvr {
namespace assets {
namespace helper {
void VertexRead(const uint8_t* data, const DataType type, uint32_t count, float* out)
{
	uint32_t	i;

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

	case DataType::Float32:
		for (i = 0; i < count; ++i)
		{
			out[i] = reinterpret_cast<const float*>(data)[i];
		}
		break;

	case DataType::Fixed16_16:
		for (i = 0; i < count; ++i)
		{
			out[i] = reinterpret_cast<const int32_t*>(data)[i] * 1.0f / static_cast<float>(1 << 16);
		}
		break;

	case DataType::Int32:
		for (i = 0; i < count; ++i)
		{
			out[i] = static_cast<float>(reinterpret_cast<const int32_t*>(data)[i]);
		}
		break;

	case DataType::UInt32:
		for (i = 0; i < count; ++i)
		{
			out[i] = static_cast<float>(reinterpret_cast<const uint32_t*>(data)[i]);
		}
		break;

	case DataType::Int8:
		for (i = 0; i < count; ++i)
		{
			out[i] = static_cast<float>(reinterpret_cast<const char*>(data)[i]);
		}
		break;

	case DataType::Int8Norm:
		for (i = 0; i < count; ++i)
		{
			out[i] = static_cast<float>(reinterpret_cast<const char*>(data)[i]) / static_cast<float>((1 << 7) - 1);
		}
		break;

	case DataType::UInt8:
		for (i = 0; i < count; ++i)
		{
			out[i] = static_cast<float>(reinterpret_cast<const char*>(data)[i]);
		}
		break;

	case DataType::UInt8Norm:
		for (i = 0; i < count; ++i)
		{
			out[i] = static_cast<float>(reinterpret_cast<const char*>(data)[i]) / static_cast<float>((1 << 8) - 1);
		}
		break;

	case DataType::Int16:
		for (i = 0; i < count; ++i)
		{
			out[i] = static_cast<float>(reinterpret_cast<const int16_t*>(data)[i]);
		}
		break;

	case DataType::Int16Norm:
		for (i = 0; i < count; ++i)
		{
			out[i] = static_cast<float>(reinterpret_cast<const int16_t*>(data)[i]) / static_cast<float>((1 << 15) - 1);
		}
		break;

	case DataType::UInt16:
		for (i = 0; i < count; ++i)
		{
			out[i] = static_cast<float>(reinterpret_cast<const uint16_t*>(data)[i]);
		}
		break;

	case DataType::RGBA:
	{
		uint32_t dwVal = *reinterpret_cast<const uint32_t*>(data);
		char v[4];

		v[0] = static_cast<char>(dwVal >> 24);
		v[1] = static_cast<char>(dwVal >> 16);
		v[2] = static_cast<char>(dwVal >> 8);
		v[3] = static_cast<char>(dwVal >> 0);

		for (i = 0; i < 4; ++i)
		{
			out[i] = 1.0f / 255.0f * static_cast<float>(v[i]);
		}
	}
	break;

	case DataType::ABGR:
	{
		uint32_t dwVal = *reinterpret_cast<const uint32_t*>(data);
		char v[4];

		v[0] = static_cast<char>(dwVal >> 0);
		v[1] = static_cast<char>(dwVal >> 8);
		v[2] = static_cast<char>(dwVal >> 16);
		v[3] = static_cast<char>(dwVal >> 24);

		for (i = 0; i < 4; ++i)
		{
			out[i] = 1.0f / 255.0f * static_cast<float>(v[i]);
		}
	}
	break;

	case DataType::ARGB:
	case DataType::D3DCOLOR:
	{
		uint32_t dwVal = *reinterpret_cast<const uint32_t*>(data);
		char v[4];

		v[0] = static_cast<char>(dwVal >> 16);
		v[1] = static_cast<char>(dwVal >> 8);
		v[2] = static_cast<char>(dwVal >> 0);
		v[3] = static_cast<char>(dwVal >> 24);

		for (i = 0; i < 4; ++i)
		{
			out[i] = 1.0f / 255.0f * static_cast<float>(v[i]);
		}
	}
	break;

	case DataType::UBYTE4:
	{
		uint32_t dwVal = *reinterpret_cast<const uint32_t*>(data);
		char v[4];

		v[0] = static_cast<char>(dwVal >> 0);
		v[1] = static_cast<char>(dwVal >> 8);
		v[2] = static_cast<char>(dwVal >> 16);
		v[3] = static_cast<char>(dwVal >> 24);

		for (i = 0; i < 4; ++i)
		{
			out[i] = v[i];
		}
	}
	break;

	case DataType::DEC3N:
	{
		int32_t dwVal = *reinterpret_cast<const int32_t*>(data);
		int32_t v[4];

		v[0] = (dwVal << 22) >> 22;
		v[1] = (dwVal << 12) >> 22;
		v[2] = (dwVal << 2) >> 22;
		v[3] = 0;

		for (i = 0; i < 3; ++i)
		{
			out[i] = static_cast<float>(v[i]) * (1.0f / 511.0f);
		}
	}
	break;
	}
}

void VertexIndexRead(const uint8_t* data, const IndexType type, uint32_t* const out)
{
	switch (type)
	{
	default:
		assertion(false);
		break;

	case IndexType::IndexType16Bit:
		*out = *reinterpret_cast<const uint16_t*>(data);
		break;

	case IndexType::IndexType32Bit:
		*out = *reinterpret_cast<const uint32_t*>(data);
		break;
	}
}

bool loadModel(IAssetProvider& assetProvider, const char* filename, assets::ModelHandle& outModel)
{
	Stream::ptr_type assetStream = assetProvider.getAssetStream(filename);
	if (!assetStream.get())
	{
		Log(LogLevel::Error, "Error for filename %s : File not found", filename);
		return false;
	}

	assets::PODReader reader(std::move(assetStream));
	outModel = assets::Model::createWithReader(reader);

	// Load the scene
	if (outModel.isNull())
	{
		Log(LogLevel::Error, "Could not load the file: %s", filename);
		return false;
	}
	return true;
}
}// namespace helper
}// namespace assets
}// namespace pvr