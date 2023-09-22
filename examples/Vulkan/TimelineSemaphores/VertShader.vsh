#version 320 es
#define VERTEX_ARRAY	0
#define NORMAL_ARRAY	1
#define TEXCOORD_ARRAY	2

layout(location = VERTEX_ARRAY) in highp vec4 inVertex;
layout(location = NORMAL_ARRAY) in mediump vec3 inNormal;
layout(location = TEXCOORD_ARRAY) in mediump vec2 inTexCoord;

layout (set = 1, binding = 0)uniform PerMesh
{
	highp mat4 MVPMatrix; // model view projection transformation
};
layout(location = 0) out mediump vec2 TexCoord;

layout(push_constant) uniform PushConstants
{
	vec3 planePosition;
} pc;


void main()
{
	// Adjust position based on the push constant
	highp vec4 adjustedVertex = inVertex + vec4(pc.planePosition, 0.0);

	// Transform position
	gl_Position = MVPMatrix * adjustedVertex;
	TexCoord = inTexCoord;
}
