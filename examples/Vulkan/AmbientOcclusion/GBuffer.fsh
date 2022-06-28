#version 320 es

// Model albedo texture
layout(set = 0, binding = 1) uniform mediump sampler2D modelAlbedoTexture;

// Vertex shader inputs
layout(location = 0) in mediump vec2 vTexCoord;
layout(location = 1) in highp vec3 vNormal;

// fragment shader outputs
layout(location = 0) out highp vec4 outAlbedo;
layout(location = 1) out highp vec4 outWorldNormal;

void main()
{
	// Normalize the output normals, so that they can be used to calculate a TBN matrix later
	outWorldNormal = vec4(normalize(vNormal), 1.0);
	outAlbedo = texture(modelAlbedoTexture, vTexCoord);
}
