attribute highp vec3  inPosition;
attribute highp float inLifespan;

uniform highp mat4  uModelViewProjectionMatrix;

varying mediump vec2 vTexCoord;

void main()
{
	gl_Position = uModelViewProjectionMatrix * vec4(inPosition, 1.0);
	gl_PointSize = 3.0;
	float scale = clamp(inLifespan / 20.0, 0.0, 1.0);
	scale = scale * scale;
	vTexCoord.st = vec2(scale, scale);
}
