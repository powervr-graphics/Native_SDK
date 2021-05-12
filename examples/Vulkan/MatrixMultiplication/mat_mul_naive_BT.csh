/*!**********************************************************************
\File			mat_mul_naive_BT.csh
\Title			Naive Matrix Multiplication B transposed
\Author			PowerVR by Imagination, Developer Technology Team.
\Copyright		Copyright(c) Imagination Technologies Limited.
\brief			Absolutley no optimisations, the most straight forward
				matrix calcultion, B is uploaded as transpose to see how
				the memory layout affects speed
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
************************************************************************/

/*********************** shader  Explanantion **************************
This is a naive implementation of matrix multiplication
There is no no optimisations, each incation just straight forwardly
calculates a cell in the product matrix. B is entered as transposed to
trial different memory layouts

No local memory
Input A BT
Output C
************************************************************************/
void main()
{
	uint x = gl_GlobalInvocationID.x;
	uint y = gl_GlobalInvocationID.y;

	float sum = 0.0;
	for (int k = 0; k < N; ++k) { sum += A[y][k] * BT[x][k]; }
	C[y][x] = sum;
}
