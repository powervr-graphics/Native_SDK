#version 320 es

layout (set = 0, binding = 0) uniform mediump sampler2D sBaseTex0;
layout (set = 0, binding = 1) uniform mediump sampler2D sBaseTex1;
layout (set = 0, binding = 2) uniform mediump sampler2D sBaseTex2;
layout (set = 0, binding = 3) uniform mediump sampler2D sBaseTex3;
	
layout(location = 0) in highp vec2 TexCoords;
layout(location = 0) out mediump vec4 oColor;

void main()
{
   ivec2 texturePosition = ivec2(gl_FragCoord.x, gl_FragCoord.y);

   highp ivec2 moduleResult = ivec2(int(mod(float(texturePosition.x), 2.0)), int(mod(float(texturePosition.y), 2.0)));

   // Assuming:
   // sBaseTex0 has the upscaled pixel (0,0) from the [0,1]x[0,1] square generated
   // sBaseTex1 has the upscaled pixel (1,0) from the [0,1]x[0,1] square generated
   // sBaseTex2 has the upscaled pixel (0,1) from the [0,1]x[0,1] square generated
   // sBaseTex3 has the upscaled pixel (1,1) from the [0,1]x[0,1] square generated
   highp vec4 finalColor;
   if(moduleResult == ivec2(0,0))
   {
      ivec2 finalTexturePosition = texturePosition / 2;
      finalColor = texelFetch(sBaseTex0, finalTexturePosition, 0);
   }
   else if (moduleResult == ivec2(1,0))
   {
      ivec2 finalTexturePosition = (texturePosition / 2) + ivec2(1, 0);
      finalColor = texelFetch(sBaseTex1, finalTexturePosition, 0);  
   }
   else if (moduleResult == ivec2(0,1))
   {
      ivec2 finalTexturePosition = (texturePosition / 2) + ivec2(0, 1);
      finalColor = texelFetch(sBaseTex2, finalTexturePosition, 0);  
   }
   else if (moduleResult == ivec2(1,1))
   {
      ivec2 finalTexturePosition = (texturePosition / 2) + ivec2(1, 1);
      finalColor = texelFetch(sBaseTex3, finalTexturePosition, 0);  
   }
   
   oColor = finalColor;
}