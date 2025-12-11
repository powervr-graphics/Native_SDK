#version 320 es

layout(input_attachment_index = 0, set = 0, binding = 0) uniform mediump subpassInputMS sBaseTex;
	
layout(location = 0) in highp vec2 TexCoords;
layout(location = 0) out mediump vec4 oColor;

void main()
{
   mediump vec4 color = vec4(0.0);

   color += subpassLoad(sBaseTex, 0);
   color += subpassLoad(sBaseTex, 1);
   color += subpassLoad(sBaseTex, 2);
   color += subpassLoad(sBaseTex, 3);
   
   color /= 4.0f;
   oColor = color;
}