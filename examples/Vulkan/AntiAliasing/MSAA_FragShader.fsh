#version 320 es

layout (set = 0, binding = 0) uniform mediump sampler2DMS sBaseTex;
	
layout(location = 0) in highp vec2 TexCoords;
layout(location = 0) out mediump vec4 oColor;

void main()
{
   mediump vec4 color = vec4(0.0);

   ivec2 texturePosition = ivec2(gl_FragCoord.x, gl_FragCoord.y);

   color += texelFetch(sBaseTex, texturePosition, 0);
   color += texelFetch(sBaseTex, texturePosition, 1);
   color += texelFetch(sBaseTex, texturePosition, 2);
   color += texelFetch(sBaseTex, texturePosition, 3);
   
   color /= 4.0f;
   oColor = color;
}