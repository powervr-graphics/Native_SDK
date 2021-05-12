/*!**********************************************************************
\File			mat_mul_naive_AT.csh
\Title			Naive Matrix Multiplication A transposed
\Author			PowerVR by Imagination, Developer Technology Team.
\Copyright		Copyright(c) Imagination Technologies Limited.
\brief			Absolutley no optimisations, the most straight forward
				matrix calcultion, A is uploaded as transpose to see how
				the memory layout affects speed
************************************************************************/

/************************************************************************
* Versions and defines have been ommited from this file
* This is because matrix sizes need to be added into the
* source code during run time
* A list of defines:
			Defined at runtime
					version - 320 es
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
************************************************************************/

/*********************** shader  Explanantion **************************
This is a naive implementation of matrix multiplication
There is no no optimisations, each incation just straight forwardly
calculates a cell in the product matrix. A is entered as transposed to
trial different memory layouts

No local memory
Input AT B
Output C
************************************************************************/
void main()
{
	uint x = gl_GlobalInvocationID.x;
	uint y = gl_GlobalInvocationID.y;
	float sum = 0.0;
	for (int k = 0; k < N; ++k) { sum += AT[k][y] * B[k][x]; }
	C[y][x] = sum;
}
