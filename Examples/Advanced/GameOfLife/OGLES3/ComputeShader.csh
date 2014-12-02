#version 310 es

/*********** INPUT DEFINES: THESE MUST BE SELECTED AND DEFINED BY THE PROGRAM ***********/
/*
IMAGE_BINDING_INPUT  : Image unit used for the texture that contains the current generation
IMAGE_BINDING_OUTPUT : Image unit used for the texture that contains the current generation
WG_WIDTH			 : Dimension X of the workgroup size
WG_HEIGHT			 : Dimension Y of the workgroup size
*/
/*********************************** END INPUT DEFINES **********************************/

//These defines determine local memory requirement ("cache" in this shader) depending on Workgroup size
#define CACHE_WIDTH (WG_WIDTH + 2)
#define CACHE_HEIGHT (WG_HEIGHT + 2)
#define CACHE_AREA (CACHE_WIDTH * CACHE_HEIGHT)

uniform layout(rgba8, binding = IMAGE_BINDING_INPUT) readonly highp image2D imageIn;
uniform layout(rgba8, binding = IMAGE_BINDING_OUTPUT) writeonly highp image2D imageOut;
shared float local_cache[CACHE_AREA+1]; //+1 so that we don't have to guard for a single out-of-bounds

/*
 *   -------------------------------------------- CACHING STRATEGY --------------------------------------------
 *   GLSL Compute shaders, DirectX compute shaders and OpenCL all define the concept of "shared" or "local" memory,
 *   which instructed to be fast, on-chip memory local to a workgroup and shared by all threads (work items et.c.,
 *   depending on terminology). 
 *   Much of the optimization of GPGPU programs that have a concept of reading data "around" their position in an
 *   input domain is finding the correct strategy to utilize local memory.
 *   In this case, each shader will need to read the input texture at own cell location and all neighbouring cells.
 *   That means that apart from the area of the grid that corresponds to our workgroup, we also need the border of that area.
 * 
 *   It is obvious that it is impossible to have all threads doing 100% the same amount of work, as our total domain
 *   (i.e. the part of the grid that is our workgroup plus its borders) is greater than AND not a multiple of our 
 *   workgroup size.
 *
 *   By examining the problem parameters (leaving the math as an exercise to the reader) we also see that, unless our 
 *   workgroup is less than 8x4 (Rogue warp size is 32, hence this would be the minimum as well!), the domain is 
 *   always less than twice the size of our workgroup.
 *   
 *   In that case, we will exploit that information to fetch either 1 or 2 texels per thread, by mapping our 2D local 
 *   coordinates to a 1D cache area (cache[]), and use each thread to fetch 2 texels, skipping the ones that are not needed.
 *   In order for that to work, each item does NOT fetch its own data from global memory (the texture), but the ones that 
 *   the mapping dictates by sequentially getting two items per thread, until we cover the entire sampling area.
 *   
 *   Workgroup area : height x width
 *   Cache area     : (height+2) x (width+2).
 *   Cache index 1 = local index y * group width + local index x
 *   Cache index 2 = Cache index 1 + 1
 *   Et cetera...
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
  * @Input grid   : The global 2D coordinates of the top-left (0,0) element of our workgroup (used to offset into the image)
  * @Input lid    : The local 2D coordinates of our work-item
  * @Input bounds : The maximum size of the image
  * Will fetch a sample into the shared workgroup cache, based on the local id of the item. 
  * ************************************************************************************************************************/
void prefetch_texture_samples(ivec2 grid, ivec2 lid, int cache_id, ivec2 bounds)
{
	if (cache_id < CACHE_AREA)
	{
		//Map the cache index to the actual global coordinates that must be fetched
		const ivec2 c_01 = ivec2(0,1);
		ivec2 cache_ids = ivec2(cache_id, cache_id) + ivec2(0,1);

		ivec2 idByCacheWidth = cache_ids / CACHE_WIDTH;

		ivec4 offset = ivec4(cache_ids - idByCacheWidth * CACHE_WIDTH - 1, idByCacheWidth - 1);

		ivec4 coords = grid.xxyy + offset;
		local_cache[cache_ids.x] = imageLoad(imageIn, coords.xz).x;
		local_cache[cache_ids.y] = imageLoad(imageIn, coords.yw).x;
	}
}


//In general, if we can use #defines that are passed to the shader at compile time we can achieve far better 
//flexibility to optimize for a specific platform
layout (local_size_x = WG_WIDTH, local_size_y = WG_HEIGHT) in;
void main()
{
	ivec2 sz = imageSize(imageIn);
	ivec2 gid = ivec2(gl_GlobalInvocationID.xy);
	ivec2 lid = ivec2(gl_LocalInvocationID.xy);
	ivec2 grid = ivec2(gl_WorkGroupID.xy * gl_WorkGroupSize.xy); //Global coords of the top left corner of our WG in order to offset to it
	
	int baseid = (lid.y * WG_WIDTH + lid.x) * 2;
	
	//Get items from global memory.
	prefetch_texture_samples(grid, lid, baseid, sz);
	//No need to sync yet - no two threads they do not touch the same values.
	//prefetch_texture_sample(grid, lid, baseid+1, sz);

	//CAUTION --- We need explicit, full execution synchronization here  - barrier() - in order to ensure that all threads have actually reached
	//this execution point. A simple memoryBarrier...() would not guarrantee that all threads in the group have actually executed - only that 
	//what HAS executed is visible to the rest of the workgroup.
	barrier();

	//CAUTION --- Bounds checking (for domain that is not divisible by our WG size) must also be done AFTER fetching for the cache : with our 
	//prefetching algorithm, inactive threads may still be responsible for fetching data.
	if (gid.x >= sz.x || gid.y >= sz.y) { return; }
	
	//Sum up the amount of neighbours for the thread. Everything is now loaded into shared memory.
	//For our cached area, Index 0 is the item that in local coords would be (-1,-1) (that is, the item upwards and leftwards of our first, (0,0) item)
	//and the rest follows suite keeping in mind that the local cache's width also has the borders (workgroup width + 2). 
	//We are vectorising and reusing ALU ops as much as possible.
	
	const ivec3 c_012 = ivec3(0,1,2);
	ivec3 idx_y = (lid.yyy + c_012) * ivec3(CACHE_WIDTH,CACHE_WIDTH,CACHE_WIDTH);
	ivec3 idx_x = lid.xxx + c_012;

	float nNeighbours	= local_cache[idx_y.x + idx_x.x]	+	local_cache[idx_y.x + idx_x.y]	+ local_cache[idx_y.x + idx_x.z]
						+ local_cache[idx_y.y + idx_x.x]          /* omit myself */				+ local_cache[idx_y.y + idx_x.z]
						+ local_cache[idx_y.z + idx_x.x]	+	local_cache[idx_y.z + idx_x.y]	+ local_cache[idx_y.z + idx_x.z];
	float self = local_cache[idx_y.y + idx_x.y];

	//*********************** The deceptively simple Conway's Game of Life algorithm ***********************//
	//ALGORITHM (UNOPTIMISED) CODE
	//For an empty cell...
	//float nextState = 0.0;
	//if (self < 1.)	
	//{
	//	//... be born if there are exactly three neighbours.
	//	if (nNeighbours == 3.0f) { nextState = 1.; }
	//}
	////... while, for a live cell...
	//else
	//{
	//	//... die, from loneliness or overcrowding if there are less than two or more than three neighbours
	//	if((nNeighbours < 2.0f) || (nNeighbours > 3.0f))
	//		nextState = 0.;
	//	//... otherwise, stay alive
	//	else
	//		nextState = 1.;			
	//}

	//OPTIMISED Expression : the slightly faster following expression (eliminate 3 branches in favor of a 
	//mix and two convert to float).
	float nextState = mix(float(nNeighbours == 3.), float(nNeighbours >= 2.0f && nNeighbours <= 3.0f), self);
	//This works thus : "Mix" can be used as a mask with values that are either bool, or 0/1 by choosing between them.
	//Hence, the "mix" instruction is the outer if.
	//The inner instructions could not be optimised further, but boolean converts to float 0/1 which suits us just fine.

	//Store red into the output image.
	imageStore(imageOut, gid, vec4(nextState,0.,0.,1.));

	//CAUTION : Image Store is *INCOHERENT* (see opengl memory model) - it will need to be explicitly synchronized 
	//through the OpenGL API with glMemoryBarrier(...) with the appropriate barrier bits set before the written 
	//memory is used in another shader or other use. For this example (in which it must be fed back as Image variable,
	//GL_SHADER_IMAGE_ACCESS_BARRIER_BIT will need to be used.
}
