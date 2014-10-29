/******************************************************************************

 @File         ProceduralTextures.h

 @Title        ProceduralTextures

 @Version

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independent

 @Description  Procedural texture example based on Steven Worley's  Cellular
               Texture Basis Functions.

******************************************************************************/

#ifndef _PROCEDURALTEXTURES__H_
#define _PROCEDURALTEXTURES__H_

#include "OGLES31Tools.h"

/******************************************************************************
 Enumerations
******************************************************************************/

enum eGenerator
{
  EUCLID = 0,
  MANHATTAN,
  CHESSBOARD,
  NUM_GENERATORS
};

/*!****************************************************************************
 Class implementing the Procedural Textures functionality.
 The model will be an OpenGL texture and a generator type to be provided, 
 and the texture will be generated in place in that GL texture object.
******************************************************************************/
class ProceduralTextures
{
public:

	ProceduralTextures();
	~ProceduralTextures();

	GLuint      m_Programs[NUM_GENERATORS];

public:

	bool Init(CPVRTString* pErrorStr);

	void Release();

	bool GenerateIntoTexture(const eGenerator generator, GLuint texture, float width, float height, float scale);

	const char* GetModeDescription(eGenerator generator) const;
};

#endif //_PROCEDURALTEXTURES__H_

/******************************************************************************
 End of file (ProceduralTextures.h)
******************************************************************************/

