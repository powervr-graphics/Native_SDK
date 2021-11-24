#version 320 es

layout(push_constant) uniform PushConsts {
	highp vec2 offset0;
	highp vec2 offset1;
	highp float weight0;
	highp float weight1;
};

layout(location = 0) out mediump vec2 outUV[2];
layout(location = 2) out mediump float outWeight[2];

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
	mediump vec2 texcoord = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position           = vec4(texcoord * 2.0 + -1.0, 0.0, 1.0);
	outUV[0]              = texcoord + offset0;
	outUV[1]              = texcoord + offset1;
	outWeight[0]          = weight0;
	outWeight[1]          = weight1;
}
