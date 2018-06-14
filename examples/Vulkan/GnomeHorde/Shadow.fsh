#version 450

layout(set = 0, binding = 0) uniform sampler2D tex;

layout(location = 0) in highp vec2 uvs;


layout(location = 0) out highp vec4 out_colour;

void main()
{


	out_colour = vec4(0.1, 0.0, 0.15, texture(tex, uvs).r);
}


