#version 310 es

layout(location = 0) in mediump vec2 vTexCoords;
layout(location = 0) out mediump vec4 oColor;

mediump uniform sampler2D sLUT2D;
mediump uniform float sliceIndex;
mediump uniform float lutSize;

void main()
{
	// convert normalized quad coordinates [0,1] into LUT grid coordinates [0, lutSize]
	mediump float r = vTexCoords.x * lutSize;
	mediump float g = vTexCoords.y * lutSize;
	mediump float b = sliceIndex;
	
	// x coordinate is taken from red axis in a blue tile
	mediump float x = (r + b * lutSize) / (lutSize * lutSize);
	// y coordinate corresponds directly to green axis
	mediump float y = g / lutSize;
	
	// map from the 2D texture to a vec3 color
	mediump vec3 color = texture(sLUT2D, vec2(x, y)).rgb;

	// converts from linear to sRGB
	color = pow(color, vec3(1.0 / 2.2));

	oColor = vec4(color, 1.0);
}
