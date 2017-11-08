#version 450
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

layout(std140, set = 1, binding = 0) uniform TexOffsets
{
	mediump float TexelOffsetX;
	mediump float TexelOffsetY;
};

layout(location = 0) out mediump vec2 TexCoord0;
layout(location = 1) out mediump vec2 TexCoord1;
layout(location = 2) out mediump vec2 TexCoord2;

void main()
{
	highp vec2 texcoord = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(texcoord * 2.0 + -1.0, 0.0, 1.0);

	highp vec2 vTexCoord = texcoord;

	// Calculate texture offsets and pass through
	mediump vec2 offset = vec2(TexelOffsetX, TexelOffsetY);

	TexCoord0 = vTexCoord - offset;
	TexCoord1 = vTexCoord;
	TexCoord2 = vTexCoord + offset;
}