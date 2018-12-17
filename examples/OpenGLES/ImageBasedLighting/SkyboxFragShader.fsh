#version 310 es

layout (binding = 8) uniform samplerCube skyboxMap;
layout (location = 0) out mediump vec4 outColor;

layout(location = 0) in mediump vec3 inRayDir;

const mediump float HDR_SCALER = 100.0f;

const mediump float ExposureBias = 4.0;

void main()
{
	// Sample skybox cube map
	mediump vec3 RayDir = inRayDir;
    
	mediump vec4 color = texture(skyboxMap, RayDir);
	highp float tmpLight = color.a;
	tmpLight = HDR_SCALER * tmpLight * tmpLight + 1.0;
	color.rgb = color.rgb * tmpLight;
	
	// http://filmicworlds.com/blog/filmic-tonemapping-operators/
	// Reinhard tonemapping
	mediump vec3 ldrColor = color.rgb * ExposureBias;
	ldrColor = ldrColor / (1.0 + ldrColor);

#ifndef FRAMEBUFFER_SRGB
	ldrColor = pow(ldrColor, vec3(0.454545));
#endif
	outColor = vec4(ldrColor, 1.0);
}






