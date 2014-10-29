/*
	The vertex shader used for extruding the shadow volume along the light 
	direction. If inExtrude is > 0 then the vertex of the shadow volume is 
	extruded along the light direction by VolumeScale. If it is 0 then
	the vertex position is calculated as normal.
*/

attribute highp vec3   inVertex;
attribute lowp  float  inExtrude;

uniform highp   mat4   ModelViewProjMatrix;
uniform highp   vec3   LightDirection;
uniform mediump float  VolumeScale;

uniform lowp    vec4   FlatColour;
varying lowp    vec4   vColour;

void main()
{
/*
	mediump vec3 extrudedPos = inVertex + (VolumeScale * LightDirection) * inExtrude;
	gl_Position = ModelViewProjMatrix * vec4(extrudedPos, 1.0);
	*/
	
	vColour = FlatColour;

	if (inExtrude > 0.0)
	{
		mediump vec3 extrudedPos = inVertex + (VolumeScale * LightDirection);
		gl_Position = ModelViewProjMatrix * vec4(extrudedPos, 1.0);
	}
	else
	{
		gl_Position = ModelViewProjMatrix * vec4(inVertex, 1.0);
	}
}
