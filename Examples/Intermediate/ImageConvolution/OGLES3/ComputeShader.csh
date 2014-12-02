#version 310 es

/*********** INPUT DEFINES: THESE MUST BE SELECTED AND DEFINED BY THE PROGRAM ***********/
/*
IMAGE_BINDING_INPUT		: Image unit used for the texture that contains the current generation
IMAGE_BINDING_OUTPUT	: Image unit used for the texture that contains the current generation
WG_WIDTH				: Dimension X of the workgroup size
WG_HEIGHT				: Dimension Y of the workgroup size
FILTER_DISTANCE			: Distance from our point that the farthest pixel that will need to be
						  taken into consideration will be, in one direction.
						  I.e for needing no pixels but the target, distance is 0 (degenerate case -
						  no convolution). For all neighbouring, it is 1 (for a total of 9 pixels - 3x3). 
						  For 2 neighbouring pixels, it is 2 (for a total of 25 pixels in 2D).
*/

/* To hardcode, uncomment the following lines and change values as needed.*/
//#define IMAGE_BINDING_INPUT 0
//#define IMAGE_BINDING_OUTPUT 1
//#define WG_WIDTH 16
//#define WG_HEIGHT 16
//#define FILTER_RADIUS 2
/*********************************** END INPUT DEFINES **********************************/


#define CACHE_WIDTH (WG_WIDTH + (FILTER_RADIUS - 1) * 2)
#define CACHE_HEIGHT (WG_HEIGHT + (FILTER_RADIUS - 1) * 2)
#define CACHE_AREA (CACHE_WIDTH * CACHE_HEIGHT)




uniform layout(rgba8, binding = IMAGE_BINDING_INPUT) readonly highp image2D imageIn;
uniform layout(rgba8, binding = IMAGE_BINDING_OUTPUT) writeonly highp image2D imageOut;
const lowp vec3 rgb_to_luminance = vec3(0.3f, 0.59f, 0.11f);

shared lowp vec3 colours[CACHE_AREA];
#if (defined ERODE || defined DILATE)
shared lowp float luminances[CACHE_AREA];
#endif

layout (local_size_x = WG_WIDTH, local_size_y = WG_HEIGHT) in;

/*
 *   -------------------------------------------- CACHING STRATEGY --------------------------------------------
 *   GLSL Compute shaders, DirectX compute shaders and OpenCL all define the concept of "shared" or "local" memory,
 *   which instructed to be fast, on-chip memory local to a workgroup and shared by all threads (work items et.c.,
 *   depending on terminology). 
 *   Much of the optimization of GPGPU programs that have a concept of reading data "around" their position in an
 *   input domain is finding the correct strategy to utilize local memory.
 *   In this case, each shader will need to read the input texture at own cell location and all neighbouring cells.
 *   That means that apart from the 1-1 area of the grid that is our workgroup, we also need the border of that area.
 * 
 *   It is obvious that it is impossible to have all threads doing 100% the same amount of work, as our total domain
 *   (i.e. the part of the grid that is our workgroup plus its borders) is greater than AND not a multiple of our 
 *   workgroup size.
 *
 *   By examining the problem parameters (leaving the math as an exercise to the reader) we also see that, unless our 
 *   workgroup is less than 8x4 (which is small enough to be undesirable), the domain is always less than twice the 
 *   size of our workgroup.
 *   
 *   In that case, we will exploit that information to avoid any loops and fetch either 1 or 2 texels per thread, 
 *   by mapping our 2D local coordinates to a 1D cache area (cache[]), and use each thread to fetch 2 texels, skipping the 
 *   ones that are not needed.
 *   In order for that to work, each item does NOT fetch its OWN data from global memory (the texture), but the ones that 
 *   the mapping dictates by sequentially getting two items per thread, until we cover the entire sampling area.
 *   
 *   Using the algorithm below, the items will be fetched in the following order (example :4x8 kernel => 6x10 cache) 
 *   (0,0), (0,0), (0,1), (0,1), (0,2), (0,2), 
 *   (0,3), (0,3), (1,0), (1,0), (1,1), (1,1), 
 *   (1,2), (1,2), (1,3), (1,3), (2,0), (2,0),
 *   (2,1), (2,1), (2,2), (2,2), (2,3), (2,3),
 *   (3,0), (3,0), (3,1), (3,1), (3,2), (3,2),
 *   (3,3), (3,3), (4,0), (4,0), (4,1), (4,1),
 *   (4,2), (4,2), (4,3), (4,3), (5,0), (5,0),
 *   (5,1), (5,1), (5,2), (5,2), (5,3), (5,3),
 *   (6,0), (6,0), (6,1), (6,1), (6,2), (6,2),
 *   (6,3), (6,3), (7,0), (7,0), (7,1), (7,1) DONE
 *   THE FOLLOWING THREADS ARE SKIPPED BY THE CACHE INDEX CHECK:
 *   (7,2), (7,2), (7,3), (7,3)
 *
 *   NOTE : This strategy spreads the texture fetching optimally among threads, but introduces a few extra ALU ops. Depending 
 *   on a many factors, in case for example ALU instructions were a bottleneck, another strategy might be more suitable, for
 *   example making indexing simpler with fewer ops but paying the cost of less well distributed texture fetches.
 */
 
 /* ************************************************************************************************************************
  * @Function prefetch_texture_samples
  * @Input groupid  : The global 2D coordinates of the top-left (0,0) element of our workgroup (used to offset into the image)
  * @Input lid      : The local 2D coordinates of our work-item
  * @Input cache_id : The 1D id into our local cache
  * @Input bounds   : The maximum size of the image
  * ************************************************************************************************************************/
void prefetch_item(ivec2 groupid, ivec2 lid, int cache_id, ivec2 bounds)
{
	//Cacge
	if (cache_id < CACHE_AREA)
	{
		//idByCacheWidth is the actual y coordinate of the top - left
		int idByCacheWidth = cache_id / CACHE_WIDTH;
		ivec2 global_offset = ivec2(cache_id - idByCacheWidth * CACHE_WIDTH - 1, idByCacheWidth - 1);
		ivec2 global_coords = groupid + global_offset;
		
		//CLAMP-TO-EDGE
		global_coords = clamp(global_coords, ivec2(0,0), bounds-ivec2(1,1));
		
		colours[cache_id] = imageLoad(imageIn, global_coords).xyz;
#if (defined ERODE || defined DILATE)
		luminances[cache_id] = dot(rgb_to_luminance, colours[cache_id]);  
#endif
	}
}

/*
 * Picks the highest luminance value in a 3x3 surrounding and 
 * writes the corresping colour as result.
 */
void main()
{

//***************** BEGIN COMMON CODE *****************//
// All the filters will do basically the same steps:

//Step 1: Get globals and values we will need
	ivec2 image_bounds = imageSize(imageIn);
	ivec2 global_id = ivec2(gl_GlobalInvocationID.xy);
	ivec2 local_id = ivec2(gl_LocalInvocationID.xy); 
	//Global coords of the top left corner of our WG in order to offset to it
	ivec2 group_coords = ivec2(gl_WorkGroupID.xy * gl_WorkGroupSize.xy); 
	
	//We will use this for the caching ids.
	int cache_id = (local_id.y * WG_WIDTH + local_id.x) * 2;

//Step 2: Prefetch all needed texels into the fast shared local memory
	//Cache two consecutive values per kernel
	prefetch_item(group_coords, local_id, cache_id, image_bounds);
	prefetch_item(group_coords, local_id, cache_id + 1, image_bounds);
	
//Step 3: SYNCHRONIZE so that the values in the caches are visible to all threads
	barrier();
	
	vec3 result_colour;

//Step 4: Actually execute the convolutions
//***************** END COMMON CODE *****************//

#ifdef DILATE
//********************** DILATE **********************//
/* Writes to the current pixel tha color of the neighbouring
 * pixel with the HIGHEST luminance*/

	//Find the max of these values
	int max_idx = local_id.y * CACHE_WIDTH + local_id.x;										//(0,0)
	int idx = max_idx + 1;																		//(0,1)
							max_idx = luminances[idx] > luminances[max_idx] ? idx : max_idx;
	idx += 1;				max_idx = luminances[idx] > luminances[max_idx] ? idx : max_idx;	//(0,2)
	idx += CACHE_WIDTH;		max_idx = luminances[idx] > luminances[max_idx] ? idx : max_idx; 	//(1,2)
	idx -= 1;				max_idx = luminances[idx] > luminances[max_idx] ? idx : max_idx;	//(1,1)
	idx -= 1;				max_idx = luminances[idx] > luminances[max_idx] ? idx : max_idx;	//(1,0)
	idx += CACHE_WIDTH;		max_idx = luminances[idx] > luminances[max_idx] ? idx : max_idx;	//(2,0)
	idx += 1;				max_idx = luminances[idx] > luminances[max_idx] ? idx : max_idx;	//(2,1)
	idx += 1;				max_idx = luminances[idx] > luminances[max_idx] ? idx : max_idx;	//(2,2)

	result_colour = colours[max_idx];
#endif //DILATE

#ifdef ERODE
//********************** ERODE **********************//
/* Writes to the current pixel tha color of the neighbouring
 * pixel with the LOWEST luminance*/

	//Find the min of these values
	int min_idx = local_id.y * CACHE_WIDTH + local_id.x;									//(0,0)
	int idx = min_idx + 1;																	//(0,1)
	min_idx = luminances[idx] < luminances[min_idx] ? idx : min_idx;
	idx += 1;				min_idx = luminances[idx] < luminances[min_idx] ? idx : min_idx;  //(0,2)
	idx += CACHE_WIDTH;		min_idx = luminances[idx] < luminances[min_idx] ? idx : min_idx; 	//(1,2)
	idx -= 1;				min_idx = luminances[idx] < luminances[min_idx] ? idx : min_idx;	//(1,1)
	idx -= 1;				min_idx = luminances[idx] < luminances[min_idx] ? idx : min_idx;	//(1,0)
	idx += CACHE_WIDTH;		min_idx = luminances[idx] < luminances[min_idx] ? idx : min_idx;	//(2,0)
	idx += 1;				min_idx = luminances[idx] < luminances[min_idx] ? idx : min_idx;	//(2,1)
	idx += 1;				min_idx = luminances[idx] < luminances[min_idx] ? idx : min_idx;	//(2,2)

	result_colour = colours[min_idx];
#endif //ERODE

#if (defined GRADIENT_LAPLACE || defined EDGEDETECT_LAPLACE)
//********************** LAPLACIAN EDGE DETECTION **********************//
/* This is a simplistic convolution kernel for edge detection. It is not
 * frequently used as it is extremely sensitive to noise.
 * Edge detection convolution typically works by calculating the gradient
 * of an image in "some" direction, in that case the horizontal, vertical
 * AND diagonal at the same time. Note, that the sum of all weights is zero,
 * and that we are, effectively, calculating gradients.
 */

	lowp float w[] = 
	float[](- .7071, -1.    , - .7071,
	  -1.    ,  6.8284, -1.    ,
	  - .7071, -1.    , - .7071);
	//Normally we need to multiply all neighbouring pixels with the widths, but we of course can omit zeros so we are a little less verbose
	int idx = (local_id.y + 1) * CACHE_WIDTH + local_id.x + 1;	

	result_colour =	 w[0]*colours[idx - CACHE_WIDTH -1] + w[1]*colours[idx - CACHE_WIDTH] + w[2]*colours[idx - CACHE_WIDTH +1];
	result_colour += w[3]*colours[idx               -1]	+ w[4]*colours[idx              ] + w[5]*colours[idx               +1];
	result_colour += w[6]*colours[idx + CACHE_WIDTH -1] + w[7]*colours[idx + CACHE_WIDTH] + w[8]*colours[idx + CACHE_WIDTH +1];
	result_colour = abs(result_colour);
#endif //EDGEDETECT_LAPLACE

#if (defined GRADIENT_SOBEL || defined EDGEDETECT_SOBEL)
//********************** SOBEL EDGE DETECTION **********************//
/* This is a convolution kernel that is frequently used for edge detection. 
 * It is not as sensitive to noise, and is frequently used modified or unmodified
 * in actual production code, though usually the results are post-processed for
 * the actual application (clamped, enhanced et.c.) to detect actual edges
 * There are Sobel convolution kernels to detects edges in any direction facing 
 * a "compass point" (by rotating the values clockwise or counterclockwise). The
 * most common use, though, is by detecting the x and y ones. We also do not
 * need the "reverse" passes, as we can get the absolute values, in which case 
 * obviously both left - to - right and right - to - left gradients are detected.
 * The formula G=sqrt(Gx^2+Gy^2) is fine for most practical purposes.
 */

	lowp float  sx[] = float[]
	(-1., 0.,  1.,
	 -2., 0.,  2.,
	 -1., 0.,  1.);
	lowp float  sy[] = float[]
	(-1.,-2.,-1.,
	  0., 0., 0.,
	  1., 2., 1.);

	//Sx calculates the horizontal gradient, Sy the vertical gradient. The actual total gradient G = sqrt(gx^2 + gy^2)
	int idx = (local_id.y + 1) * CACHE_WIDTH + local_id.x + 1;																//(0,1)

	lowp vec3 gx;
	gx  = colours[idx - CACHE_WIDTH - 1] * sx[0] + /*colours[idx - CACHE_WIDTH] * sx[1]*/ + colours[idx - CACHE_WIDTH + 1] * sx[2];
	gx += colours[idx               - 1] * sx[3] + /*colours[idx              ] * sx[4]*/ + colours[idx               + 1] * sx[5];
	gx += colours[idx + CACHE_WIDTH - 1] * sx[6] + /*colours[idx + CACHE_WIDTH] * sx[7]*/ + colours[idx + CACHE_WIDTH + 1] * sx[8];

	// vertical filter
	lowp vec3 gy;
	  gy  = colours[idx - CACHE_WIDTH - 1] * sx[0] + colours[idx - CACHE_WIDTH] * sx[1] + colours[idx - CACHE_WIDTH + 1] * sx[2];
	/*gy += colours[idx               - 1] * sx[3] + colours[idx              ] * sx[4] + colours[idx               + 1] * sx[5];*/
	  gy += colours[idx + CACHE_WIDTH - 1] * sx[6] + colours[idx + CACHE_WIDTH] * sx[7] + colours[idx + CACHE_WIDTH + 1] * sx[8];

	result_colour = sqrt(gx*gx + gy*gy);
#endif //EDGEDETECT_SOBEL

//#if (defined EDGEDETECT_SOBEL||defined EDGEDETECT_LAPLACE)
//////In order to actually DETECT the edges in a "boolean" fashion, we take the magnitude of the color and threshold it
//lowp float s = clamp(dot(result_colour, rgb_to_luminance)-.25 * .5);
//result_colour = vec3(s,s,s);
//#endif

#ifdef GAUSSIAN
//**********************------------ GAUSSIAN BLUR -------------**********************//
/* This is a convolution kernel that performs a gaussian blur FOR ILLUSTRATION PURPOSES 
 * ONLY. It is very rarely used in this form, or as a multipass in general, because: the 
 * 1) It is separable, meaning that it can be composed by 2 1D passes, one horizontal and
 * one vertical. As the 2D pass requires the SQUARE of the samples that the 1D needs, it
 * is much, much more preferable, especially for kernels larger than the 3x3 we use here 
 * 2) Multiple passes are identical as doing one pass with a larger radius. It should be
 * obvious that instead of doing a radius 4 pass that will require 7x7 = 49 samples per 
 * texel, it is quite preferable to do two passes, requiring a total of 7+7 = 14 samples
 * per texel...
 */

	lowp float  g[] = float[]
	(0.0625, 0.1250,  0.0625,
	 0.1250, 0.2500,  0.1250,
	 0.0625, 0.1250,  0.0625);

	//Sx calculates the horizontal gradient, Sy the vertical gradient. The actual total gradient G = sqrt(gx^2 + gy^2)
	int idx = (local_id.y + 1) * CACHE_WIDTH + local_id.x +1;
	result_colour  = colours[idx - CACHE_WIDTH - 1] * g[0] + colours[idx - CACHE_WIDTH] * g[1] + colours[idx - CACHE_WIDTH + 1] * g[2];
	result_colour += colours[idx               - 1] * g[3] + colours[idx              ] * g[4] + colours[idx               + 1] * g[5];
	result_colour += colours[idx + CACHE_WIDTH - 1] * g[6] + colours[idx + CACHE_WIDTH] * g[7] + colours[idx + CACHE_WIDTH + 1] * g[8];
#endif //GAUSSIAN

#ifdef EMBOSS
//**********************------------ EMBOSS -------------**********************//
/* Emboss effect */

	lowp float  g[] = float[]
	(-2, -1,  0,
	 -1, 1,  1,
	  0, 1,  2);

	//Sx calculates the horizontal gradient, Sy the vertical gradient. The actual total gradient G = sqrt(gx^2 + gy^2)
	int idx = (local_id.y + 1) * CACHE_WIDTH + local_id.x +1;
	result_colour  = colours[idx - CACHE_WIDTH - 1] * g[0] + colours[idx - CACHE_WIDTH] * g[1] + colours[idx - CACHE_WIDTH + 1] * g[2];
	result_colour += colours[idx               - 1] * g[3] + colours[idx              ] * g[4] + colours[idx               + 1] * g[5];
	result_colour += colours[idx + CACHE_WIDTH - 1] * g[6] + colours[idx + CACHE_WIDTH] * g[7] + colours[idx + CACHE_WIDTH + 1] * g[8];
#endif //EMBOSS

#ifdef SHARPEN
//**********************------------ SHARPEN -------------**********************//
/* Sharpen effect */

	lowp float  g[] = float[]
	(-.2,-.28284,-.2,
	  -.28284, 2.931, -.28284,
	 -.2,-.28284,-.2);

	//Sx calculates the horizontal gradient, Sy the vertical gradient. The actual total gradient G = sqrt(gx^2 + gy^2)
	int idx = (local_id.y + 1) * CACHE_WIDTH + local_id.x +1;
	result_colour  = colours[idx - CACHE_WIDTH - 1] * g[0] + colours[idx - CACHE_WIDTH] * g[1] + colours[idx - CACHE_WIDTH + 1] * g[2];
	result_colour += colours[idx               - 1] * g[3] + colours[idx              ] * g[4] + colours[idx               + 1] * g[5];
	result_colour += colours[idx + CACHE_WIDTH - 1] * g[6] + colours[idx + CACHE_WIDTH] * g[7] + colours[idx + CACHE_WIDTH + 1] * g[8];
#endif //SHARPEN

	imageStore(imageOut, global_id, vec4(result_colour,1.));
}
