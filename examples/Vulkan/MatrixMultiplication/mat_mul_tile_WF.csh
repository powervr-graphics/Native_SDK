/*!**********************************************************************
\File			mat_mul_tile_WF.csh
\Title			Matrix Multiplication using square tiles and a work factor
\Author			PowerVR by Imagination, Developer Technology Team.
\Copyright		Copyright(c) Imagination Technologies Limited.
\brief			The tiles in this example remain square but each invocation
				performs multiple products, as a result the workgroups are
				rectangular
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
This next shader is an evolution of the squared tiled one, there is
something else we can do to increase our speed, and that is to reduce
the number of reads to shared memory simultenously we increase the amount
of work done per thread.

The name of this example is maybe a tad misleading, we are not creating
rectangular tiled work groups, but instead taking thw tiled work groups
and increasing the number of cells in the product matrix that each local
invocation calculates, as a result the work groups do become rectangular.
But the purpose of this example is not to test the shape of the tiles
notice that the number of tiles actually remains the same.

The amount of extra work done per invocation will be called the WF
factor, meaning the work per thread factor. If WF = 1 we just get the
square tiling example.
************************************************************************/

// recover the tile size from the y group size, this remains the same
// because extra work per thread will be done in consecutive columns
#define TS WG_Y_SIZE
// number of tiles to be fetched from A and B per C tile.
#define NUM_OF_TILES N / TS
// The amount of extra work that each thread does is called the work factor
// we can recover the work factor by the difference between the X work group size
// and the Y work group size (WG_Y_SIZE = TS = WG_X_SIZE * WF)
#define WF TS / WG_X_SIZE

// declare the shared data
shared float[TS][TS] ACache;
shared float[TS][TS] BCache;

void main()
{
	int globalX = int(gl_GlobalInvocationID.x); // 0, ... , P/WF
	int globalY = int(gl_GlobalInvocationID.y);

	// This is the x,y coordinates of the work within this tile
	int localX = int(gl_LocalInvocationID.x);
	int localY = int(gl_LocalInvocationID.y);

	float[WF] accumulators;
	for (int i = 0; i < WF; ++i) { accumulators[i] = 0.0; }

	// for every tile for this tile in the product
	for (int t = 0; t < NUM_OF_TILES; ++t)
	{
		// load the cache
		// this needs to be done WF times
		for (int w = 0; w < WF; ++w)
		{
			ACache[localY][(localX * WF) + w] = A[globalY][(t * TS) + (localX * WF) + w];
			BCache[localY][(localX * WF) + w] = B[(t * TS) + localY][(globalX * WF) + w];
		}
		// wait for all threads to catch up
		barrier();

		// increment the accumulators for each tile
		for (int k = 0; k < TS; ++k)
		{
			for (int w = 0; w < WF; w++)
			{
				// Notice that the value being read from the A cache doesn't change
				accumulators[w] += ACache[localY][k] * BCache[k][(localX * WF) + w];
			}
		}
		barrier();
	}

	// place the accumulators into the product matrix CT
	for (int w = 0; w < WF; ++w) { C[globalY][(globalX * WF) + w] = accumulators[w]; }
}
