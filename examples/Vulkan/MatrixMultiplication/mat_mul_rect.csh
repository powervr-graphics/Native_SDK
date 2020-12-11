/*!**********************************************************************
\File			mat_mul_rect.csh
\Title			Matrix multiplication with rectangular workgroups
\Author			PowerVR by Imagination, Developer Technology Team.
\Copyright		Copyright(c) Imagination Technologies Limited.
\brief			Produces matrix multiplication using a rectangular tiling
				method, the size of these tiles can be specified in the 
				M,N and P dimensions.
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
This is a generalisation of the square tiling method, at the moment 
each invocation produces one element in the product but the workgroups
change to be rectangular, but to get the tile size in the n dimension 
an extra bit of information needs to be defined, so a different header 
is placed on this file

local memory : Two caches which are tiles taken from matrices A and B
Input : A, B
Output: C
************************************************************************/
//reconstruct the tile sizes from the workgroup sizes
//for the demo matrix this is the best performing paramaters
//N_TILE DEFINED IN HEADER
#define M_TILE WG_Y_SIZE
#define P_TILE (WG_X_SIZE) 

//Tile number refers to how many tiles must be looped over in A and B to 
//get the result in one tile of C.
#define TILE_NUM (N / N_TILE)

//Since tiles dimensions wont line up, we will probably have to add extra 
//cells to the shared memory per invocation.
//A extra, since A's tile height matches C's tile height, this is how many
	//extra tiles need to be added horizontally to A for every cell in C
	//N_TILE / P_TILE rounded up
#define A_LOCAL_EXTRA ((N_TILE + P_TILE - 1) / P_TILE)
//The extra cells to add vertically to B local (N_TILE / M_FACTOR)
#define B_LOCAL_EXTRA ((N_TILE + M_TILE - 1) / M_TILE)

//The shared memory tiles
shared float[M_TILE][N_TILE] ACache;
shared float[N_TILE][P_TILE] BCache;
void main()
{
	float accumulator = 0;
	
	uint globalX = gl_GlobalInvocationID.x;
	uint globalY = gl_GlobalInvocationID.y;
	
	uint localX = gl_LocalInvocationID.x;
	uint localY = gl_LocalInvocationID.y;
	
	//for every tile in A and B which needs to be fetched
	for(int t = 0; t < TILE_NUM; ++t)
	{
		//load the cache including the extra horizontal ones for A and extra virtical ones for B
		for(int a = 0; a < A_LOCAL_EXTRA; ++a)
		{
			ACache[localY][(A_LOCAL_EXTRA * localX) + a] = A[globalY][(t * N_TILE) + (A_LOCAL_EXTRA * localX) + a];
		}
		for(int b = 0; b < B_LOCAL_EXTRA; ++b)
		{
			BCache[(B_LOCAL_EXTRA * localY) + b][localX] = B[(t * N_TILE) + (B_LOCAL_EXTRA * localY) + b][globalX];
		}

		//wait for all threads
		barrier();

		//incriment the accumulator for every element in the loaded tiles
		for(int k = 0; k < N_TILE; ++k)
		{
			accumulator += ACache[localY][k] * BCache[k][localX];
		}
		//wait for this computation to finish before we start editing the tile cache
		barrier();
	}
	
	//update the value in C
	C[globalY][globalX] = accumulator;
}