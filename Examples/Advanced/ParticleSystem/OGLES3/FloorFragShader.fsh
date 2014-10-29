#version 300 es

in highp vec3 vNormal;
in highp vec3 vLightDirection;

out lowp vec4 fragColor;

void main()
{
	highp float inv_lightdist = 1.0f / length(vLightDirection);
	highp float diffuse = max(dot(normalize(vNormal), vLightDirection * inv_lightdist), 0.0);
		
	fragColor = vec4(vec3(diffuse), 1.0);
}
