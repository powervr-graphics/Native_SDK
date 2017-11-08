/*!
\brief Contains a function to write TGA data to a file
\file PVRCore/TGAWriter.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/CoreIncludes.h"
#include "PVRCore/IO/FileStream.h"

namespace pvr {
/// <summary>Write out TGA data from an image.</summary>
/// <param name="filename">C-style string with the filename to write the TGA.</param>
/// <param name="w">The width of the image</param>
/// <param name="h">The height of the image</param>
/// <param name="imageData">Pointer to the raw image data</param>
/// <param name="stride">Size in bytes of each pixel</param>
/// <param name="pixelReplicate">Upscale factor.</param>
/// <returns>True if successfuly completed, otherwise False. (log error on false)</returns>
inline bool writeTGA(const char* const filename, unsigned int w, unsigned int h, const unsigned char* const imageData, const unsigned char stride, unsigned int pixelReplicate = 1)
{
	if (pixelReplicate == 0 || w == 0 || h == 0)
	{
		Log("writeTGA: Invalid size.");
		return false;
	}

	if (!imageData)
	{
		Log("writeTGA: Pointer to data was null");
		return false;
	}

	// header
	unsigned char lengthID(0);
	unsigned char colorMapType(0);
	unsigned char imageType(2);

	// header - color map specification (5 bytes)
	unsigned short  firstEntryIndex(0);
	unsigned short	colorMapLength(0);
	unsigned char	colorMapEntrySize(0);

	// header - image specification (10 bytes)
	unsigned short originX(0);
	unsigned short originY(0);
	unsigned short imageWidth(static_cast<unsigned short>(w * pixelReplicate));
	unsigned short imageHeight(static_cast<unsigned short>(h * pixelReplicate));
	unsigned char  bitsperpixel(static_cast<unsigned char>(stride * 8));
	unsigned char  imageDescriptor(0);

	FileStream file(filename, "wb");

	if (!file.open())
	{
		Log("WriteTGA: Could not create file.");
		return false;
	}

	// Write header
	size_t dataWritten;
	file.write(sizeof(lengthID), 1, &lengthID, dataWritten);
	file.write(sizeof(colorMapType), 1, &colorMapType, dataWritten);
	file.write(sizeof(imageType), 1, &imageType, dataWritten);
	file.write(sizeof(firstEntryIndex), 1, &firstEntryIndex, dataWritten);
	file.write(sizeof(colorMapLength), 1, &colorMapLength, dataWritten);
	file.write(sizeof(colorMapEntrySize), 1, &colorMapEntrySize, dataWritten);
	file.write(sizeof(originX), 1, &originX, dataWritten);
	file.write(sizeof(originY), 1, &originY, dataWritten);
	file.write(sizeof(imageWidth), 1, &imageWidth, dataWritten);
	file.write(sizeof(imageHeight), 1, &imageHeight, dataWritten);
	file.write(sizeof(bitsperpixel), 1, &bitsperpixel, dataWritten);
	file.write(sizeof(imageDescriptor), 1, &imageDescriptor, dataWritten);

	// write out the data
	if (pixelReplicate == 1)
	{
		file.write(w * h * stride, 1, imageData, dataWritten);
	}
	else
	{
		for (unsigned int y = 0; y < h; ++y)
		{
			const unsigned char* row = &imageData[stride * w * y];

			for (unsigned int repY = 0; repY < pixelReplicate; ++repY)
			{
				for (unsigned int x = 0; x < w; ++x)
				{
					const unsigned char* pixel = &row[stride * x];

					for (unsigned int repX = 0; repX < pixelReplicate; ++repX)
					{
						file.write(stride, 1, pixel, dataWritten);
					}
				}
			}
		}
	}
	file.close();
	return true;
}
}