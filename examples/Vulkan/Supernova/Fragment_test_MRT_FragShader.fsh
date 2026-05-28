#version 320 es

layout (set = 0, binding = 0) uniform mediump sampler2D sBaseTex;
   
layout(location = 0) in highp vec2 TexCoords;

layout(location = 0) out mediump vec4 MRT0;
layout(location = 1) out mediump vec4 MRT1;
layout(location = 2) out mediump vec4 MRT2;
layout(location = 3) out mediump vec4 MRT3;

int round_half_up(int inp, int right_shift)
{
    return ((inp >> (right_shift - 1)) + 1) >> 1;
}

highp vec3 YUVToRGBMatrixRow0 = vec3( 16384.0,     0.0, 18675.0);
highp vec3 YUVToRGBMatrixRow1 = vec3( 16384.0, -6466.0, -9512.0);
highp vec3 YUVToRGBMatrixRow2 = vec3( 16384.0, 33297.0,     0.0);

void main()
{
   highp ivec2 texturePosition = ivec2(gl_FragCoord.x, gl_FragCoord.y);

   // Recreate the kernel size samples of SuperNova V1 Mode x2
   highp vec4 accumulatedValues = vec4(0.0);
   for(int i = -2; i <= 2; ++i)
   {
      for(int j = -2; j <= 2; ++j)
      {
         accumulatedValues += texelFetch(sBaseTex, texturePosition + ivec2(i, j), 0);
      }
   }

   accumulatedValues /= 25.0;

   // Recreate the conversion from YUVA to RGBA
   highp float alpha = float(0xFF);
   highp float R = float(round_half_up(int(dot(accumulatedValues.xyz, YUVToRGBMatrixRow0)), 18));
   highp float G = float(round_half_up(int(dot(accumulatedValues.xyz, YUVToRGBMatrixRow1)), 18));
   highp float B = float(round_half_up(int(dot(accumulatedValues.xyz, YUVToRGBMatrixRow2)), 18));
   highp vec4 outputRGBA = vec4(R, G, B, alpha);

   MRT0 = sin(outputRGBA);
   MRT1 = cos(outputRGBA);
   MRT2 = sinh(outputRGBA);
   MRT3 = cosh(outputRGBA);
}
