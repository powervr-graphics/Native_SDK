#version 300 es
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

uniform mediump float  TexelOffsetX;
uniform mediump float  TexelOffsetY;
out mediump vec2  TexCoord0;
out mediump vec2  TexCoord1;
out mediump vec2  TexCoord2;

void main()
{
	highp vec2 texcoord = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
	gl_Position = vec4(texcoord * 2.0 + -1.0, 0.0, 1.0);

	highp vec2 vTexCoord = texcoord;

	// Calculate texture offsets and pass through
	mediump vec2 offset = vec2(TexelOffsetX, TexelOffsetY);

	TexCoord0 = vTexCoord - offset;
	TexCoord1 = vTexCoord;
	TexCoord2 = vTexCoord + offset;
}