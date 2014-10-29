#version 300 es

uniform sampler2D  sThicknessTex;

uniform highp float  MinThickness;
uniform highp float  MaxVariation;

in mediump float CosViewAngle;
in mediump float LightIntensity;
in mediump vec2  TexCoord;

// We use wave numbers (k) for the iridescence effect, given as
//   k =  2 * pi / wavelength in nm.
const highp float  PI = 3.141592654;
const highp vec3   cRgbK = 2.0 * PI * vec3(1.0/475.0, 1.0/510.0, 1.0/650.0);

layout (location = 0) out lowp vec4 oColour;

void main()
{
	highp float thickness = texture(sThicknessTex, TexCoord).r * MaxVariation + MinThickness;
	highp float delta = (thickness / LightIntensity) + (thickness / CosViewAngle);
	lowp vec3 colour = cos(delta * cRgbK) * LightIntensity;
	oColour = vec4(colour, 1.0);
}