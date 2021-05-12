/*!**********************************************************************
\File			mat_mul_naive_CT.csh
\Title			Naive Matrix Multiplication C transposed
\Author			PowerVR by Imagination, Developer Technology Team.
\Copyright		Copyright(c) Imagination Technologies Limited.
\brief			Absolutley no optimisations, the most straight forward
				matrix calcultion, C is stored transposed to see how
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

					VecA  Vec4 version of the matrix A
					VecBT Vec4 version of the matrix BT
************************************************************************/

/*********************** shader  Explanantion **************************
This is a naive implementation of matrix multiplication
It simpily uses one work group per cell of the product
In this instance the Lhs matrix has been transposed this
is to test if the different buffer layout has an effect
Input : A, B
Output: C transposed
************************************************************************/
void main()
{
	uint x = gl_GlobalInvocationID.x;
	uint y = gl_GlobalInvocationID.y;
	float sum = 0.0;
	for (int k = 0; k < N; ++k) { sum += A[y][k] * B[k][x]; }
	CT[x][y] = sum;
}
