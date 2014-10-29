#version 310 es

#define NUM_BOXES         12

/****************************************************************************
Expected define inputs
NUM_SAMPLEPOINTS
WORKGROUPSIZE_X
WORKGROUPSIZE_Y
****************************************************************************/


/* 
 *  Sample distribution index that defines the amount of sample points 
 *  that reside within a bounding box.
 */
const int SAMPLE_DISTRIBUTION[64]= int[](

	2, 3, 2, 2, 2, 1, 2, 1, 3, 0, 2, 3, 0, 0, 2, 2,
	0, 3, 1, 1, 1, 2, 0, 2, 2, 2, 1, 1, 0, 2, 1, 2,		
	1, 3, 3, 1, 2, 1, 3, 3, 2, 0, 2, 1, 3, 2, 2, 2,
	2, 0, 0, 3, 2, 2, 2, 3, 0, 2, 2, 0, 2, 2, 1, 1
);

const vec2 consts=vec2(0.,1.);

shared vec2 randomPoints[NUM_SAMPLEPOINTS * NUM_BOXES];

uniform layout(rgba8, binding=0) writeonly highp image2D dstImage;
uniform float uniform_input_scale;

/* 
 *  Generates a set of random points based on the bounding box indices.
 */
void generatePseudoRandomSamplePoints(const ivec2 gid, int offset)
{	
	// Calculate initial seed value based on work group indices
	uint seed = uint(gid.x * 702395077 + gid.y * 915488749);	
	
	// Determine number of points the reside within the current bounding box.
	int numpoints = SAMPLE_DISTRIBUTION[seed >> 26];
	// Pre-calculate common values
	vec2 gidf = vec2(gid.x, gid.y);
	vec2 rangeXforms = vec2(WORKGROUPSIZE_X, WORKGROUPSIZE_Y);

	// Generate random points based on the seed
	for (int i=0; i < NUM_SAMPLEPOINTS; ++i)
	{
		
		randomPoints[i+offset] = vec2(-9999999.9f, -9999999.9f); 
		if (i<numpoints)
		{
			vec2 randvals;
			seed = 1402024253u * seed + 586950981u;
			randvals.x = float(seed) * 0.0000000002328306436538696f;
			seed = 1402024253u * seed + 586950981u;
			randvals.y = float(seed) * 0.0000000002328306436538696f;
		
			randomPoints[i+offset] = (gidf + randvals) * rangeXforms;
		}
	}
}


float distance_metric(vec2 a1, vec2 a2)
{
#if (defined Euclid)
	return distance(a1, a2);
#elif (defined Manhattan)
	return abs(a1.x - a2.x) + abs(a1.y - a2.y);
	//return dot(abs(a1 - a2), consts.yy));
#elif (defined Chessboard)
	return max(abs(a1.x - a2.x), abs(a1.y - a2.y));
	//max(dot(abs(a1 - a2), consts.yy));
#endif
}

#if (defined Euclid)
const vec4 scale = vec4(0.1677, 0.3354, 0.6708, 0.523);
#elif (defined Manhattan)
const vec4 scale = vec4(0.75f, 0.65f, 0.55f, 0.45f);
#elif (defined Chessboard)
const vec4 scale = vec4(0.4141f, 0.8282f, 0.9191f, 0.9999f);
#endif	


layout (local_size_x = WORKGROUPSIZE_X, local_size_y = WORKGROUPSIZE_Y) in;
void main()
{

	ivec2 gid = ivec2(gl_WorkGroupID.xy);
	ivec2 lid = ivec2(gl_LocalInvocationID.xy);
	vec2 curr_point_coords = vec2(gl_GlobalInvocationID.xy);

	if (lid.x < 3)
	{
		int offset = NUM_SAMPLEPOINTS * 4 * lid.x + NUM_SAMPLEPOINTS * lid.y;
		generatePseudoRandomSamplePoints(gid + ivec2(lid.y - 1, lid.x - 1), offset);
	}
	barrier();

	
	///* Calculate n nearest neighbours */
	float dist = distance_metric(randomPoints[0], curr_point_coords);
	vec4 min_distances = vec4(dist, dist, dist, dist);
		
	for (int i=1; i < NUM_BOXES * NUM_SAMPLEPOINTS; ++i)
	{
		dist = distance_metric(randomPoints[i], curr_point_coords);

		if (dist < min_distances.x)
		{
			min_distances.yzw = min_distances.xyz;
			min_distances.x = dist;
		}
		else if (dist < min_distances.y)
		{
			min_distances.zw = min_distances.yz;
			min_distances.y = dist;
		}
		else if (dist < min_distances.z)
		{
			min_distances.w = min_distances.z;
			min_distances.z = dist;
		}
		else if (dist < min_distances.w)
		{
			min_distances.w = dist;
		}		
	}
			
	/*
	 * Normalise and write back result
	 */	

	min_distances = min_distances * scale * uniform_input_scale;

	imageStore(dstImage, ivec2(gl_GlobalInvocationID.xy), min_distances);
}
