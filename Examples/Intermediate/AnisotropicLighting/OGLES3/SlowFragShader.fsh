#version 300 es
in lowp vec3  DiffuseIntensity; 
in lowp vec3  SpecularIntensity;

const lowp vec3 cBaseColor = vec3(0.9, 0.1, 0.1); 
layout (location = 0) out lowp vec4 oColour;

void main() 
{ 
	oColour = vec4((cBaseColor * DiffuseIntensity) + SpecularIntensity, 1.0); 
}
