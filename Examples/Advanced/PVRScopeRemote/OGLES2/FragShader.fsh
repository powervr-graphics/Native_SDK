uniform sampler2D  sThicknessTex;

uniform highp float  MinThickness;
uniform highp float  MaxVariation;

varying mediump float  CosViewAngle;
varying mediump float  LightIntensity;
varying mediump vec2   TexCoord;

// We use wave numbers (k) for the iridescence effect, given as
//   k =  2 * pi / wavelength in nm.
const highp float  PI = 3.141592654;
const highp vec3   cRgbK = 2.0 * PI * vec3(1.0/475.0, 1.0/510.0, 1.0/650.0);

void main()
{
	highp float thickness = texture2D(sThicknessTex, TexCoord).r * MaxVariation + MinThickness;
	highp float delta = (thickness / LightIntensity) + (thickness / CosViewAngle);
	lowp vec3 color = cos(delta * cRgbK) * LightIntensity;
	gl_FragColor = vec4(color, 1.0);
}