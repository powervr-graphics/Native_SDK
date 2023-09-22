#version 320 es

#define WG_WIDTH 8
#define WG_HEIGHT 4

const float textureResolution = 128.;

struct NoiseComputePushConstant
{
	float scale;
	int isFirstImage; // Using int instead of bool for better compatibility
	vec2 offset;
};

layout(push_constant) uniform PushConstants
{
	NoiseComputePushConstant data;
} pc;

uniform layout(binding = 0) sampler2D imageIn;
uniform layout(rgba8, binding = 1) writeonly highp image2D imageOut;

shared highp int cache[WG_HEIGHT + 2][WG_WIDTH + 2];

vec2 random2(vec2 st)
{
	st = vec2(dot(st, vec2(127.1, 311.7)),
	dot(st, vec2(269.5, 183.3)));
	return -1.0 + 2.0 * fract(sin(st) * 43758.5453123);
}

float noise(vec2 st)
{
	vec2 i = floor(st);
	vec2 f = fract(st);

	vec2 u = f * f * (3.0 - 2.0 * f);

	float a = dot(random2(i), f);
	float b = dot(random2(i + vec2(1.0, 0.0)), f - vec2(1.0, 0.0));
	float c = dot(random2(i + vec2(0.0, 1.0)), f - vec2(0.0, 1.0));
	float d = dot(random2(i + vec2(1.0, 1.0)), f - vec2(1.0, 1.0));

	return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

layout(local_size_x = WG_WIDTH, local_size_y = WG_HEIGHT) in;
void main()
{
	highp vec2 sz = vec2(textureSize(imageIn, 0));

	highp ivec2 gid = ivec2(gl_GlobalInvocationID.xy);

	highp vec2 uv = vec2(gid) / vec2(textureResolution);
	highp float n = (noise((uv + pc.data.offset) * pc.data.scale) + 1.) / 2.;
	highp vec4 color = vec4(vec3(n * 0.25), 1.0);

	//This condition can be determined before shader is executed, so multiple paths wont be executed
	//It only copies data from last texture, if this is not first texture
	if(pc.data.isFirstImage == 0)
	{
		highp vec4 inputColor = texture(imageIn, uv);
		color.rgb += inputColor.rgb;
	}

	imageStore(imageOut, gid, color);
}
