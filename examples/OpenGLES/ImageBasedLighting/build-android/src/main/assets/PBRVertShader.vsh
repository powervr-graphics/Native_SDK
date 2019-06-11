#version 310 es
layout(location = 0)in highp   vec3  inVertex;
layout(location = 1)in highp vec3  inNormal;
layout(location = 2)in mediump vec2  inTexCoord;
layout(location = 3)in highp vec4  inTangent;

layout(std140, binding = 1) uniform UboDynamic
{
	highp vec3 camPos;
	highp mat4 VPMatrix;
} uboDynamic;

layout(std140, binding = 2) uniform UboPerModel
{
	highp mat4 ModelMatrix;
	mediump float emissiveScale;
} uboPerModel;

#ifndef INSTANCING
layout(location = 0) uniform mediump vec2 metallicityRoughness;
layout(location = 1) uniform mediump vec3 rgb;
#endif

layout(location = 0) out highp vec3 outWorldPos;
layout(location = 1) out mediump vec3 outNormal;
layout(location = 2) out mediump vec2 TexCoord;
layout(location = 3) out mediump vec2 outMetallicRoughness;
layout(location = 4) out mediump vec3 outRgb;
layout(location = 5) out mediump vec3 outTangent;
layout(location = 6) out mediump vec3 outBitTangent;

#ifdef INSTANCING
// https://seblagarde.wordpress.com/2011/08/17/feeding-a-physical-based-lighting-mode/
const vec3 MaterialRGB[] = vec3[]
(
	vec3(0.951519, 0.959915, 0.955324), // silver
	vec3(1.0f,      0.765557, 0.336057), // Colored metal
	vec3(0.75f,     0.75f,      0.75f),     // plastic
	vec3(0.01f,    0.05f,    0.2f)      // Colored plastic
);

// r: metallic
// g: roughness/+

const vec2 MetallicRoughness[] = vec2[]
(
// METALLIC
vec2(1.0, 0.00),
vec2(1.0, 0.15),
vec2(1.0, 0.25),
vec2(1.0, 0.35),
vec2(1.0, 0.60),
vec2(1.0, 0.90),
            
vec2(1.0, 0.00),
vec2(1.0, 0.15),
vec2(1.0, 0.25),
vec2(1.0, 0.35),
vec2(1.0, 0.60),
vec2(1.0, 0.90),
            
// PLASTIC
vec2(0.0, 0.00),
vec2(0.0, 0.15),
vec2(0.0, 0.25),
vec2(0.0, 0.35),
vec2(0.0, 0.65),
vec2(0.0, 1.00),
            
vec2(0.0, 0.00),
vec2(0.0, 0.15),
vec2(0.0, 0.25),
vec2(0.0, 0.35),
vec2(0.0, 0.65),
vec2(0.0, 1.00)
);
#endif

void main()
{
	gl_Position = uboPerModel.ModelMatrix * vec4(inVertex, 1.0);
    
	// Sphere
#ifdef INSTANCING
	highp float x = -float(gl_InstanceID % 6) * 10.0 + 25.;
	highp float y = -float(gl_InstanceID / 6) * 10.0 + 15.;
	gl_Position.xy += vec2(x,y);
    
	// selects the rgb material base color. This is different for each row of the sphere.
	outMetallicRoughness = MetallicRoughness[gl_InstanceID];      
	outRgb =  MaterialRGB[0 + int(gl_InstanceID >= 6) + int(gl_InstanceID >= 12) + int(gl_InstanceID >= 18)];
    
#else // helmet
    
	outMetallicRoughness = metallicityRoughness;
	outRgb = rgb; 
	outTangent = inTangent.xyz;
	outBitTangent = cross(inNormal, inTangent.xyz) * inTangent.w;    
#endif
	outWorldPos = gl_Position.xyz;
	gl_Position = uboDynamic.VPMatrix * gl_Position;// mvp
    
	outNormal = normalize(transpose(inverse(mat3(uboPerModel.ModelMatrix))) * inNormal);
	TexCoord = inTexCoord;
}
