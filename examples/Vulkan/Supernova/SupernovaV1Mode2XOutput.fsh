#version 320 es

layout(set = 0, binding = 0) uniform highp isampler2D YTexture;
layout(set = 0, binding = 1) uniform highp isampler2D UTexture;
layout(set = 0, binding = 2) uniform highp isampler2D VTexture;

layout(location = 0) in highp vec2 TexCoords;

layout(location = 0) out highp vec4 outputResult;

highp const ivec3 YUVToRGBMatrixRow0 = ivec3( 16384,     0, 18675);
highp const ivec3 YUVToRGBMatrixRow1 = ivec3( 16384, -6466, -9512);
highp const ivec3 YUVToRGBMatrixRow2 = ivec3( 16384, 33297,     0);

ivec3 round_half_up_vec3(ivec3 inp, ivec3 right_shift)
{
    return ((inp >> (right_shift - 1)) + 1) >> 1;
}

highp vec3 convertYUVToRGB(highp ivec3 YUV)
{
   // As the YUV to RGB matrix has several zeros, do only the computation for YUV->RGB which return non-zero values
   highp int RValue = YUV.x * YUVToRGBMatrixRow0.x + YUV.z * YUVToRGBMatrixRow0.z;
   highp int GValue = YUV.x * YUVToRGBMatrixRow1.x + YUV.y * YUVToRGBMatrixRow1.y + YUV.z * YUVToRGBMatrixRow1.z;
   highp int BValue = YUV.x * YUVToRGBMatrixRow2.x + YUV.y * YUVToRGBMatrixRow2.y;
   highp ivec3 RGBInteger = clamp(round_half_up_vec3(ivec3(RValue, GValue, BValue), ivec3(18)), 0, 255);
   return (vec3(RGBInteger) / 255.0);
}

void main()
{
    // YTexture is a VK_FORMAT_R16G16B16A16_SINT texture where each one of the four generated pixels' Y channel is stored
    // UTexture is a VK_FORMAT_R16G16B16A16_SINT texture where each one of the four generated pixels' U channel is stored
    // VTexture is a VK_FORMAT_R16G16B16A16_SINT texture where each one of the four generated pixels' V channel is stored

    ivec2 texturePosition = ivec2(gl_FragCoord.x, gl_FragCoord.y);
    highp ivec2 moduleResult = ivec2(int(mod(float(texturePosition.x), 2.0)), int(mod(float(texturePosition.y), 2.0)));

    ivec2 finalTexturePosition = texturePosition / 2;
    highp ivec4 YChannel = texelFetch(YTexture, finalTexturePosition, 0);
    highp ivec4 UChannel = texelFetch(UTexture, finalTexturePosition, 0);
    highp ivec4 VChannel = texelFetch(VTexture, finalTexturePosition, 0);

    int moduleResultToBase10 = moduleResult.x + moduleResult.y * 2;

    highp int YFinal = YChannel[moduleResultToBase10];
    highp int UFinal = UChannel[moduleResultToBase10];
    highp int VFinal = VChannel[moduleResultToBase10];

    outputResult = vec4(convertYUVToRGB(ivec3(YFinal, UFinal, VFinal)), 1.0);
    outputResult.xyz = pow(outputResult.xyz, vec3(2.2));
}
