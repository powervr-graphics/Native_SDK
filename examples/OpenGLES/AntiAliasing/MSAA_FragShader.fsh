#version 310 es

uniform mediump sampler2DMS screenTexture;
		
layout(location = 0) out mediump vec4 oColor;

void main()
{
   mediump vec4 color = vec4(0.0);

   ivec2 texturePosition = ivec2(gl_FragCoord.x, gl_FragCoord.y);

   color += texelFetch(screenTexture, texturePosition, 0);
   color += texelFetch(screenTexture, texturePosition, 1);
   color += texelFetch(screenTexture, texturePosition, 2);
   color += texelFetch(screenTexture, texturePosition, 3);
   color /= 4.0f;

   oColor = color;
}
