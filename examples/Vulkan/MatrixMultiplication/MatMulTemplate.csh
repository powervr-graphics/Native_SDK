/*!**********************************************************************
\File			MatMulTemplate.csh
\Title			Matrix Multiplication Template
\Author			PowerVR by Imagination, Developer Technology Team.
\Copyright		Copyright(c) Imagination Technologies Limited.
\brief			This file is a template shader, it is combined with each 
				individual shader during runtime to make a fully compiled 
				shader. This mainly lists the binding points
************************************************************************/
/*************************** Binding Points  ***************************
/*
/* A(MxN)
/* B(NxP)
/* C(MxP)
/*
/*And the transposes of those matrices and the vec4 implementations 
/*for A and B
/*
/***********************************************************************/

/******** Input Defines - These must be defined on the CPU side ********/
//Shader Buffer Object to describe the matrix on the LHS 
//Typically A
layout(std430, binding = 0) readonly buffer ssboAMatrixIn
{
	float[M][N] A;
};

//Shader Buffer Object to desccribe the matrix on the RHS
//Typically B
layout(std430, binding = 1) readonly buffer ssboBMatrixIn
{
	float[N][P] B;
};

//Shader Buffer object to describe the product of the resulting matrix multiplication
//Typically C
layout(std430, binding = 2) writeonly buffer ssboCMatrixOut
{
	float[M][P] C;
};

//Shader Buffer Object to describe the matrix on the LHS after transposition
//Typically AT
layout(std430, binding = 3) readonly buffer ssboATMatrixIn
{
	float[N][M] AT;
};

//Shader Buffer Object to desccribe the matrix on the RHS after transposition
//Typically BT
layout(std430, binding = 4) readonly buffer ssboBTMatrixIn
{
	float[P][N] BT;
};

//Shader Buffer object to describe the product of the resulting matrix multiplication after transposition
//Typically CT
layout(std430, binding = 5) writeonly buffer ssboCTMatrixOut
{
	float[P][M] CT;
};

//Shader Buffer objects to represent the vector4 1 dimensional array for the lhs
//typically A
layout(std430, binding = 6) readonly buffer ssboVec4AMatrixIn
{
	vec4[M * N / 4] VecA;
};

//Shader Buffer objects to represent the vector4 1 dimensional array for the rhs
//typically BT so that the vector reads line up correctly
layout(std430, binding = 7) readonly buffer ssboVec4BTMatrixIn
{
	vec4[N * P / 4] VecBT;
};

//Set the workgroupo sizes from our define
layout(local_size_x = WG_X_SIZE, local_size_y = WG_Y_SIZE) in;





