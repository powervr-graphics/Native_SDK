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

const highp vec2 positions[4] = vec2[4]
                                (
                                  vec2(-1., -1.),
                                  vec2(-1., 1.),
                                  vec2(1., -1.),
                                  vec2(1., 1.)
                                );

layout(set = 1, binding = 0) uniform TexOffsets
{
	mediump float TexelOffsetX;
	mediump float TexelOffsetY;
};

layout(location = 0) out mediump vec2 TexCoord0;
layout(location = 1) out mediump vec2 TexCoord1;
layout(location = 2) out mediump vec2 TexCoord2;

void main()
{
	highp vec2 position = positions[gl_VertexIndex];

	gl_Position = vec4(position, 0.5, 1.);

	// Calculate texture offsets and pass through
	mediump vec2 offset = vec2(TexelOffsetX, TexelOffsetY);

	highp vec2 vTexCoord = position * .5 + .5; //Map -1..1->0..1

	TexCoord0 = vTexCoord - offset;
	TexCoord1 = vTexCoord;
	TexCoord2 = vTexCoord + offset;
}