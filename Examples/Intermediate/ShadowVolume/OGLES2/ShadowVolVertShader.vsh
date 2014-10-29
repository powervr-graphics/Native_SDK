/*
	The vertex shader used for extruding the shadow volume along the light 
	direction. If inExtrude is > 0 then the vertex of the shadow volume is 
	extruded along the light direction by VolumeScale. If it is 0 then
	the vertex position is calculated as normal.
*/

attribute highp vec3   inVertex;
attribute lowp  float  inExtrude;

uniform highp   mat4   MVPMatrix;
uniform highp   vec3   LightPosModel;
uniform mediump float  VolumeScale;

void main()
{
	if (inExtrude > 0.0)
	{
		mediump vec3 lightDir = normalize(inVertex - LightPosModel);
		mediump vec3 extrudedPos = inVertex + (VolumeScale * lightDir);
		gl_Position = MVPMatrix * vec4(extrudedPos, 1.0);
	}
	else
	{
		gl_Position = MVPMatrix * vec4(inVertex, 1.0);
	}
}
