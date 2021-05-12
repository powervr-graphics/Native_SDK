/*!**********************************************************************
\File			mat_mul_tile.csh
\Title			Matrix multiplication with square tiling
\Author			PowerVR by Imagination, Developer Technology Team.
\Copyright		Copyright(c) Imagination Technologies Limited.
\brief			Matrix multiplication using square tiles, the workgroups
				are the same size as the tiles.
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
This is potentially the best performing algorithm for large desktop gpus,
it might not fair too well on mobile gpus.
Each workgroup workgroup represents a square tile of the product matrix.
Since the tiles taken as local memory from Matrices A and B are the same
size as the product tiles this implies the size of the tiles must divide
m,n and p. The tile size is extracted from the workgroup Y size, it will
be reasonably assumed that the workgroup sizes were correctly applied from
the CPU side.
local memory : Two caches which are tiles taken from matrices A and B
Input : A, B
Output: C
************************************************************************/

// define the Tile size for convinece
#define TS WG_Y_SIZE
// The number of tiles in A and B per tile in C
#define TILE_NUM N / TS
shared float[TS][TS] ACache;
shared float[TS][TS] BCache;
void main()
{
	float accumulator = 0.0;

	int globalX = int(gl_GlobalInvocationID.x);
	int globalY = int(gl_GlobalInvocationID.y);

	int localX = int(gl_LocalInvocationID.x);
	int localY = int(gl_LocalInvocationID.y);

	// for every tile in A and B which needs to be fetched
	for (int i = 0; i < TILE_NUM; ++i)
	{
		// load the cache
		ACache[localY][localX] = A[globalY][i * TS + localX];
		BCache[localY][localX] = B[i * TS + localY][globalX];
		// wait for all threads
		barrier();
		// incriment the accumulator for every element in the loaded tiles
		for (int k = 0; k < TS; ++k) { accumulator += ACache[localY][k] * BCache[k][localX]; }
		// wait for this computation to finish before we start editing the tile cache
		barrier();
	}
	// update the value in C
	C[globalY][globalX] = accumulator;
}
