#version 320 es

layout(set = 0, binding = 0) uniform mediump samplerCube sSkybox;
layout(location = 0) in mediump vec3 RayDir;
layout(location = 0) out mediump vec4 outColor;

const mediump float HDR_SCALER = 500.f;
const highp  float ExposureBias = 4.0;

void main()
{
	mediump vec4 tmpColor = texture(sSkybox, RayDir);
	mediump float tmpLight = tmpColor.a;
	tmpLight = HDR_SCALER * tmpLight * tmpLight + 1.0;   
	tmpColor.rgb = tmpColor.rgb * tmpLight;

	// http://filmicworlds.com/blog/filmic-tonemapping-operators/
	// Reinhard tonemapping
	mediump vec3 ldrColor = tmpColor.rgb * ExposureBias;
	ldrColor = ldrColor / (1.0 + ldrColor);

	outColor = vec4(ldrColor, 1.0);
}
