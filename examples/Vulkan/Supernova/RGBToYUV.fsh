#version 320 es

layout (set = 0, binding = 0) uniform highp sampler2D sBaseTex;
   
layout(location = 0) in highp vec2 TexCoords;

layout(location = 0) out highp int outputY;
layout(location = 1) out highp ivec2 outputUV;

highp ivec3 RGBToYUVMatrixRow0 = ivec3( 4899,   9617,  1868);
highp ivec3 RGBToYUVMatrixRow1 = ivec3(-2411,  -4733,  7143);
highp ivec3 RGBToYUVMatrixRow2 = ivec3(10076,  -8438, -1639);

highp ivec3 round_half_up_vec3(highp ivec3 inp, highp ivec3 right_shift)
{
    return ((inp >> (right_shift - 1)) + 1) >> 1;
}

int integerDot(ivec3 a, ivec3 b)
{
   return a.x * b.x + a.y * b.y + a.z * b.z;
}

void main()
{
   ivec2 texturePosition = ivec2(gl_FragCoord.x, gl_FragCoord.y);
   highp vec4 textureValue = texelFetch(sBaseTex, texturePosition, 0) * 255.0;

   highp ivec4 textureValueInteger = ivec4(textureValue + vec4(0.5));
   
   highp ivec3 YUV = ivec3(integerDot(textureValueInteger.xyz, RGBToYUVMatrixRow0),
                           integerDot(textureValueInteger.xyz, RGBToYUVMatrixRow1),
                           integerDot(textureValueInteger.xyz, RGBToYUVMatrixRow2));

   YUV = round_half_up_vec3(YUV, ivec3(10));

   outputY = YUV.x;
   outputUV = YUV.yz;
}
