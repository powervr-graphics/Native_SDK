// Blur filter kernel shader
//
// 0  1  2  3  4
// x--x--X--x--x    <- original filter kernel
//   y---X---y      <- filter kernel abusing the hardware texture filtering
//       |
//      texel center
//
// 
// Using hardware texture filtering, the amount of samples can be
// reduced to three. To calculate the offset, use this formula:
// d = w1 / (w1 + w2),  whereas w1 and w2 denote the filter kernel weights

attribute highp   vec3  inVertex;
attribute mediump vec2  inTexCoord;

uniform mediump float  TexelOffsetX;
uniform mediump float  TexelOffsetY;

varying mediump vec2  TexCoord0;
varying mediump vec2  TexCoord1;
varying mediump vec2  TexCoord2;

void main()
{
	// Pass through vertex
	gl_Position = vec4(inVertex, 1.0);
	
	// Calculate texture offsets and pass through	
	mediump vec2 offset = vec2(TexelOffsetX, TexelOffsetY);
  
    TexCoord0 = inTexCoord - offset;
    TexCoord1 = inTexCoord;
    TexCoord2 = inTexCoord + offset;    

}