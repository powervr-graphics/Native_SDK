/*!
\brief Implementation of the Texture Decompression functions.
\file PVRCore/Texture/PVRTDecompress.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN

#include <cstdlib>
#include <cstdio>
#include <climits>
#include <cmath>
#include <algorithm>
#include <cstring>
#include "PVRTDecompress.h"
#include "PVRCore/Texture.h"
namespace pvr {
enum
{
	ETC_MIN_TEXWIDTH = 4,
	ETC_MIN_TEXHEIGHT = 4,
	DXT_MIN_TEXWIDTH = 4,
	DXT_MIN_TEXHEIGHT = 4,
};

struct Pixel32
{
	uint8 red, green, blue, alpha;
};

struct Pixel128S
{
	int32 red, green, blue, alpha;
};

struct PVRTCWord
{
	uint32 u32ModulationData;
	uint32 u32ColorData;
};

struct PVRTCWordIndices
{
	int P[2], Q[2], R[2], S[2];
};

static Pixel32 getColorA(uint32 u32ColorData)
{
	Pixel32 color;

	// Opaque Color Mode - RGB 554
	if ((u32ColorData & 0x8000) != 0)
	{
		color.red   = (uint8)((u32ColorData & 0x7c00) >> 10); // 5->5 bits
		color.green = (uint8)((u32ColorData & 0x3e0)  >> 5); // 5->5 bits
		color.blue  = (uint8)(u32ColorData  & 0x1e) | ((u32ColorData & 0x1e) >> 4); // 4->5 bits
		color.alpha = (uint8)0xf;// 0->4 bits
	}
	// Transparent Color Mode - ARGB 3443
	else
	{
		color.red   = (uint8)((u32ColorData & 0xf00)  >> 7) | ((u32ColorData & 0xf00) >> 11); // 4->5 bits
		color.green = (uint8)((u32ColorData & 0xf0)   >> 3) | ((u32ColorData & 0xf0)  >> 7); // 4->5 bits
		color.blue  = (uint8)((u32ColorData & 0xe)    << 1) | ((u32ColorData & 0xe)   >> 2); // 3->5 bits
		color.alpha = (uint8)((u32ColorData & 0x7000) >> 11);// 3->4 bits - note 0 at right
	}

	return color;
}

static Pixel32 getColorB(uint32 u32ColorData)
{
	Pixel32 color;

	// Opaque Color Mode - RGB 555
	if (u32ColorData & 0x80000000)
	{
		color.red   = (uint8)((u32ColorData & 0x7c000000) >> 26); // 5->5 bits
		color.green = (uint8)((u32ColorData & 0x3e00000)  >> 21); // 5->5 bits
		color.blue  = (uint8)((u32ColorData & 0x1f0000)   >> 16); // 5->5 bits
		color.alpha = (uint8)0xf;// 0 bits
	}
	// Transparent Color Mode - ARGB 3444
	else
	{
		color.red   = (uint8)(((u32ColorData & 0xf000000)  >> 23) | ((u32ColorData & 0xf000000) >> 27)); // 4->5 bits
		color.green = (uint8)(((u32ColorData & 0xf00000)   >> 19) | ((u32ColorData & 0xf00000)  >> 23)); // 4->5 bits
		color.blue  = (uint8)(((u32ColorData & 0xf0000)    >> 15) | ((u32ColorData & 0xf0000)   >> 19)); // 4->5 bits
		color.alpha = (uint8)((u32ColorData & 0x70000000) >> 27);// 3->4 bits - note 0 at right
	}

	return color;
}

static void interpolateColors(Pixel32 P, Pixel32 Q, Pixel32 R, Pixel32 S,
                              Pixel128S* pPixel, uint8 ui8Bpp)
{
	uint32 ui32WordWidth = 4;
	uint32 ui32WordHeight = 4;
	if (ui8Bpp == 2)
	{
		ui32WordWidth = 8;
	}

	//Convert to int 32.
	Pixel128S hP = {(int32)P.red, (int32)P.green, (int32)P.blue, (int32)P.alpha};
	Pixel128S hQ = {(int32)Q.red, (int32)Q.green, (int32)Q.blue, (int32)Q.alpha};
	Pixel128S hR = {(int32)R.red, (int32)R.green, (int32)R.blue, (int32)R.alpha};
	Pixel128S hS = {(int32)S.red, (int32)S.green, (int32)S.blue, (int32)S.alpha};

	//Get vectors.
	Pixel128S QminusP = {hQ.red - hP.red, hQ.green - hP.green, hQ.blue - hP.blue, hQ.alpha - hP.alpha};
	Pixel128S SminusR = {hS.red - hR.red, hS.green - hR.green, hS.blue - hR.blue, hS.alpha - hR.alpha};

	//Multiply colors.
	hP.red    *=  ui32WordWidth;
	hP.green  *=  ui32WordWidth;
	hP.blue   *=  ui32WordWidth;
	hP.alpha  *=  ui32WordWidth;
	hR.red    *=  ui32WordWidth;
	hR.green  *=  ui32WordWidth;
	hR.blue   *=  ui32WordWidth;
	hR.alpha  *=  ui32WordWidth;

	if (ui8Bpp == 2)
	{
		//Loop through pixels to achieve results.
		for (unsigned int x = 0; x < ui32WordWidth; x++)
		{
			Pixel128S result = {4 * hP.red, 4 * hP.green, 4 * hP.blue, 4 * hP.alpha};
			Pixel128S dY = {hR.red - hP.red, hR.green - hP.green, hR.blue - hP.blue, hR.alpha - hP.alpha};

			for (unsigned int y = 0; y < ui32WordHeight; y++)
			{
				pPixel[y * ui32WordWidth + x].red = (int32)((result.red >> 7) + (result.red >> 2));
				pPixel[y * ui32WordWidth + x].green = (int32)((result.green >> 7) + (result.green >> 2));
				pPixel[y * ui32WordWidth + x].blue = (int32)((result.blue >> 7) + (result.blue >> 2));
				pPixel[y * ui32WordWidth + x].alpha = (int32)((result.alpha >> 5) + (result.alpha >> 1));

				result.red += dY.red;
				result.green += dY.green;
				result.blue += dY.blue;
				result.alpha += dY.alpha;
			}

			hP.red    += QminusP.red;
			hP.green  += QminusP.green;
			hP.blue   += QminusP.blue;
			hP.alpha  += QminusP.alpha;

			hR.red    += SminusR.red;
			hR.green  += SminusR.green;
			hR.blue   += SminusR.blue;
			hR.alpha  += SminusR.alpha;
		}
	}
	else
	{
		//Loop through pixels to achieve results.
		for (unsigned int y = 0; y < ui32WordHeight; y++)
		{
			Pixel128S result = {4 * hP.red, 4 * hP.green, 4 * hP.blue, 4 * hP.alpha};
			Pixel128S dY = {hR.red - hP.red, hR.green - hP.green, hR.blue - hP.blue, hR.alpha - hP.alpha};

			for (unsigned int x = 0; x < ui32WordWidth; x++)
			{
				pPixel[y * ui32WordWidth + x].red   = (int32)((result.red   >> 6) + (result.red   >> 1));
				pPixel[y * ui32WordWidth + x].green = (int32)((result.green >> 6) + (result.green >> 1));
				pPixel[y * ui32WordWidth + x].blue  = (int32)((result.blue  >> 6) + (result.blue  >> 1));
				pPixel[y * ui32WordWidth + x].alpha = (int32)((result.alpha >> 4) + (result.alpha));

				result.red += dY.red;
				result.green += dY.green;
				result.blue += dY.blue;
				result.alpha += dY.alpha;
			}

			hP.red += QminusP.red;
			hP.green += QminusP.green;
			hP.blue += QminusP.blue;
			hP.alpha += QminusP.alpha;

			hR.red += SminusR.red;
			hR.green += SminusR.green;
			hR.blue += SminusR.blue;
			hR.alpha += SminusR.alpha;
		}
	}
}

static void unpackModulations(const PVRTCWord& word, int offsetX, int offsetY, int32 i32ModulationValues[16][8],
                              int32 i32ModulationModes[16][8], uint8 ui8Bpp)
{
	uint32 WordModMode = word.u32ColorData & 0x1;
	uint32 ModulationBits = word.u32ModulationData;

	// Unpack differently depending on 2bpp or 4bpp modes.
	if (ui8Bpp == 2)
	{
		if (WordModMode)
		{
			// determine which of the three modes are in use:

			// If this is the either the H-only or V-only interpolation mode...
			if (ModulationBits & 0x1)
			{
				// look at the "LSB" for the "centre" (V=2,H=4) texel. Its LSB is now
				// actually used to indicate whether it's the H-only mode or the V-only...

				// The centre texel data is the at (y==2, x==4) and so its LSB is at bit 20.
				if (ModulationBits & (0x1 << 20))
				{
					// This is the V-only mode
					WordModMode = 3;
				}
				else
				{
					// This is the H-only mode
					WordModMode = 2;
				}

				// Create an extra bit for the centre pixel so that it looks like
				// we have 2 actual bits for this texel. It makes later coding much easier.
				if (ModulationBits & (0x1 << 21))
				{
					// set it to produce code for 1.0
					ModulationBits |= (0x1 << 20);
				}
				else
				{
					// clear it to produce 0.0 code
					ModulationBits &= ~(0x1 << 20);
				}
			}// end if H-Only or V-Only interpolation mode was chosen

			if (ModulationBits & 0x2)
			{
				ModulationBits |= 0x1; /*set it*/
			}
			else
			{
				ModulationBits &= ~0x1; /*clear it*/
			}

			// run through all the pixels in the block. Note we can now treat all the
			// "stored" values as if they have 2bits (even when they didn't!)
			for (int y = 0; y < 4; y++)
			{
				for (int x = 0; x < 8; x++)
				{
					i32ModulationModes[x + offsetX][y + offsetY] = WordModMode;

					// if this is a stored value...
					if (((x ^ y) & 1) == 0)
					{
						i32ModulationValues[x + offsetX][y + offsetY] = ModulationBits & 3;
						ModulationBits >>= 2;
					}
				}
			} // end for y
		}
		// else if direct encoded 2bit mode - i.e. 1 mode bit per pixel
		else
		{
			for (int y = 0; y < 4; y++)
			{
				for (int x = 0; x < 8; x++)
				{
					i32ModulationModes[x + offsetX][y + offsetY] = WordModMode;

					/*
					// double the bits so 0=> 00, and 1=>11
					*/
					if (ModulationBits & 1)
					{
						i32ModulationValues[x + offsetX][y + offsetY] = 0x3;
					}
					else
					{
						i32ModulationValues[x + offsetX][y + offsetY] = 0x0;
					}
					ModulationBits >>= 1;
				}
			}// end for y
		}
	}
	else
	{
		//Much simpler than the 2bpp decompression, only two modes, so the n/8 values are set directly.
		// run through all the pixels in the word.
		if (WordModMode)
		{
			for (int y = 0; y < 4; y++)
			{
				for (int x = 0; x < 4; x++)
				{
					i32ModulationValues[y + offsetY][x + offsetX] = ModulationBits & 3;
					//if (i32ModulationValues==0) {}; don't need to check 0, 0 = 0/8.
					if (i32ModulationValues[y + offsetY][x + offsetX] == 1)
					{
						i32ModulationValues[y + offsetY][x + offsetX] = 4;
					}
					else if (i32ModulationValues[y + offsetY][x + offsetX] == 2)
					{
						i32ModulationValues[y + offsetY][x + offsetX] = 14; //+10 tells the decompressor to punch through alpha.
					}
					else if (i32ModulationValues[y + offsetY][x + offsetX] == 3)
					{
						i32ModulationValues[y + offsetY][x + offsetX] = 8;
					}
					ModulationBits >>= 2;
				} // end for x
			} // end for y
		}
		else
		{
			for (int y = 0; y < 4; y++)
			{
				for (int x = 0; x < 4; x++)
				{
					i32ModulationValues[y + offsetY][x + offsetX] = ModulationBits & 3;
					i32ModulationValues[y + offsetY][x + offsetX] *= 3;
					if (i32ModulationValues[y + offsetY][x + offsetX] > 3) { i32ModulationValues[y + offsetY][x + offsetX] -= 1; }
					ModulationBits >>= 2;
				} // end for x
			} // end for y
		}
	}
}

static int32 getModulationValues(int32 i32ModulationValues[16][8], int32 i32ModulationModes[16][8], uint32 xPos, uint32 yPos,
                                 uint8 ui8Bpp)
{
	if (ui8Bpp == 2)
	{
		const int RepVals0[4] = {0, 3, 5, 8};

		// extract the modulation value. If a simple encoding
		if (i32ModulationModes[xPos][yPos] == 0)
		{
			return RepVals0[i32ModulationValues[xPos][yPos]];
		}
		else
		{
			// if this is a stored value
			if (((xPos ^ yPos) & 1) == 0)
			{
				return RepVals0[i32ModulationValues[xPos][yPos]];
			}

			// else average from the neighbours
			// if H&V interpolation...
			else if (i32ModulationModes[xPos][yPos] == 1)
			{
				return (RepVals0[i32ModulationValues[xPos][yPos - 1]] +
				        RepVals0[i32ModulationValues[xPos][yPos + 1]] +
				        RepVals0[i32ModulationValues[xPos - 1][yPos]] +
				        RepVals0[i32ModulationValues[xPos + 1][yPos]] + 2) / 4;
			}
			// else if H-Only
			else if (i32ModulationModes[xPos][yPos] == 2)
			{
				return (RepVals0[i32ModulationValues[xPos - 1][yPos]] +
				        RepVals0[i32ModulationValues[xPos + 1][yPos]] + 1) / 2;
			}
			// else it's V-Only
			else
			{
				return (RepVals0[i32ModulationValues[xPos][yPos - 1]] +
				        RepVals0[i32ModulationValues[xPos][yPos + 1]] + 1) / 2;
			}
		}
	}
	else if (ui8Bpp == 4)
	{
		return i32ModulationValues[xPos][yPos];
	}

	return 0;
}

static void pvrtcGetDecompressedPixels(const PVRTCWord& P, const PVRTCWord& Q,
                                       const PVRTCWord& R, const PVRTCWord& S,
                                       Pixel32* pColorData,
                                       uint8 ui8Bpp)
{
	//4bpp only needs 8*8 values, but 2bpp needs 16*8, so rather than wasting processor time we just statically allocate 16*8.
	int32 i32ModulationValues[16][8];
	//Only 2bpp needs this.
	int32 i32ModulationModes[16][8];
	//4bpp only needs 16 values, but 2bpp needs 32, so rather than wasting processor time we just statically allocate 32.
	Pixel128S upscaledColorA[32];
	Pixel128S upscaledColorB[32];

	uint32 ui32WordWidth = 4;
	uint32 ui32WordHeight = 4;
	if (ui8Bpp == 2)
	{
		ui32WordWidth = 8;
	}

	//Get the modulations from each word.
	unpackModulations(P, 0, 0, i32ModulationValues, i32ModulationModes, ui8Bpp);
	unpackModulations(Q, ui32WordWidth, 0, i32ModulationValues, i32ModulationModes, ui8Bpp);
	unpackModulations(R, 0, ui32WordHeight, i32ModulationValues, i32ModulationModes, ui8Bpp);
	unpackModulations(S, ui32WordWidth, ui32WordHeight, i32ModulationValues, i32ModulationModes, ui8Bpp);

	// Bilinear upscale image data from 2x2 -> 4x4
	interpolateColors(getColorA(P.u32ColorData), getColorA(Q.u32ColorData),
	                  getColorA(R.u32ColorData), getColorA(S.u32ColorData),
	                  upscaledColorA, ui8Bpp);
	interpolateColors(getColorB(P.u32ColorData), getColorB(Q.u32ColorData),
	                  getColorB(R.u32ColorData), getColorB(S.u32ColorData),
	                  upscaledColorB, ui8Bpp);

	for (unsigned int y = 0; y < ui32WordHeight; y++)
	{
		for (unsigned int x = 0; x < ui32WordWidth; x++)
		{
			int32 mod = getModulationValues(i32ModulationValues, i32ModulationModes, x + ui32WordWidth / 2, y + ui32WordHeight / 2, ui8Bpp);
			bool punchthroughAlpha = false;
			if (mod > 10)
			{
				punchthroughAlpha = true;
				mod -= 10;
			}

			Pixel128S result;
			result.red   = (upscaledColorA[y * ui32WordWidth + x].red * (8 - mod) + upscaledColorB[y * ui32WordWidth + x].red * mod) / 8;
			result.green = (upscaledColorA[y * ui32WordWidth + x].green * (8 - mod) + upscaledColorB[y * ui32WordWidth + x].green * mod) /
			               8;
			result.blue  = (upscaledColorA[y * ui32WordWidth + x].blue * (8 - mod) + upscaledColorB[y * ui32WordWidth + x].blue * mod) / 8;
			if (punchthroughAlpha) { result.alpha = 0; }
			else { result.alpha = (upscaledColorA[y * ui32WordWidth + x].alpha * (8 - mod) + upscaledColorB[y * ui32WordWidth + x].alpha * mod) / 8; }

			//Convert the 32bit precision Result to 8 bit per channel color.
			if (ui8Bpp == 2)
			{
				pColorData[y * ui32WordWidth + x].red = (uint8)result.red;
				pColorData[y * ui32WordWidth + x].green = (uint8)result.green;
				pColorData[y * ui32WordWidth + x].blue = (uint8)result.blue;
				pColorData[y * ui32WordWidth + x].alpha = (uint8)result.alpha;
			}
			else if (ui8Bpp == 4)
			{
				pColorData[y + x * ui32WordHeight].red = (uint8)result.red;
				pColorData[y + x * ui32WordHeight].green = (uint8)result.green;
				pColorData[y + x * ui32WordHeight].blue = (uint8)result.blue;
				pColorData[y + x * ui32WordHeight].alpha = (uint8)result.alpha;
			}
		}
	}
}

static unsigned int wrapWordIndex(unsigned int numWords, int word)
{
	return ((word + numWords) % numWords);
}

/// <summary>Check that a number is an integer power of two, i.e.: 1, 2, 4, 8, ... etc.</summary>
/// <param name="input">Value to be checked</param>
/// <returns>true if the number is a non-zero, integer power of two, else false.</returns>
static bool isPowerOf2(unsigned int input)
{
	unsigned int minus1;

	if (!input) { return 0; }

	minus1 = input - 1;
	return ((input | minus1) == (input ^ minus1));
}

static uint32 TwiddleUV(uint32 XSize, uint32 YSize, uint32 XPos, uint32 YPos)
{
	//Initially assume X is the larger size.
	uint32 MinDimension = XSize;
	uint32 MaxValue = YPos;
	uint32 Twiddled = 0;
	uint32 SrcBitPos = 1;
	uint32 DstBitPos = 1;
	int ShiftCount = 0;

	//Check the sizes are valid.
	assertion(YPos < YSize);
	assertion(XPos < XSize);
	assertion(isPowerOf2(YSize));
	assertion(isPowerOf2(XSize));

	//If Y is the larger dimension - switch the min/max values.
	if (YSize < XSize)
	{
		MinDimension = YSize;
		MaxValue   = XPos;
	}

	// Step through all the bits in the "minimum" dimension
	while (SrcBitPos < MinDimension)
	{
		if (YPos & SrcBitPos)
		{
			Twiddled |= DstBitPos;
		}

		if (XPos & SrcBitPos)
		{
			Twiddled |= (DstBitPos << 1);
		}

		SrcBitPos <<= 1;
		DstBitPos <<= 2;
		ShiftCount += 1;
	}

	// Prepend any unused bits
	MaxValue >>= ShiftCount;
	Twiddled |= (MaxValue << (2 * ShiftCount));

	return Twiddled;
}

static void mapDecompressedData(Pixel32* pOutput, int width,
                                const Pixel32* pWord,
                                const PVRTCWordIndices& words,
                                uint8 ui8Bpp)
{
	uint32 ui32WordWidth = 4;
	uint32 ui32WordHeight = 4;
	if (ui8Bpp == 2)
	{
		ui32WordWidth = 8;
	}

	for (unsigned int y = 0; y < ui32WordHeight / 2; y++)
	{
		for (unsigned int x = 0; x < ui32WordWidth / 2; x++)
		{
			pOutput[(((words.P[1] * ui32WordHeight) + y + ui32WordHeight / 2)
			         * width + words.P[0] *ui32WordWidth + x + ui32WordWidth / 2)]  = pWord[y * ui32WordWidth + x];     // map P

			pOutput[(((words.Q[1] * ui32WordHeight) + y + ui32WordHeight / 2)
			         * width + words.Q[0] *ui32WordWidth + x)]          = pWord[y * ui32WordWidth + x + ui32WordWidth / 2];   // map Q

			pOutput[(((words.R[1] * ui32WordHeight) + y)
			         * width + words.R[0] *ui32WordWidth + x + ui32WordWidth / 2)]  = pWord[(y + ui32WordHeight / 2) * ui32WordWidth + x];    // map R

			pOutput[(((words.S[1] * ui32WordHeight) + y)
			         * width + words.S[0] *ui32WordWidth + x)]          = pWord[(y + ui32WordHeight / 2) * ui32WordWidth + x + ui32WordWidth / 2];  // map S
		}
	}
}
static int pvrtcDecompress(uint8* pCompressedData,
                           Pixel32* pDecompressedData,
                           uint32 ui32Width,
                           uint32 ui32Height,
                           uint8 ui8Bpp)
{
	uint32 ui32WordWidth = 4;
	uint32 ui32WordHeight = 4;
	if (ui8Bpp == 2)
	{
		ui32WordWidth = 8;
	}

	uint32* pWordMembers = (uint32*)pCompressedData;
	Pixel32* pOutData = pDecompressedData;

	// Calculate number of words
	int i32NumXWords = (int)(ui32Width / ui32WordWidth);
	int i32NumYWords = (int)(ui32Height / ui32WordHeight);

	// Structs used for decompression
	PVRTCWordIndices indices;
	Pixel32* pPixels;
	pPixels = (Pixel32*)malloc(ui32WordWidth * ui32WordHeight * sizeof(Pixel32));

	// For each row of words
	for (int wordY = -1; wordY < i32NumYWords - 1; wordY++)
	{
		// for each column of words
		for (int wordX = -1; wordX < i32NumXWords - 1; wordX++)
		{
			indices.P[0] = wrapWordIndex(i32NumXWords, wordX);
			indices.P[1] = wrapWordIndex(i32NumYWords, wordY);
			indices.Q[0] = wrapWordIndex(i32NumXWords, wordX + 1);
			indices.Q[1] = wrapWordIndex(i32NumYWords, wordY);
			indices.R[0] = wrapWordIndex(i32NumXWords, wordX);
			indices.R[1] = wrapWordIndex(i32NumYWords, wordY + 1);
			indices.S[0] = wrapWordIndex(i32NumXWords, wordX + 1);
			indices.S[1] = wrapWordIndex(i32NumYWords, wordY + 1);

			//Work out the offsets into the twiddle structs, multiply by two as there are two members per word.
			uint32 WordOffsets[4] =
			{
				TwiddleUV(i32NumXWords, i32NumYWords, indices.P[0], indices.P[1]) * 2,
				TwiddleUV(i32NumXWords, i32NumYWords, indices.Q[0], indices.Q[1]) * 2,
				TwiddleUV(i32NumXWords, i32NumYWords, indices.R[0], indices.R[1]) * 2,
				TwiddleUV(i32NumXWords, i32NumYWords, indices.S[0], indices.S[1]) * 2,
			};

			//Access individual elements to fill out PVRTCWord
			PVRTCWord P, Q, R, S;
			P.u32ColorData = pWordMembers[WordOffsets[0] + 1];
			P.u32ModulationData = pWordMembers[WordOffsets[0]];
			Q.u32ColorData = pWordMembers[WordOffsets[1] + 1];
			Q.u32ModulationData = pWordMembers[WordOffsets[1]];
			R.u32ColorData = pWordMembers[WordOffsets[2] + 1];
			R.u32ModulationData = pWordMembers[WordOffsets[2]];
			S.u32ColorData = pWordMembers[WordOffsets[3] + 1];
			S.u32ModulationData = pWordMembers[WordOffsets[3]];

			// assemble 4 words into struct to get decompressed pixels from
			pvrtcGetDecompressedPixels(P, Q, R, S, pPixels, ui8Bpp);
			mapDecompressedData(pOutData, ui32Width, pPixels, indices, ui8Bpp);

		} // for each word
	} // for each row of words

	free(pPixels);
	//Return the data size
	return ui32Width * ui32Height / (uint32)(ui32WordWidth / 2);
}

int PVRTDecompressPVRTC(const void* pCompressedData,
                        int Do2bitMode,
                        int XDim,
                        int YDim,
                        unsigned char* pResultImage)
{
	//Cast the output buffer to a Pixel32 pointer.
	Pixel32* pDecompressedData = (Pixel32*)pResultImage;

	//Check the X and Y values are at least the minimum size.
	int XTrueDim = std::max(XDim, ((Do2bitMode == 1) ? 16 : 8));
	int YTrueDim = std::max(YDim, 8);

	//If the dimensions aren't correct, we need to create a new buffer instead of just using the provided one, as the buffer will overrun otherwise.
	if (XTrueDim != XDim || YTrueDim != YDim)
	{
		pDecompressedData = (Pixel32*)malloc(XTrueDim * YTrueDim * sizeof(Pixel32));
	}

	//Decompress the surface.
	int retval = pvrtcDecompress((uint8*)pCompressedData, pDecompressedData, XTrueDim, YTrueDim, (Do2bitMode == 1 ? 2 : 4));

	//If the dimensions were too small, then copy the new buffer back into the output buffer.
	if (XTrueDim != XDim || YTrueDim != YDim)
	{
		//Loop through all the required pixels.
		for (int x = 0; x < XDim; ++x)
		{
			for (int y = 0; y < YDim; ++y)
			{
				((Pixel32*)pResultImage)[x + y * XDim] = pDecompressedData[x + y * XTrueDim];
			}
		}

		//Free the temporary buffer.
		free(pDecompressedData);
	}
	return retval;
}

////////////////////////////////////// ETC Compression //////////////////////////////////////

#define _CLAMP_(X,Xmin,Xmax) (  (X)<(Xmax) ?  (  (X)<(Xmin)?(Xmin):(X)  )  : (Xmax)    )

unsigned int ETC_FLIP =  0x01000000;
unsigned int ETC_DIFF = 0x02000000;
const int mod[8][4] = {{2, 8, -2, -8},
	{5, 17, -5, -17},
	{9, 29, -9, -29},
	{13, 42, -13, -42},
	{18, 60, -18, -60},
	{24, 80, -24, -80},
	{33, 106, -33, -106},
	{47, 183, -47, -183}
};

static unsigned int modifyPixel(int red, int green, int blue, int x, int y, unsigned int modBlock, int modTable)
{
	int index = x * 4 + y, pixelMod;
	unsigned int mostSig = modBlock << 1;

	if (index < 8)
	{
		pixelMod = mod[modTable][((modBlock >> (index + 24)) & 0x1) + ((mostSig >> (index + 8)) & 0x2)];
	}
	else
	{
		pixelMod = mod[modTable][((modBlock >> (index + 8)) & 0x1) + ((mostSig >> (index - 8)) & 0x2)];
	}

	red = _CLAMP_(red + pixelMod, 0, 255);
	green = _CLAMP_(green + pixelMod, 0, 255);
	blue = _CLAMP_(blue + pixelMod, 0, 255);

	return ((red << 16) + (green << 8) + blue) | 0xff000000;
}

static int ETCTextureDecompress(const void* pSrcData, int x, int y, const void* pDestData, int /*nMode*/)
{
	unsigned int blockTop, blockBot, *input = (unsigned int*)pSrcData, *output;
	unsigned char red1, green1, blue1, red2, green2, blue2;
	bool bFlip, bDiff;
	int modtable1, modtable2;

	for (int i = 0; i < y; i += 4)
	{
		for (int m = 0; m < x; m += 4)
		{
			blockTop = *(input++);
			blockBot = *(input++);

			output = (unsigned int*)pDestData + i * x + m;

			// check flipbit
			bFlip = (blockTop & ETC_FLIP) != 0;
			bDiff = (blockTop & ETC_DIFF) != 0;

			if (bDiff)
			{
				// differential mode 5 color bits + 3 difference bits
				// get base color for subblock 1
				blue1 = (unsigned char)((blockTop & 0xf80000) >> 16);
				green1 = (unsigned char)((blockTop & 0xf800) >> 8);
				red1 = (unsigned char)(blockTop & 0xf8);

				// get differential color for subblock 2
				signed char blues = (signed char)(blue1 >> 3) + ((signed char)((blockTop & 0x70000) >> 11) >> 5);
				signed char greens = (signed char)(green1 >> 3) + ((signed char)((blockTop & 0x700) >> 3) >> 5);
				signed char reds = (signed char)(red1 >> 3) + ((signed char)((blockTop & 0x7) << 5) >> 5);

				blue2 = (unsigned char)blues;
				green2 = (unsigned char)greens;
				red2 = (unsigned char)reds;

				red1 = red1 + (red1 >> 5);  // copy bits to lower sig
				green1 = green1 + (green1 >> 5);  // copy bits to lower sig
				blue1 = blue1 + (blue1 >> 5); // copy bits to lower sig

				red2 = (red2 << 3) + (red2 >> 2); // copy bits to lower sig
				green2 = (green2 << 3) + (green2 >> 2); // copy bits to lower sig
				blue2 = (blue2 << 3) + (blue2 >> 2);  // copy bits to lower sig
			}
			else
			{
				// individual mode 4 + 4 color bits
				// get base color for subblock 1
				blue1 = (unsigned char)((blockTop & 0xf00000) >> 16);
				blue1 = blue1 + (blue1 >> 4); // copy bits to lower sig
				green1 = (unsigned char)((blockTop & 0xf000) >> 8);
				green1 = green1 + (green1 >> 4);  // copy bits to lower sig
				red1 = (unsigned char)(blockTop & 0xf0);
				red1 = red1 + (red1 >> 4);  // copy bits to lower sig

				// get base color for subblock 2
				blue2 = (unsigned char)((blockTop & 0xf0000) >> 12);
				blue2 = blue2 + (blue2 >> 4); // copy bits to lower sig
				green2 = (unsigned char)((blockTop & 0xf00) >> 4);
				green2 = green2 + (green2 >> 4);  // copy bits to lower sig
				red2 = (unsigned char)((blockTop & 0xf) << 4);
				red2 = red2 + (red2 >> 4);  // copy bits to lower sig
			}
			// get the modtables for each subblock
			modtable1 = (blockTop >> 29) & 0x7;
			modtable2 = (blockTop >> 26) & 0x7;

			if (!bFlip)
			{
				// 2 2x4 blocks side by side

				for (int j = 0; j < 4; j++)   // vertical
				{
					for (int k = 0; k < 2; k++)   // horizontal
					{
						*(output + j * x + k) = modifyPixel(red1, green1, blue1, k, j, blockBot, modtable1);
						*(output + j * x + k + 2) = modifyPixel(red2, green2, blue2, k + 2, j, blockBot, modtable2);
					}
				}

			}
			else
			{
				// 2 4x2 blocks on top of each other
				for (int j = 0; j < 2; j++)
				{
					for (int k = 0; k < 4; k++)
					{
						*(output + j * x + k) = modifyPixel(red1, green1, blue1, k, j, blockBot, modtable1);
						*(output + (j + 2)*x + k) = modifyPixel(red2, green2, blue2, k, j + 2, blockBot, modtable2);
					}
				}
			}
		}
	}

	return x * y / 2;
}

int PVRTDecompressETC(const void* pSrcData,
                      unsigned int x,
                      unsigned int y,
                      void* pDestData,
                      int nMode)
{
	int i32read;

	if (x < ETC_MIN_TEXWIDTH || y < ETC_MIN_TEXHEIGHT)
	{
		// decompress into a buffer big enough to take the minimum size
		char* pTempBuffer = (char*)malloc(std::max<uint32>(x, ETC_MIN_TEXWIDTH) * std::max<uint32>(y, ETC_MIN_TEXHEIGHT) * 4);
		i32read = ETCTextureDecompress(pSrcData, std::max<uint32>(x, ETC_MIN_TEXWIDTH), std::max<uint32>(y, ETC_MIN_TEXHEIGHT),
		                               pTempBuffer, nMode);

		for (unsigned int i = 0; i < y; i++)
		{
			// copy from larger temp buffer to output data
			memcpy((char*)(pDestData) + i * x * 4, pTempBuffer + std::max<uint32>(x, ETC_MIN_TEXWIDTH) * 4 * i, x * 4);
		}

		if (pTempBuffer) { free(pTempBuffer); }
	}
	else  // decompress larger MIP levels straight into the output data
	{
		i32read = ETCTextureDecompress(pSrcData, x, y, pDestData, nMode);
	}

	// swap r and b channels
	unsigned char* pSwap = (unsigned char*)pDestData, swap;

	for (unsigned int i = 0; i < y; i++)
		for (unsigned int j = 0; j < x; j++)
		{
			swap = pSwap[0];
			pSwap[0] = pSwap[2];
			pSwap[2] = swap;
			pSwap += 4;
		}

	return i32read;
}
}
//!\endcond