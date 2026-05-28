#version 320 es

layout(set = 0, binding = 0) uniform mediump isampler2D YTexture;
layout(set = 0, binding = 1) uniform mediump isampler2D UVTexture;

layout(location = 0) in highp vec2 TexCoords;

layout(location = 0) out mediump ivec4 outputY;
layout(location = 1) out mediump ivec4 outputU;
layout(location = 2) out mediump ivec4 outputV;

#define MAX_UNSIGNED_INT_16_BITS 65535
#define MAX_UNSIGNED_INT_12_BITS 4095

highp vec4 dataRow0;
highp vec4 dataRow1;

highp vec2 computeContrastLowRegistry_Part1()
{
    // Contrast is computed for the values of the neighbourhood from [-1,-1] to [2,2]
    // (-1, -1) (-1, 0) (-1, 1) (-1, 2)
    // ( 0, -1) ( 0, 0) ( 0, 1) ( 0, 2)
    // ( 1, -1) ( 1, 0) ( 1, 1) ( 1, 2)
    // ( 2, -1) ( 2, 0) ( 2, 1) ( 2, 2)

    // dataRow0 has indices ( 0,-1) ( 0, 0) ( 0, 1) ( 0, 2)
    // dataRow1 has indices ( 1,-1) ( 1, 0) ( 1, 1) ( 1, 2)

    // Currently only a part of the whole computation can be performed, with
    // the information in dataRow0 and dataRow1

    highp float maxValue = 0.0;
    highp float minValue = 4095.0;
    for (int i = 0; i < 4; i++)
    {
        maxValue = max(maxValue, max(dataRow0[i], dataRow1[i]));
        minValue = min(minValue, min(dataRow0[i], dataRow1[i]));
    }

    return vec2(minValue, maxValue);
}

highp int computeContrastLowRegistry_Part2(highp vec2 minMaxValues)
{
    // Contrast is computed for the values of the neighbourhood from [-1,-1] to [2,2]
    // (-1, -1) (-1, 0) (-1, 1) (-1, 2)
    // ( 0, -1) ( 0, 0) ( 0, 1) ( 0, 2)
    // ( 1, -1) ( 1, 0) ( 1, 1) ( 1, 2)
    // ( 2, -1) ( 2, 0) ( 2, 1) ( 2, 2)

    // dataRow0 has indices (-1, -1) (-1, 0) (-1, 1) (-1, 2)
    // dataRow1 has indices ( 2, -1) ( 2, 0) ( 2, 1) ( 2, 2)

    // The second part of the contrast computations can now be done

    highp float maxValue = minMaxValues.y;
    highp float minValue = minMaxValues.x;
    for (int i = 0; i < 4; i++)
    {
        maxValue = max(maxValue, max(dataRow0[i], dataRow1[i]));
        minValue = min(minValue, min(dataRow0[i], dataRow1[i]));
    }

    highp int contrast = int(maxValue - minValue);

    mediump const int lowerClipValue = 82;
    
    // processing contrast by clipping small values to 1 and multiplying by boost factor
    if (contrast < lowerClipValue)
    {
        contrast = MAX_UNSIGNED_INT_12_BITS;
    }

    mediump const int contrastSensitivity = 96;

    // n.b. reg_contrast_sensitivity has 4 fractional bits. Not all 32 bits will be needed in this multiply.
    contrast = clamp(contrast * contrastSensitivity, 0, MAX_UNSIGNED_INT_16_BITS);
    contrast >>= 4;
    return contrast;
}

int roundInteger(int value, int shift)
{
    highp int tmp = value >> shift;
    highp int rem = value - (tmp << shift);
    if (rem >= (1 << (shift - 1)))
    {
       return tmp + 1;
    }
    else
    {
       return tmp;
    }
}

ivec2 roundIntegerVec2(ivec2 value, int shift)
{
    ivec2 tmp = value >> ivec2(shift);
    ivec2 rem = value - (tmp << shift);
    ivec2 result;
    if (rem.x >= (1 << (shift - 1)))
    {
        result.x = tmp.x + 1;
    }
    else
    {
        result.x = tmp.x;
    }

    if (rem.y >= (1 << (shift - 1)))
    {
        result.y = tmp.y + 1;
    }
    else
    {
        result.y = tmp.y;
    }

    return result;
}

int conv2d_int_edgeX_LowRegistry_Part1(int shift)
{
    // Edge X is computed for all the in the neighbourhood, from [-2,-2] to [2,2]
    // (-2, -2) (-2, -1) (-2, 0) (-2, 1) (-2, 2)
    // (-1, -2) (-1, -1) (-1, 0) (-1, 1) (-1, 2)
    // ( 0, -2) ( 0, -1) ( 0, 0) ( 0, 1) ( 0, 2)
    // ( 1, -2) ( 1, -1) ( 1, 0) ( 1, 1) ( 1, 2)
    // ( 2, -2) ( 2, -1) ( 2, 0) ( 2, 1) ( 2, 2)

    // dataRow0 has indices ( 0,-1) ( 0, 0) ( 0, 1) ( 0, 2)
    // dataRow1 has indices ( 1,-1) ( 1, 0) ( 1, 1) ( 1, 2)

    // Currently only a part of the whole computation can be performed, with
    // the information in dataRow0 and dataRow1

    // As the edgesr x kernel has several zeros, do only the computation which return non-zero values
    /*const int16_t edgesr_x_kernel[5][5] = {
    { 0,  -1,  -1,  -1,   0},
    {-1,  -2,  -1,   0,   1},
    {-1,  -1,   0,   1,   1},
    {-1,   0,   1,   2,   1},
    { 0,   1,   1,   1,   0} };*/

    highp int result = int(
        dataRow0.x * -1.0 + 
        dataRow0.z        + 
        dataRow0.w        + 
        dataRow1.y        + 
        dataRow1.z *  2.0 + 
        dataRow1.w
        );

    return result;
}

int conv2d_int_edgeX_LowRegistry_Part2(int shift)
{
    // Edge X is computed for all the in the neighbourhood, from [-2,-2] to [2,2]
    // (-2, -2) (-2, -1) (-2, 0) (-2, 1) (-2, 2)
    // (-1, -2) (-1, -1) (-1, 0) (-1, 1) (-1, 2)
    // ( 0, -2) ( 0, -1) ( 0, 0) ( 0, 1) ( 0, 2)
    // ( 1, -2) ( 1, -1) ( 1, 0) ( 1, 1) ( 1, 2)
    // ( 2, -2) ( 2, -1) ( 2, 0) ( 2, 1) ( 2, 2)

    // dataRow0 has indices (-1, -1) (-1, 0) (-1, 1) (-1, 2)
    // dataRow1 has indices ( 2, -1) ( 2, 0) ( 2, 1) ( 2, 2)

    // Another part of the convolution can be done

    // As the edgesr x kernel has several zeros, do only the computation which return non-zero values
    /*const int16_t edgesr_x_kernel[5][5] = {
    { 0,  -1,  -1,  -1,   0},
    {-1,  -2,  -1,   0,   1},
    {-1,  -1,   0,   1,   1},
    {-1,   0,   1,   2,   1},
    { 0,   1,   1,   1,   0} };*/

    highp int result = int(
        dataRow0.x * -2.0 + 
        dataRow0.y * -1.0 + 
        dataRow0.w        + 
        dataRow1.x        + 
        dataRow1.y        + 
        dataRow1.z
        );

    return result;
}

int conv2d_int_edgeY_LowRegistry_Part1(int shift)
{
    // Edge Y is computed for all the in the neighbourhood, from [-2,-2] to [2,2]
    // (-2, -2) (-2, -1) (-2, 0) (-2, 1) (-2, 2)
    // (-1, -2) (-1, -1) (-1, 0) (-1, 1) (-1, 2)
    // ( 0, -2) ( 0, -1) ( 0, 0) ( 0, 1) ( 0, 2)
    // ( 1, -2) ( 1, -1) ( 1, 0) ( 1, 1) ( 1, 2)
    // ( 2, -2) ( 2, -1) ( 2, 0) ( 2, 1) ( 2, 2)

    // dataRow0 has indices ( 0,-1) ( 0, 0) ( 0, 1) ( 0, 2)
    // dataRow1 has indices ( 1,-1) ( 1, 0) ( 1, 1) ( 1, 2)

    // Currently only a part of the whole computation can be performed, with
    // the information in dataRow0 and dataRow1

    // As the edgesr x kernel has several zeros, do only the computation which return non-zero values
    /*const int16_t edgesr_y_kernel[5][5] = {
    { 0,   1,   1,   1,   0},
    {-1,   0,   1,   2,   1},
    {-1,  -1,   0,   1,   1},
    {-1,  -2,  -1,   0,   1},
    { 0,  -1,  -1,  -1,   0} };*/

    highp int result = int(
        dataRow0.x * -1.0 + 
        dataRow0.z        + 
        dataRow0.w        + 
        dataRow1.x * -2.0 + 
        dataRow1.y * -1.0 + 
        dataRow1.w
        );

    return result;
}

int conv2d_int_edgeY_LowRegistry_Part2(int shift)
{
    // Edge Y is computed for all the in the neighbourhood, from [-2,-2] to [2,2]
    // (-2, -2) (-2, -1) (-2, 0) (-2, 1) (-2, 2)
    // (-1, -2) (-1, -1) (-1, 0) (-1, 1) (-1, 2)
    // ( 0, -2) ( 0, -1) ( 0, 0) ( 0, 1) ( 0, 2)
    // ( 1, -2) ( 1, -1) ( 1, 0) ( 1, 1) ( 1, 2)
    // ( 2, -2) ( 2, -1) ( 2, 0) ( 2, 1) ( 2, 2)

    // dataRow0 has indices (-1, -1) (-1, 0) (-1, 1) (-1, 2)
    // dataRow1 has indices ( 2, -1) ( 2, 0) ( 2, 1) ( 2, 2)

    // Another part of the convolution can be done

    // As the edgesr x kernel has several zeros, do only the computation which return non-zero values
    /*const int16_t edgesr_y_kernel[5][5] = {
    { 0,   1,   1,   1,   0},
    {-1,   0,   1,   2,   1},
    {-1,  -1,   0,   1,   1},
    {-1,  -2,  -1,   0,   1},
    { 0,  -1,  -1,  -1,   0} };*/

    highp int result = int(
        dataRow0.y        + 
        dataRow0.z *  2.0 + 
        dataRow0.w        + 
        dataRow1.x * -1.0 + 
        dataRow1.y * -1.0 + 
        dataRow1.z * -1.0
        );

    return result;
}

// To generate new pixel (0,0) from the final set of new pixels [0,1]x[0,1]
int conv2d_int_Y_subRegion_pixel00LowRegistry(int shift) // Shift is 2
{
    // This is neighbourhood
    // ( 0, 0) ( 0, 1)
    // ( 1, 0) ( 1, 1)
    // Which is dataRow0.yz and dataRow1.yz

    // dataRow0 has indices ( 0,-1) ( 0, 0) ( 0, 1) ( 0, 2)
    // dataRow1 has indices ( 1,-1) ( 1, 0) ( 1, 1) ( 1, 2)

    //The kernel is
    // {
    // {4,0},
    // {0,0}
    // }
    // only one real multiplication to perform for the (0, 0) neighbourhood element
    highp int result = int(dataRow0.y * 4.0);

    // Perform rounding
    return roundInteger(result, shift);
}

// To generate new pixel (0,0) from the final set of new pixels [0,1]x[0,1]
int conv2d_int_Y_sharpRegion_pixel00_LowRegistry_Part1(int shift) // Shift is 4
{
    // This is neighbourhood
    // (-1, -1) (-1, 0) (-1, 1) (-1, 2)
    // ( 0, -1) ( 0, 0) ( 0, 1) ( 0, 2)
    // ( 1, -1) ( 1, 0) ( 1, 1) ( 1, 2)
    // ( 2, -1) ( 2, 0) ( 2, 1) ( 2, 2)

    // dataRow0 has indices ( 0,-1) ( 0, 0) ( 0, 1) ( 0, 2)
    // dataRow1 has indices ( 1,-1) ( 1, 0) ( 1, 1) ( 1, 2)

    // This means at the moment only a part of the convolution can be done as values
    // in rows (-1, ...) and (2, ...) are still not loaded

    //The kernel is
    // {
    // { -3, -5, -3, 0},
    // { -5, 48, -5, 0},
    // { -3, -5, -3, 0},
    // {  0,  0,  0, 0}
    // }
    highp int result = int(
        dataRow0.x * -5.0 + 
        dataRow0.y * 48.0 + 
        dataRow0.z * -5.0 + 
        dataRow1.x * -3.0 + 
        dataRow1.y * -5.0 + 
        dataRow1.z * -3.0
        );

    return result;
}

// To generate new pixel (0,0) from the final set of new pixels [0,1]x[0,1]
int conv2d_int_Y_sharpRegion_pixel00_LowRegistry_Part2(int shift) // Shift is 4
{
    // This is neighbourhood
    // (-1, -1) (-1, 0) (-1, 1) (-1, 2)
    // ( 0, -1) ( 0, 0) ( 0, 1) ( 0, 2)
    // ( 1, -1) ( 1, 0) ( 1, 1) ( 1, 2)
    // ( 2, -1) ( 2, 0) ( 2, 1) ( 2, 2)

    // dataRow0 has indices (-1, -1) (-1, 0) (-1, 1) (-1, 2)
    // dataRow1 has indices ( 2, -1) ( 2, 0) ( 2, 1) ( 2, 2)

    // The second part of the convolution can now be done

    //The kernel is
    // {
    // { -3, -5, -3, 0},
    // { -5, 48, -5, 0},
    // { -3, -5, -3, 0},
    // {  0,  0,  0, 0}
    // }
    highp int result = int(
        dataRow0.x * -3.0 + 
        dataRow0.y * -5.0 + 
        dataRow0.z * -3.0
        );

    return result;
}

// To generate new pixel (0,1) from the final set of new pixels [0,1]x[0,1]
int conv2d_int_Y_subRegion_pixel01LowRegistry(int shift) // Shift is 2
{
    // This is neighbourhood
    // ( 0, 0) ( 0, 1)
    // ( 1, 0) ( 1, 1)
    // Which is dataRow0.yz and dataRow1.yz

    // dataRow0 has indices ( 0,-1) ( 0, 0) ( 0, 1) ( 0, 2)
    // dataRow1 has indices ( 1,-1) ( 1, 0) ( 1, 1) ( 1, 2)

    //The kernel is
    // {
    // {2, 2},
    // {0, 0}
    // }
    // Only two real multiplications to perform for neighbourhood elements (0, 0) and (0, 1)
    highp int result = int(dataRow0.y * 2.0 + dataRow0.z * 2.0);

    // Perform rounding
    return roundInteger(result, shift);
}

// To generate new pixel (0,1) from the final set of new pixels [0,1]x[0,1]
int conv2d_int_Y_sharpRegion_pixel01_LowRegistry_Part1(int shift) // Shift is 4
{
    // This is neighbourhood
    // (-1, -1) (-1, 0) (-1, 1) (-1, 2)
    // ( 0, -1) ( 0, 0) ( 0, 1) ( 0, 2)
    // ( 1, -1) ( 1, 0) ( 1, 1) ( 1, 2)
    // ( 2, -1) ( 2, 0) ( 2, 1) ( 2, 2)

    // dataRow0 has indices ( 0,-1) ( 0, 0) ( 0, 1) ( 0, 2)
    // dataRow1 has indices ( 1,-1) ( 1, 0) ( 1, 1) ( 1, 2)

    // This means at the moment only a part of the convolution can be done as values
    // in rows (-1, ...) and (2, ...) are still not loaded

    //The kernel is
    // {
    // { -1, -4, -4, -1},
    // { -1, 19, 19, -1},
    // { -1, -4, -4, -1},
    // {  0,  0,  0,  0}
    // }
    highp int result = int(
        dataRow0.x * -1.0 + 
        dataRow0.y * 19.0 + 
        dataRow0.z * 19.0 + 
        dataRow0.w * -1.0 + 
        dataRow1.x * -1.0 + 
        dataRow1.y * -4.0 + 
        dataRow1.z * -4.0 + 
        dataRow1.w * -1.0
        );

    return result;
}

// To generate new pixel (0,1) from the final set of new pixels [0,1]x[0,1]
int conv2d_int_Y_sharpRegion_pixel01_LowRegistry_Part2(int shift) // Shift is 4
{
    // This is neighbourhood
    // (-1, -1) (-1, 0) (-1, 1) (-1, 2)
    // ( 0, -1) ( 0, 0) ( 0, 1) ( 0, 2)
    // ( 1, -1) ( 1, 0) ( 1, 1) ( 1, 2)
    // ( 2, -1) ( 2, 0) ( 2, 1) ( 2, 2)

    // dataRow0 has indices (-1, -1) (-1, 0) (-1, 1) (-1, 2)
    // dataRow1 has indices ( 2, -1) ( 2, 0) ( 2, 1) ( 2, 2)

    // The second part of the convolution can now be done

    //The kernel is
    // {
    // { -1, -4, -4, -1},
    // { -1, 19, 19, -1},
    // { -1, -4, -4, -1},
    // {  0,  0,  0,  0}
    // }
    highp int result = int(
        dataRow0.x * -1.0 + 
        dataRow0.y * -4.0 + 
        dataRow0.z * -4.0 + 
        dataRow0.w * -1.0
        );

    return result;
}

// To generate new pixel (1,0) from the final set of new pixels [0,1]x[0,1]
int conv2d_int_Y_subRegion_pixel10LowRegistry(int shift) // Shift is 2
{
    // This is neighbourhood
    // ( 0, 0) ( 0, 1)
    // ( 1, 0) ( 1, 1)
    // Which is dataRow0.yz and dataRow1.yz

    // dataRow0 has indices ( 0,-1) ( 0, 0) ( 0, 1) ( 0, 2)
    // dataRow1 has indices ( 1,-1) ( 1, 0) ( 1, 1) ( 1, 2)

    //The kernel is
    // {
    // {2, 0},
    // {2, 0}
    // }
    // Only two real multiplications to perform for neighbourhood elements (0, 0) and (1, 0)
    highp int result = int(dataRow0.y * 2.0 + dataRow1.y * 2.0);

    return roundInteger(result, shift);
}

// To generate new pixel (1,0) from the final set of new pixels [0,1]x[0,1]
int conv2d_int_Y_sharpRegion_pixel10_LowRegistry_Part1(int shift) // Shift is 4
{
    // This is neighbourhood
    // (-1, -1) (-1, 0) (-1, 1) (-1, 2)
    // ( 0, -1) ( 0, 0) ( 0, 1) ( 0, 2)
    // ( 1, -1) ( 1, 0) ( 1, 1) ( 1, 2)
    // ( 2, -1) ( 2, 0) ( 2, 1) ( 2, 2)

    // dataRow0 has indices ( 0,-1) ( 0, 0) ( 0, 1) ( 0, 2)
    // dataRow1 has indices ( 1,-1) ( 1, 0) ( 1, 1) ( 1, 2)

    // This means at the moment only a part of the convolution can be done as values
    // in rows (-1, ...) and (2, ...) are still not loaded

    //The kernel is
    // {
    // { -1, -1, -1,  0},
    // { -4, 19, -4,  0},
    // { -4, 19, -4,  0},
    // { -1, -1, -1,  0}
    // }
    highp int result = int(
        dataRow0.x * -4.0 + 
        dataRow0.y * 19.0 + 
        dataRow0.z * -4.0 + 
        dataRow1.x * -4.0 + 
        dataRow1.y * 19.0 + 
        dataRow1.z * -4.0
        );

    return result;
}

// To generate new pixel (1,0) from the final set of new pixels [0,1]x[0,1]
int conv2d_int_Y_sharpRegion_pixel10_LowRegistry_Part2(int shift) // Shift is 4
{
    // This is neighbourhood
    // (-1, -1) (-1, 0) (-1, 1) (-1, 2)
    // ( 0, -1) ( 0, 0) ( 0, 1) ( 0, 2)
    // ( 1, -1) ( 1, 0) ( 1, 1) ( 1, 2)
    // ( 2, -1) ( 2, 0) ( 2, 1) ( 2, 2)

    // dataRow0 has indices (-1, -1) (-1, 0) (-1, 1) (-1, 2)
    // dataRow1 has indices ( 2, -1) ( 2, 0) ( 2, 1) ( 2, 2)

    // The second part of the convolution can now be done

    //The kernel is
    // {
    // { -1, -1, -1,  0},
    // { -4, 19, -4,  0},
    // { -4, 19, -4,  0},
    // { -1, -1, -1,  0}
    // }
    highp int result = int( -1.0 * (
        dataRow0.x + 
        dataRow0.y + 
        dataRow0.z + 
        dataRow1.x + 
        dataRow1.y + 
        dataRow1.z 
        )
        );

    return result;
}

// To generate new pixel (1,1) from the final set of new pixels [0,1]x[0,1]
int conv2d_int_Y_subRegion_pixel11_edgeALowRegistry(int shift) // Shift is 1
{
    // This is neighbourhood
    // ( 0, 0) ( 0, 1)
    // ( 1, 0) ( 1, 1)
    // Which is dataRow0.yz and dataRow1.yz

    // dataRow0 has indices ( 0,-1) ( 0, 0) ( 0, 1) ( 0, 2)
    // dataRow1 has indices ( 1,-1) ( 1, 0) ( 1, 1) ( 1, 2)

    //The kernel is
    // {
    // {1, 0},
    // {0, 1}
    // }
    // Only two real multiplications to perform for neighbourhood elements (0, 0) and (1, 1)
    highp int result = int(dataRow0.y + dataRow1.z);

    // Perform rounding
    return roundInteger(result, shift);
}

// To generate new pixel (1,1) from the final set of new pixels [0,1]x[0,1]
int conv2d_int_Y_subRegion_pixel11_edgeBLowRegistry(int shift) // Shift is 1
{
    // This is neighbourhood
    // ( 0, 0) ( 0, 1)
    // ( 1, 0) ( 1, 1)
    // Which is dataRow0.yz and dataRow1.yz

    // dataRow0 has indices ( 0,-1) ( 0, 0) ( 0, 1) ( 0, 2)
    // dataRow1 has indices ( 1,-1) ( 1, 0) ( 1, 1) ( 1, 2)

    //The kernel is
    // {
    // {0, 1},
    // {1, 0}
    // }
    // Only two real multiplications to perform for neighbourhood elements (0, 1) and (1, 0)
    highp int result = int(dataRow0.z + dataRow1.y);

    // Perform rounding
    return roundInteger(result, shift);
}

// To generate new pixel (1,0) from the final set of new pixels [0,1]x[0,1]
int conv2d_int_Y_sharpRegion_pixel11_A_LowRegistry_Part1(int shift) // Shift is 4
{
    // This is neighbourhood
    // (-1, -1) (-1, 0) (-1, 1) (-1, 2)
    // ( 0, -1) ( 0, 0) ( 0, 1) ( 0, 2)
    // ( 1, -1) ( 1, 0) ( 1, 1) ( 1, 2)
    // ( 2, -1) ( 2, 0) ( 2, 1) ( 2, 2)

    // dataRow0 has indices ( 0,-1) ( 0, 0) ( 0, 1) ( 0, 2)
    // dataRow1 has indices ( 1,-1) ( 1, 0) ( 1, 1) ( 1, 2)

    // This means at the moment only a part of the convolution can be done as values
    // in rows (-1, ...) and (2, ...) are still not loaded

    //The kernel is
    // {
    // {  0, -1, -1,  0},
    // { -1, 16, -4, -1},
    // { -1, -4, 16, -1},
    // {  0, -1, -1,  0}
    // }
    highp int result = int(
        dataRow0.x * -1.0 + 
        dataRow0.y * 16.0 + 
        dataRow0.z * -4.0 + 
        dataRow0.w * -1.0 + 
        dataRow1.x * -1.0 + 
        dataRow1.y * -4.0 + 
        dataRow1.z * 16.0 + 
        dataRow1.w * -1.0
        );

    return result;
}

// To generate new pixel (1,0) from the final set of new pixels [0,1]x[0,1]
int conv2d_int_Y_sharpRegion_pixel11_A_LowRegistry_Part2(int shift) // Shift is 4
{
    // This is neighbourhood
    // (-1, -1) (-1, 0) (-1, 1) (-1, 2)
    // ( 0, -1) ( 0, 0) ( 0, 1) ( 0, 2)
    // ( 1, -1) ( 1, 0) ( 1, 1) ( 1, 2)
    // ( 2, -1) ( 2, 0) ( 2, 1) ( 2, 2)

    // dataRow0 has indices (-1, -1) (-1, 0) (-1, 1) (-1, 2)
    // dataRow1 has indices ( 2, -1) ( 2, 0) ( 2, 1) ( 2, 2)

    // The second part of the convolution can now be done

    //The kernel is
    // {
    // {  0, -1, -1,  0},
    // { -1, 16, -4, -1},
    // { -1, -4, 16, -1},
    // {  0, -1, -1,  0}
    // }
    highp int result = int(-1.0 * (
        dataRow0.y + 
        dataRow0.z + 
        dataRow1.y + 
        dataRow1.z
        )
        );

    return result;
}

// To generate new pixel (1,0) from the final set of new pixels [0,1]x[0,1]
int conv2d_int_Y_sharpRegion_pixel11_B_LowRegistry_Part1(int shift) // Shift is 4
{
    // This is neighbourhood
    // (-1, -1) (-1, 0) (-1, 1) (-1, 2)
    // ( 0, -1) ( 0, 0) ( 0, 1) ( 0, 2)
    // ( 1, -1) ( 1, 0) ( 1, 1) ( 1, 2)
    // ( 2, -1) ( 2, 0) ( 2, 1) ( 2, 2)

    // dataRow0 has indices ( 0,-1) ( 0, 0) ( 0, 1) ( 0, 2)
    // dataRow1 has indices ( 1,-1) ( 1, 0) ( 1, 1) ( 1, 2)

    // This means at the moment only a part of the convolution can be done as values
    // in rows (-1, ...) and (2, ...) are still not loaded

    //The kernel is
    // {
    // {  0, -1, -1,  0},
    // { -1, -4, 16, -1},
    // { -1, 16, -4, -1},
    // {  0, -1, -1,  0}
    // }
    highp int result = int(
        dataRow0.x * -1.0 + 
        dataRow0.y * -4.0 + 
        dataRow0.z * 16.0 + 
        dataRow0.w * -1.0 + 
        dataRow1.x * -1.0 + 
        dataRow1.y * 16.0 + 
        dataRow1.z * -4.0 + 
        dataRow1.w * -1.0
        );
    
    return result;
}

// To generate new pixel (1,0) from the final set of new pixels [0,1]x[0,1]
int conv2d_int_Y_sharpRegion_pixel11_B_LowRegistry_Part2(int shift) // Shift is 4
{
    // This is neighbourhood
    // (-1, -1) (-1, 0) (-1, 1) (-1, 2)
    // ( 0, -1) ( 0, 0) ( 0, 1) ( 0, 2)
    // ( 1, -1) ( 1, 0) ( 1, 1) ( 1, 2)
    // ( 2, -1) ( 2, 0) ( 2, 1) ( 2, 2)

    // dataRow0 has indices (-1, -1) (-1, 0) (-1, 1) (-1, 2)
    // dataRow1 has indices ( 2, -1) ( 2, 0) ( 2, 1) ( 2, 2)

    // The second part of the convolution can now be done

    //The kernel is
    // {
    // {  0, -1, -1,  0},
    // { -1, -4, 16, -1},
    // { -1, 16, -4, -1},
    // {  0, -1, -1,  0}
    // }
    highp int result = int(-1.0 * (
        dataRow0.y + 
        dataRow0.z + 
        dataRow1.y + 
        dataRow1.z
        )
        );
    
    // Perform rounding
    return result;
}

ivec2 conv2d_int_UV_subRegion_pixel00_LowRegistry(int shift) // Shift is 2
{
    // dataRow0 row has indices (0, 0) (0, 1) (1, 0) (1, 1) for U channel
    // dataRow1 row has indices (0, 0) (0, 1) (1, 0) (1, 1) for V channel

    //The kernel is
    // {
    // {4,0},
    // {0,0}
    // }
    // only one real multiplication to perform for the [0][0] element
    highp ivec2 resultUV = ivec2(dataRow0.x * 4.0, dataRow1.x * 4.0);

    // Perform rounding
    return roundIntegerVec2(resultUV, shift);
}

ivec2 conv2d_int_UV_subRegion_pixel10_LowRegistry(int shift) // Shift is 2
{
    // dataRow0 row has indices (0, 0) (0, 1) (1, 0) (1, 1) for U channel
    // dataRow1 row has indices (0, 0) (0, 1) (1, 0) (1, 1) for V channel

    //The kernel is
    // {
    // {2,0},
    // {2,0}
    // }
    highp ivec2 resultUV = ivec2(
        dataRow0.x * 2.0 + dataRow0.z * 2.0,
        dataRow1.x * 2.0 + dataRow1.z * 2.0
        );

    // Perform rounding
    return roundIntegerVec2(resultUV, shift);
}

// Pixel (0,1) UV channel convolution
ivec2 conv2d_int_UV_subRegion_pixel01_LowRegistry(int shift) // Shift is 2
{
    // dataRow0 row has indices (0, 0) (0, 1) (1, 0) (1, 1) for U channel
    // dataRow1 row has indices (0, 0) (0, 1) (1, 0) (1, 1) for V channel

    //The kernel is
    // {
    // {2,2},
    // {0,0}
    // }
    highp ivec2 resultUV = ivec2(
        dataRow0.x * 2.0 + dataRow0.y * 2.0,
        dataRow1.x * 2.0 + dataRow1.y * 2.0
        );

    // Perform rounding
    return roundIntegerVec2(resultUV, shift);
}

// Pixel (1,1) UV channel convolution
ivec2 conv2d_int_UV_subRegion_pixel11_LowRegistry(int shift) // Shift is 2
{
    // Apply a convolution for the arrayYData subregion

    //The kernel is
    // {
    // {1,1},
    // {1,1}
    // }
    highp ivec2 resultUV = ivec2(
        dot(dataRow0, vec4(1.0)),
        dot(dataRow1, vec4(1.0))
        );

    // Perform rounding
    return roundIntegerVec2(resultUV, shift);
}

void completeEdgeXEdgeYComputations(inout highp int edgeX, inout highp int edgeY)
{
    ivec2 texturePosition = ivec2(gl_FragCoord.x, gl_FragCoord.y);

    /*edgeX kernel
    { 0,  -1,  -1,  -1,   0},
    {-1,  -2,  -1,   0,   1},
    {-1,  -1,   0,   1,   1},
    {-1,   0,   1,   2,   1},
    { 0,   1,   1,   1,   0}

    edgeY kernel
    { 0,   1,   1,   1,   0},
    {-1,   0,   1,   2,   1},
    {-1,  -1,   0,   1,   1},
    {-1,  -2,  -1,   0,   1},
    { 0,  -1,  -1,  -1,   0}*/

    dataRow0.xyz = vec3(
         float(texelFetch(YTexture, texturePosition + ivec2(-2, -1), 0).r), 
         float(texelFetch(YTexture, texturePosition + ivec2(-2,  0), 0).r),
         float(texelFetch(YTexture, texturePosition + ivec2(-2,  1), 0).r)
        );

    dataRow1.xyz = vec3(
         float(texelFetch(YTexture, texturePosition + ivec2(-1, -2), 0).r), 
         float(texelFetch(YTexture, texturePosition + ivec2( 0, -2), 0).r),
         float(texelFetch(YTexture, texturePosition + ivec2( 1, -2), 0).r)
        );

    edgeX -= int(dataRow0.x + dataRow0.y + dataRow0.z + dataRow1.x + dataRow1.y + dataRow1.z);
    edgeY += int(dataRow0.x + dataRow0.y + dataRow0.z);
    edgeY -= int(dataRow1.x + dataRow1.y + dataRow1.z);

    // Rounding when shift is zero, in this case the computations result in adding "1" to the value.
    edgeX += 1;
    edgeY += 1;

    edgeX = abs(edgeX);
    edgeY = abs(edgeY);
}

void main()
{
    mediump ivec2 texturePosition = ivec2(gl_FragCoord.x, gl_FragCoord.y);

    // This algorithm loads a 5x5 pixel neighbourhood centered on the pixel being processed:
    // (-2,-2) (-2,-1) (-2, 0) (-2, 1) (-2, 2)
    // (-1,-2) (-1,-1) (-1, 0) (-1, 1) (-1, 2)
    // ( 0,-2) ( 0,-1) ( 0, 0) ( 0, 1) ( 0, 2)
    // ( 1,-2) ( 1,-1) ( 1, 0) ( 1, 1) ( 1, 2)
    // ( 2,-2) ( 2,-1) ( 2, 0) ( 2, 1) ( 2, 2)

    // Several convolutions are done with this information, not always requiring the
    // whole neighbourhood:
    //
    // Sub region convolutions need indices
    // ( 0, 0) ( 0, 1)
    // ( 1, 0) ( 1, 1)
    //
    // Sharp region convolutions need indices
    // (-1,-1) (-1, 0) (-1, 1) (-1, 2)
    // ( 0,-1) ( 0, 0) ( 0, 1) ( 0, 2)
    // ( 1,-1) ( 1, 0) ( 1, 1) ( 1, 2)
    // ( 2,-1) ( 2, 0) ( 2, 1) ( 2, 2)

    // Edge X, Edge Y and contrast computations need the whole neighbourhood

    // To reduce the texturing bottleneck, subparts of the 5x5 neighbourhood will be loaded
    // to perform the sub region and sharp region convolutions in batches, loading eight
    // elements of the neighbourhood from two rows (four elements of each row).
    // The variables dataRow0 and dataRow1 will contain those values.

    // First loading operation:
    // Load in dataRow0 row neighbourhood indices ( 0,-1) ( 0, 0) ( 0, 1) ( 0, 2)
    // Load in dataRow1 row neighbourhood indices ( 1,-1) ( 1, 0) ( 1, 1) ( 1, 2)
    dataRow0 = vec4(
        float(texelFetch(YTexture, texturePosition + ivec2( 0, -1), 0).r),
        float(texelFetch(YTexture, texturePosition + ivec2( 0,  0), 0).r),
        float(texelFetch(YTexture, texturePosition + ivec2( 0,  1), 0).r),
        float(texelFetch(YTexture, texturePosition + ivec2( 0,  2), 0).r)
        );

    dataRow1 = vec4(
        float(texelFetch(YTexture, texturePosition + ivec2( 1, -1), 0).r),
        float(texelFetch(YTexture, texturePosition + ivec2( 1,  0), 0).r),
        float(texelFetch(YTexture, texturePosition + ivec2( 1,  1), 0).r),
        float(texelFetch(YTexture, texturePosition + ivec2( 1,  2), 0).r)
        );

    highp vec2 minMaxValue = computeContrastLowRegistry_Part1();

    // Generate pixel 00 Y information
    highp int px00 = conv2d_int_Y_subRegion_pixel00LowRegistry(2);
    highp int s_px00 = conv2d_int_Y_sharpRegion_pixel00_LowRegistry_Part1(4);

    // Generate pixel 01 Y information
    highp int px01 = conv2d_int_Y_subRegion_pixel01LowRegistry(2);
    highp int s_px01 = conv2d_int_Y_sharpRegion_pixel01_LowRegistry_Part1(4);

    // Generate pixel 10 Y information
    highp int px10 = conv2d_int_Y_subRegion_pixel10LowRegistry(2);
    highp int s_px10 = conv2d_int_Y_sharpRegion_pixel10_LowRegistry_Part1(4);

    // Generate pixel 11 Y information
    highp int edge_x = conv2d_int_edgeX_LowRegistry_Part1(0);
    highp int edge_y = conv2d_int_edgeY_LowRegistry_Part1(0);

    highp int subRegionP11EdgeAPart1Result = conv2d_int_Y_subRegion_pixel11_edgeALowRegistry(1);
    highp int subRegionP11EdgeBPart1Result = conv2d_int_Y_subRegion_pixel11_edgeBLowRegistry(1);

    highp int sharpRegionP11EdgeAPart1Result = conv2d_int_Y_sharpRegion_pixel11_A_LowRegistry_Part1(4);
    highp int sharpRegionP11EdgeBPart1Result = conv2d_int_Y_sharpRegion_pixel11_B_LowRegistry_Part1(4);
    
    // Once all operations with the current values from dataRow0 and dataRow1
    // have been done, load new rows from the 5x5 neighbourhood to complete
    // the pending convolution computations (sharp region, edge X and edge y)
    // Second loading operation:
    // Load in dataRow0 row neighbourhood indices (-1,-1) (-1, 0) (-1, 1) (-1, 2)
    // Load in dataRow1 row neighbourhood indices ( 2,-1) ( 2, 0) ( 2, 1) ( 2, 2)
    dataRow0 = vec4(
        float(texelFetch(YTexture, texturePosition + ivec2(-1, -1), 0).r),
        float(texelFetch(YTexture, texturePosition + ivec2(-1,  0), 0).r),
        float(texelFetch(YTexture, texturePosition + ivec2(-1,  1), 0).r),
        float(texelFetch(YTexture, texturePosition + ivec2(-1,  2), 0).r)
        );

    dataRow1 = vec4(
        float(texelFetch(YTexture, texturePosition + ivec2( 2, -1), 0).r),
        float(texelFetch(YTexture, texturePosition + ivec2( 2,  0), 0).r),
        float(texelFetch(YTexture, texturePosition + ivec2( 2,  1), 0).r),
        float(texelFetch(YTexture, texturePosition + ivec2( 2,  2), 0).r)
        );

    // Complete sharp region pixel 00 Y information
    s_px00 += conv2d_int_Y_sharpRegion_pixel00_LowRegistry_Part2(4);
    s_px00 = roundInteger(s_px00, 4);
    s_px00 = clamp(s_px00, 0, MAX_UNSIGNED_INT_12_BITS);

    // Complete sharp region pixel 01 Y information
    s_px01 += conv2d_int_Y_sharpRegion_pixel01_LowRegistry_Part2(4);
    s_px01 = roundInteger(s_px01, 4);
    s_px01 = clamp(s_px01, 0, MAX_UNSIGNED_INT_12_BITS);

    // Complete sharp region pixel 10 Y information
    s_px10 += conv2d_int_Y_sharpRegion_pixel10_LowRegistry_Part2(4);
    s_px10 = roundInteger(s_px10, 4);
    s_px10 = clamp(s_px10, 0, MAX_UNSIGNED_INT_12_BITS);
    
    // Complete sharp region pixel 11 Y information
    edge_x += conv2d_int_edgeX_LowRegistry_Part2(0); // PENDING PART 3, AVIOD REPEATING THE CALL TO roundInteger
    edge_y += conv2d_int_edgeY_LowRegistry_Part2(0); // PENDING PART 3, AVIOD REPEATING THE CALL TO roundInteger

    completeEdgeXEdgeYComputations(edge_x, edge_y);
    const mediump int config_edge_sr_strength = 16;
    highp int edge_a = (1 << 15) - ((edge_x - edge_y) * config_edge_sr_strength);
    edge_a = clamp(edge_a, 0, MAX_UNSIGNED_INT_16_BITS);
    mediump int edge_b = MAX_UNSIGNED_INT_16_BITS - edge_a;
    highp int px11 = (subRegionP11EdgeAPart1Result * edge_a + subRegionP11EdgeBPart1Result * edge_b) >> 16;

    highp int s_px11 = roundInteger(sharpRegionP11EdgeAPart1Result + conv2d_int_Y_sharpRegion_pixel11_A_LowRegistry_Part2(4), 4) * edge_a + roundInteger(sharpRegionP11EdgeBPart1Result + conv2d_int_Y_sharpRegion_pixel11_B_LowRegistry_Part2(4), 4) * edge_b;

    s_px11 = s_px11 >> 16;
    s_px11 = clamp(s_px11, 0, MAX_UNSIGNED_INT_16_BITS);

    // Complete contrast computations
    highp int contrast = computeContrastLowRegistry_Part2(minMaxValue);

    // For the U and V channels, small sub region convolutions need to be done
    // Lading the information in dataRow0 and dataRow1
    // Load in dataRow0 row neighbourhood indices (0, 0) (0, 1) (1, 0) (1, 1) for U channel
    // Load in dataRow1 row neighbourhood indices (0, 0) (0, 1) (1, 0) (1, 1) for V channel
    dataRow0 = vec4(
        float(texelFetch(UVTexture, texturePosition + ivec2(0, 0), 0).r),
        float(texelFetch(UVTexture, texturePosition + ivec2(0, 1), 0).r),
        float(texelFetch(UVTexture, texturePosition + ivec2(1, 0), 0).r),
        float(texelFetch(UVTexture, texturePosition + ivec2(1, 1), 0).r)
        );

    dataRow1 = vec4(
        float(texelFetch(UVTexture, texturePosition + ivec2(0, 0), 0).g),
        float(texelFetch(UVTexture, texturePosition + ivec2(0, 1), 0).g),
        float(texelFetch(UVTexture, texturePosition + ivec2(1, 0), 0).g),
        float(texelFetch(UVTexture, texturePosition + ivec2(1, 1), 0).g)
        );

    // Generate each final pixel
    ivec4 P00;
    P00.x = (contrast * px00 + (MAX_UNSIGNED_INT_12_BITS - contrast) * s_px00) >> 12; //blend
    P00.yz = conv2d_int_UV_subRegion_pixel00_LowRegistry(2); // (UV channels)

    ivec4 P10;
    P10.x = (contrast * px10 + (MAX_UNSIGNED_INT_12_BITS - contrast) * s_px10) >> 12; //blend
    P10.yz = conv2d_int_UV_subRegion_pixel10_LowRegistry(2); // (UV channels)

    ivec4 P01;
    P01.x = (contrast * px01 + (MAX_UNSIGNED_INT_12_BITS - contrast) * s_px01) >> 12; //blend
    P01.yz = conv2d_int_UV_subRegion_pixel01_LowRegistry(2); // (UV channels)

    ivec4 P11;
    P11.x = (contrast * px11 + (MAX_UNSIGNED_INT_12_BITS - contrast) * s_px11) >> 12; //blend
    P11.yz = conv2d_int_UV_subRegion_pixel11_LowRegistry(2); // (UV channels)

    outputY = ivec4(P00.x, P10.x, P01.x, P11.x);
    outputU = ivec4(P00.y, P10.y, P01.y, P11.y);
    outputV = ivec4(P00.z, P10.z, P01.z, P11.z);
}
