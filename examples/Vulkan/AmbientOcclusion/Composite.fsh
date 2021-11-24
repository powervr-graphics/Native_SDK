#version 320 es

layout(set = 0, binding = 0) uniform mediump sampler2D Albedo;
layout(input_attachment_index = 0, set = 0, binding = 1) uniform mediump subpassInput aoAttachment;

// Uniform buffer to set how the final image will be composited
layout(set = 1, binding = 0) uniform CompositeSettings
{
	lowp float AlbedoStrength;
	lowp float AOStrength;
};

layout(location = 0) in highp vec2 vTexCoord;
layout(location = 0) out highp vec4 outColor;
void main()
{
	mediump vec3 albedoInput = texture(Albedo, vTexCoord).xyz;
	highp float aoInput = subpassLoad(aoAttachment).r;

	// If AOStrength is switched on apply the AO lighting to the albedo attachment, if not use a static lighting value
	// Do this using a branchless method
	mediump vec3 color = albedoInput * ((aoInput * AOStrength) + (1.0 - AOStrength));

	// If the AlbedoStrength is turned off then the only thing to display is the AO texture
	color = (color * AlbedoStrength) + ((1.0 - AlbedoStrength) * aoInput);

	outColor = vec4(color, 1.0);
}
