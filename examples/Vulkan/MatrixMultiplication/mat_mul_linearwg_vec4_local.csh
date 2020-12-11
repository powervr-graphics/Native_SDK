/*!**********************************************************************
\File			mat_mul_linearwg_vec4_local.csh
\Title			Matrix multiplication using vec4 and local / shared memory
\Author			PowerVR by Imagination, Developer Technology Team.
\Copyright		Copyright(c) Imagination Technologies Limited.
\brief			This is the first benchmark which uses local or shared 
				memory. Matrices stay as vec4 arrays
************************************************************************/

/************************************************************************
* Versions and defines have been ommited from this file
* This is because matrix sizes need to be added into the
* source code during run time 
* A list of defines:
			Defined at runtime
					version - 450 
					WG_X_SIZE - Size of local x work group
					WG_Y_SIZE - Size of local y work group
					N,M,P     - Matrix sizes
			Defined in templated shader
					A  (MxN)
					B  (NxP)
					C  (MxP)
					
					AT (NxM)
					BT (PxN)
					CT (PxM)
					
					VecA  Vec4 version of the matrix A
					VecBT Vec4 version of the matrix BT
************************************************************************/

/*********************** shader  Explanantion **************************
Each workgroup reepresents a vertical slice down the product matrix 
The size of this slice is wg_linear_size, as a result wg_linear_size
must divice the height of the product matrix M
Further the reads made to the matrices are vec4, as a result n must
be divisible by 4, to make the indicies line up B has to be transposed
This reduces the number of internal loops when multiplying

local memory : An entire column of B, this is expected to do poorly for 
			   large N
Input : A, BT
Output: C
************************************************************************/
#define VECS_PER_ROW (N/4)

//since the work groups are a vertical segments of the product matrix
//each workgroup needs to add the correct column into the cache.
//A column of B is actually a row of BT to make the vectors line up.
shared vec4[VECS_PER_ROW] BColumnCache;

//The cache is shared between all of the invocations of that workgroup
//So instead of each thread in a workgroup filling the entire cache
//we can make each invocation fill a small segment of that cache then
//the entire cache will then be filled and visible to the whole workgroup
#define LOADS_PER_INV ((VECS_PER_ROW + WG_Y_SIZE - 1)/ WG_Y_SIZE)


void main()
{
	//Position in the product matrix
	uint x = gl_GlobalInvocationID.x; //0...P
	uint y = gl_GlobalInvocationID.y; //0...M
	//Local position in workgroup
	uint local = gl_LocalInvocationID.y; //0...linear_wg_size(WG_Y_SIZE)
	

	//Fill the cache (shared memory)
	//OpenCL has an asynchronous copy function to automatically fill shared memory, GLSL does not
	//instead we can simulate its function with a little bit of extra maths
	//the local invocation id * loads per invocation takes us to the segment of that column to be loaded 
	//x * vecs per row takes us to the correct row in BT to load for this column in the product matrix
	for(int i = 0; i < LOADS_PER_INV; i++)
	{
		BColumnCache[i + (local * LOADS_PER_INV)] = VecBT[(x * VECS_PER_ROW) + i + (local * LOADS_PER_INV)];
	}

	//wait for all threads to get this far and thus the cache is visible and filled after this barrier
	barrier();
	

	//Begin calculating the product for these cells.
	vec4 sum = vec4(0.0,0.0,0.0,0.0);
	for(int k = 0; k < N / 4; ++k)
	{
		sum += BColumnCache[k] *VecA[y * (N / 4) + k];

	}

	C[y][x] = sum.x + sum.y + sum.z + sum.w;
}