#version 450

layout(set = 1, binding = 0) uniform _per_object {
	highp mat4 world_from_model;
	highp mat4 model_from_world;
} per_object;
layout(set = 2, binding = 0) uniform _per_frame {
	highp mat4 projection_from_world;
} per_frame;

layout(location = 0) in highp vec3 in_vertex_position;
layout(location = 1) in highp vec3 in_normal;
layout(location = 2) in highp vec2 in_uvs;



layout(location = 0) out highp vec2 uvs;


void main()
{
	uvs = in_uvs;

	gl_Position = per_frame.projection_from_world * per_object.world_from_model * vec4(in_vertex_position, 1.0);

	highp vec3 light_direction = normalize(vec3(0.7,0.7,0.0));
	
	
	
}

