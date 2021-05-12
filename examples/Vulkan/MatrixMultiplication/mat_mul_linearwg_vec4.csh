/*!**********************************************************************
\File			mat_mul_linearwg_vec4.csh
\Title			Matrix multiplication linear workgroups using vec4
\Author			PowerVR by Imagination, Developer Technology Team.
\Copyright		Copyright(c) Imagination Technologies Limited.
\brief			Performs one benchmark, Performs the multiplication
				using the input matrices as an array of vec4s
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
This reduces the number of internal loops when multiplying and a such
the number of reads to memory
no local memory
Input : A, BT
Output: C
************************************************************************/
void main()
{
	int x = int(gl_GlobalInvocationID.x); // 0,..,P
	int y = int(gl_GlobalInvocationID.y); // 0,..,M
	vec4 sum = vec4(0.0, 0.0, 0.0, 0.0);
	for (int k = 0; k < N / 4; ++k) { sum += VecA[y * (N / 4) + k] * VecBT[x * (N / 4) + k]; }

	C[y][x] = sum.x + sum.y + sum.z + sum.w;
}
