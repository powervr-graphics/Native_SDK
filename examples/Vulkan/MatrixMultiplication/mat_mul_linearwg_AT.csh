/*!**********************************************************************
\File			mat_mul_linearwg_AT.csh
\Title			Matrix multiplication linear workgroups and A transposed
\Author			PowerVR by Imagination, Developer Technology Team.
\Copyright		Copyright(c) Imagination Technologies Limited.
\brief			Performs one benchmark, using workgroups which are 
				segmented into vertical slices, The input matrix A is 
				transposed 
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
no local memory 
Input : AT, B
Output: C
************************************************************************/
void main()
{
	uint x = gl_GlobalInvocationID.x;
	uint y = gl_GlobalInvocationID.y;
	float sum = 0;
	for(int k = 0; k < N; ++k)
	{
		sum += AT[k][y] * B[k][x];
	}
	C[y][x] = sum;

}