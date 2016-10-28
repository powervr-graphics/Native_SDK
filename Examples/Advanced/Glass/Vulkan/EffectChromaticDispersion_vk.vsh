#version 450
const lowp vec3 Eta = vec3(0.85, 0.87, 0.89);

layout(std140,set = 0, binding = 0)uniform Buffer
{
	uniform highp mat4 MVPMatrix;
	uniform mediump mat3 MMatrix;
	uniform mediump vec3 EyePos;
};

layout(location = 0)in highp vec3 inVertex;
layout(location = 1)in mediump vec3 inNormal;

layout(location = 0) out mediump vec3 RefractDirRed;
layout(location = 1) out mediump vec3 RefractDirGreen;
layout(location = 2) out mediump vec3 RefractDirBlue;

void main()
{
	// Transform position
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);
	
	// Calculate view direction in model space
	mediump vec3 ViewDir = normalize(inVertex - EyePos);

	// Refract view direction and transform to world space
	RefractDirRed = MMatrix * refract(ViewDir, inNormal, Eta.r);
	RefractDirGreen = MMatrix * refract(ViewDir, inNormal, Eta.g);
	RefractDirBlue = MMatrix * refract(ViewDir, inNormal, Eta.b);
}
