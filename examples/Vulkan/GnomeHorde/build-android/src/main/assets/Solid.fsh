#version 450

layout(set = 0, binding = 0) uniform sampler2D tex;

layout(location = 0) in highp vec2 uvs;
layout(location = 1) in highp float light_dot_norm;

layout(location = 0) out highp vec4 out_colour;

void main()
{
	highp vec3 tex_res = texture(tex, uvs).rgb;
	highp vec3 ambient = vec3(0.1, 0.0, 0.15);
	out_colour = vec4((ambient + light_dot_norm) * tex_res, 1.0);
}


