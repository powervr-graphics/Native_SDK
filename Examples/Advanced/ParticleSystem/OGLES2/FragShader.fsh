varying highp vec3 vNormal;
varying highp vec3 vLightDirection;

void main()
{
	highp float inv_lightdist = 1.0 / length(vLightDirection);
	highp float diffuse = max(dot(normalize(vNormal), vLightDirection * inv_lightdist), 0.0);
	gl_FragColor = vec4(vec3(diffuse) * inv_lightdist * 10.0, 1.0);
}
