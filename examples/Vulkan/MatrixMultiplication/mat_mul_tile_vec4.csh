/*!**********************************************************************
\File			mat_mul_tile_vec4.csh
\Title			Matrix Multiplication with square tiles and vec4
\Author			PowerVR by Imagination, Developer Technology Team.
\Copyright		Copyright(c) Imagination Technologies Limited.
\brief			Matrix multiplication that stores the two input matrices
				as vec4s and uses the square tiling method
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
This shader is a squared tile example, excempt the inputs are vec4
this works on a similar principal as the rectangular examples, by
turning elemenets into vec4 we are reducing the number of reads into
local memory. as a result of the vec4 the width of the work groups
gets quatered and becomes rectangular as well
************************************************************************/

#define TILE_VEC_HEIGHT (WG_Y_SIZE)
#define TILE_VEC_WIDTH (TILE_VEC_HEIGHT / 4)
#define VECS_PER_LINE (N / 4)
#define TILE_NUM (VECS_PER_LINE / TILE_VEC_WIDTH)

shared vec4 ACache[TILE_VEC_HEIGHT][TILE_VEC_WIDTH];
shared vec4 BCache[TILE_VEC_HEIGHT][TILE_VEC_WIDTH];

void main()
{
	int globalX = int(gl_GlobalInvocationID.x);
	int globalY = int(gl_GlobalInvocationID.y);

	int localX = int(gl_LocalInvocationID.x) / 4;
	int localY = int(gl_LocalInvocationID.y);

	vec4 accumulator = vec4(0.0, 0.0, 0.0, 0.0);
	for (int t = 0; t < TILE_NUM; ++t)
	{
		// load a vec4 into the cache
		// go to right line, go to the right tile, then go to the right vector in that tile
		// --(globalY * VECS_PER_LINE) + (t * TILE_VEC_WIDTH) + localX
		ACache[localY][localX] = VecA[(globalY * VECS_PER_LINE) + (t * TILE_VEC_WIDTH) + localX];
		// BCache[localY][localX] = VecBT[(globalX * VECS_PER_LINE) + (t * TILE_VEC_WIDTH) + localY];
		//(go to the right tile row) + go to right tile column
		int TileStartIndex = (int(gl_WorkGroupID.x) * VECS_PER_LINE * TILE_VEC_HEIGHT) + (t * TILE_VEC_WIDTH);
		BCache[localY][localX] = VecBT[TileStartIndex + (localY * VECS_PER_LINE) + localX];

		// wait for the caches to fill
		barrier();

		// increment the accumulator
		for (int k = 0; k < TILE_VEC_WIDTH; ++k) { accumulator += ACache[localY][k] * BCache[gl_LocalInvocationID.x][k]; }
		// wait for this tile to finish being summed
		barrier();
	}
	// place the value into the matrix
	C[globalY][globalX] = accumulator.x + accumulator.y + accumulator.z + accumulator.w;
}
