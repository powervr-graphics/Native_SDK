#version 320 es

precision highp float;
layout (set = 2, binding = 0) uniform mediump sampler mSampler;
layout (set = 2, binding = 1) uniform mediump texture2D sBaseTex[2];

layout(std140, set = 1, binding = 2) uniform LightConstants
{
	vec4 lightDir;
	vec4 lightCol;
}lightConstants;

layout(location = 0) in mediump vec2 TexCoord;
layout(location = 1) flat in mediump uint drawID; // gl_InstanceIndex is also an alternative to have TOTAL_NUM_INSTANCES distinct textures, check device limits.
layout(location = 2) in mediump vec3 Normal;
layout(location = 3) in mediump vec3 FragPos;

layout(location = 0) out mediump vec4 oColor;

void main()
{
	// Diffuse
     vec3 lightDir = normalize(lightConstants.lightDir.xyz - FragPos);
     float diffuseStrength = max(dot(Normal, lightDir), 0.0);
     vec3 diffuse = diffuseStrength * lightConstants.lightCol.xyz;

	// Specular ( Blinnn Phong )
     vec3 viewDir = normalize(-FragPos);
	 vec3 halfwayDir = normalize(lightDir + viewDir);
	 float specularStrength = max(dot(Normal, halfwayDir), 0.0);

     vec3 specular = lightConstants.lightCol.xyz * pow(specularStrength, 32.0);

	 vec3 result = 0.1 * lightConstants.lightCol.xyz + diffuse + specular;

	oColor.rgb = texture(sampler2D(sBaseTex[drawID], mSampler), TexCoord).rgb * result;
	oColor.a = 1.0;
}
