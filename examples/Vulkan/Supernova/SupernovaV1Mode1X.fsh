#version 320 es

layout(set = 0, binding = 0) uniform highp isampler2D YTexture;
layout(set = 0, binding = 1) uniform highp isampler2D UVTexture;

layout(location = 0) in highp vec2 TexCoords;

layout(location = 0) out highp vec4 outputResult;

#define WINDOW_SIZE_Y_CHANNEL 5
#define WINDOW_SIZE_UVA_CHANNEL 2

highp const ivec3 YUVToRGBMatrixRow0 = ivec3( 16384,     0, 18675);
highp const ivec3 YUVToRGBMatrixRow1 = ivec3( 16384, -6466, -9512);
highp const ivec3 YUVToRGBMatrixRow2 = ivec3( 16384, 33297,     0);

mediump const int lowerClipValue = 82;
mediump const int contrastSensitivity = 96;

highp int arrayYData[WINDOW_SIZE_Y_CHANNEL][WINDOW_SIZE_Y_CHANNEL];

int clip_int(int x, int a, int b)
{
    if (x > b)
    {
        return b;
    }
    else if (x < a)
    {
        return a;
    }
    else
    {
        return x;
    }
}

int computeContrast()
{
    highp int maxValue = 0;
    highp int minValue = (1 << 12) - 1;
    for (int i = 1; i < WINDOW_SIZE_Y_CHANNEL; i++)
    {
        for (int j = 1; j < WINDOW_SIZE_Y_CHANNEL; j++)
        {
            highp int px = arrayYData[i][j];
            maxValue = max(maxValue, px);
            minValue = min(minValue, px);
        }
    }
    highp int contrast = maxValue - minValue;
    // processing contrast by clipping small values to 1 and multiplying by boost factor
    if (contrast < lowerClipValue)
    {
        contrast = (1 << 12) - 1;
    }
    // n.b. reg_contrast_sensitivity has 4 fractional bits. Not all 32 bits will be needed in this multiply.
    contrast = clip_int(contrast * contrastSensitivity, 0, (1 << 16) - 1) >> 4;
    return contrast;
}

int conv2d_int(int shift, int xOffset, int yOffset)
{
   // As the unsharp kernel has several zeros, do only the computation which return non-zero values
   //highp int unsharpKernel[WINDOW_SIZE_Y_CHANNEL - 1][WINDOW_SIZE_Y_CHANNEL - 1];
   //unsharpKernel[0][0] = -3;
   //unsharpKernel[0][1] = -5;
   //unsharpKernel[0][2] = -3;
   //unsharpKernel[0][3] =  0;
   //unsharpKernel[1][0] = -5;
   //unsharpKernel[1][1] = 48;
   //unsharpKernel[1][2] = -5;
   //unsharpKernel[1][3] =  0;
   //unsharpKernel[2][0] = -3;
   //unsharpKernel[2][1] = -5;
   //unsharpKernel[2][2] = -3;
   //unsharpKernel[2][3] =  0;
   //unsharpKernel[3][0] =  0;
   //unsharpKernel[3][1] =  0;
   //unsharpKernel[3][2] =  0;
   //unsharpKernel[3][3] =  0;

   // As the kernels are known beforehand, do only those computations
   // having a kernel value different from 0
   highp int result = 0;
   result += arrayYData[0 + xOffset][0 + yOffset] * -3; //unsharpKernel[0][0];
   result += arrayYData[0 + xOffset][1 + yOffset] * -5; //unsharpKernel[0][1];
   result += arrayYData[0 + xOffset][2 + yOffset] * -3; //unsharpKernel[0][2];
   result += arrayYData[1 + xOffset][0 + yOffset] * -5; //unsharpKernel[1][0];
   result += arrayYData[1 + xOffset][1 + yOffset] * 48; //unsharpKernel[1][1];
   result += arrayYData[1 + xOffset][2 + yOffset] * -5; //unsharpKernel[1][2];
   result += arrayYData[2 + xOffset][0 + yOffset] * -3; //unsharpKernel[2][0];
   result += arrayYData[2 + xOffset][1 + yOffset] * -5; //unsharpKernel[2][1];
   result += arrayYData[2 + xOffset][2 + yOffset] * -3; //unsharpKernel[2][2];

   // Perform rounding
   highp int tmp = result >> shift;
   highp int rem = result - (tmp << shift);
   if (rem >= (1 << (shift - 1)))
   {
      return tmp + 1;
   }
   else
   {
      return tmp;
   }
}

ivec3 round_half_up_vec3(ivec3 inp, ivec3 right_shift)
{
    return ((inp >> (right_shift - 1)) + 1) >> 1;
}

highp vec3 convertYUVToRGB(highp ivec3 YUV)
{
   // As the YUV to RGB matrix has several zeros, do only the computation for YUV->RGB which return non-zero values
   int RValue = YUV.x * YUVToRGBMatrixRow0.x + YUV.z * YUVToRGBMatrixRow0.z;
   int GValue = YUV.x * YUVToRGBMatrixRow1.x + YUV.y * YUVToRGBMatrixRow1.y + YUV.z * YUVToRGBMatrixRow1.z;
   int BValue = YUV.x * YUVToRGBMatrixRow2.x + YUV.y * YUVToRGBMatrixRow2.y;
   ivec3 RGBInteger = clamp(round_half_up_vec3(ivec3(RValue, GValue, BValue), ivec3(18)), 0, 255);
   return (vec3(RGBInteger) / 255.0);
}

void main()
{
   ivec2 texturePosition = ivec2(gl_FragCoord.x, gl_FragCoord.y);

   // Read a 5x5 area from YTexture and store the values in arrayYData
   // Writing manually this operations requires less registers than in a loop according to the profiling compilers for a BXM-8-256 GPU
   arrayYData[0][0] = texelFetch(YTexture, texturePosition + ivec2(-2, -2), 0).r;
   arrayYData[0][1] = texelFetch(YTexture, texturePosition + ivec2(-2, -1), 0).r;
   arrayYData[0][2] = texelFetch(YTexture, texturePosition + ivec2(-2,  0), 0).r;
   arrayYData[0][3] = texelFetch(YTexture, texturePosition + ivec2(-2,  1), 0).r;
   arrayYData[0][4] = texelFetch(YTexture, texturePosition + ivec2(-2,  2), 0).r;
   arrayYData[1][0] = texelFetch(YTexture, texturePosition + ivec2(-1, -2), 0).r;
   arrayYData[1][1] = texelFetch(YTexture, texturePosition + ivec2(-1, -1), 0).r;
   arrayYData[1][2] = texelFetch(YTexture, texturePosition + ivec2(-1,  0), 0).r;
   arrayYData[1][3] = texelFetch(YTexture, texturePosition + ivec2(-1,  1), 0).r;
   arrayYData[1][4] = texelFetch(YTexture, texturePosition + ivec2(-1,  2), 0).r;
   arrayYData[2][0] = texelFetch(YTexture, texturePosition + ivec2( 0, -2), 0).r;
   arrayYData[2][1] = texelFetch(YTexture, texturePosition + ivec2( 0, -1), 0).r;
   arrayYData[2][2] = texelFetch(YTexture, texturePosition + ivec2( 0,  0), 0).r;
   arrayYData[2][3] = texelFetch(YTexture, texturePosition + ivec2( 0,  1), 0).r;
   arrayYData[2][4] = texelFetch(YTexture, texturePosition + ivec2( 0,  2), 0).r;
   arrayYData[3][0] = texelFetch(YTexture, texturePosition + ivec2( 1, -2), 0).r;
   arrayYData[3][1] = texelFetch(YTexture, texturePosition + ivec2( 1, -1), 0).r;
   arrayYData[3][2] = texelFetch(YTexture, texturePosition + ivec2( 1,  0), 0).r;
   arrayYData[3][3] = texelFetch(YTexture, texturePosition + ivec2( 1,  1), 0).r;
   arrayYData[3][4] = texelFetch(YTexture, texturePosition + ivec2( 1,  2), 0).r;
   arrayYData[4][0] = texelFetch(YTexture, texturePosition + ivec2( 2, -2), 0).r;
   arrayYData[4][1] = texelFetch(YTexture, texturePosition + ivec2( 2, -1), 0).r;
   arrayYData[4][2] = texelFetch(YTexture, texturePosition + ivec2( 2,  0), 0).r;
   arrayYData[4][3] = texelFetch(YTexture, texturePosition + ivec2( 2,  1), 0).r;
   arrayYData[4][4] = texelFetch(YTexture, texturePosition + ivec2( 2,  2), 0).r;

   ivec2 UV = texelFetch(UVTexture, texturePosition, 0).rg;

   highp int contrast = computeContrast();

   highp int s_px = conv2d_int(4, 1, 1);
   highp int YResult = (contrast * arrayYData[2][2] + (((1 << 12) - 1) - contrast) * s_px) >> 12; // Blend values
   outputResult = vec4(convertYUVToRGB(ivec3(YResult, UV)), 1.0);
   outputResult.xyz = pow(outputResult.xyz, vec3(2.2));
}
