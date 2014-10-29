/******************************************************************************

 @File         OGLESParticles.cpp

 @Title        OGLESParticles

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independent

 @Description  Physically based particles demo. Shows 32-bit alpha blending on
               MBX. Requires the PVRShell.

******************************************************************************/
#include <math.h>

#include "PVRShell.h"
#include "OGLESTools.h"

// Contains the particle utility class
#include "Particle.h"

/******************************************************************************
 Content file names
******************************************************************************/

// PVR texture files
const char c_szLightTexFile[] = "LightTex.pvr";
const char c_szFloorTexFile[] = "FloorTex8.pvr";

/******************************************************************************
 Defines
******************************************************************************/

const unsigned int g_ui32MaxParticles = 600;		// Maximum number of m_Particles
const float		g_fFactor = 0.25f;					// Brightness of the reflected m_Particles
const PVRTVec3	g_fUp(0.0f, 1.0f, 0.0f);			// Up direction. Used for creating the camera

/******************************************************************************
 Structure definitions
******************************************************************************/
struct SVtx
{
	float x, y, z;						// Position
	unsigned char u, v;						// TexCoord
};

struct SVtxPointSprite
{
	float x, y, z, fSize;
};

struct SColors
{
	unsigned char r,g,b,a;					// Colour
};

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLESParticles : public PVRShell
{
	// Print3D class used to display text
	CPVRTPrint3D 	m_Print3D;

	// Extensions support class
	CPVRTglesExt	m_Extensions;

	// Texture names
	GLuint 			m_ui32TexName;
	GLuint 			m_ui32FloorTexName;

	// Particle instance pointers
	CParticle m_Particles[g_ui32MaxParticles];

	// View matrix
	PVRTMat4	m_mView;

	// Vectors for calculating the view matrix and saving the camera position
	PVRTVec3 m_fFrom, m_fTo;

	// Particle geometry buffers
	SVtx	m_sParticleVTXBuf[g_ui32MaxParticles*4]; // 4 Vertices per Particle - 2 triangles
	SColors m_sNormalColour[g_ui32MaxParticles*4];
	SColors m_sReflectColour[g_ui32MaxParticles*4];
	unsigned short m_ui16ParticleINDXBuf[g_ui32MaxParticles * 6]; // 3 indices per triangle

	SVtxPointSprite	m_sParticleVTXPSBuf[g_ui32MaxParticles]; // When using point sprites
	GLuint m_i32VertVboID;
	GLuint m_i32ColAVboID;
	GLuint m_i32ColBVboID;
	GLuint m_i32QuadVboID;

	SVtx	 m_sQuadVTXBuf[4];

	// Dynamic state
	int		m_i32NumParticles;
	float	m_fRot, m_fRot2;
	float	m_fPointAttenuationCoef;

public:
	OGLESParticles() :  m_fFrom(0.0f, 45.0f, 120.0f),
						m_fTo(0.0f, 20.0f, -1.0f),
						m_i32NumParticles(0),
						m_fRot(0),
						m_fRot2(0)
	{
	}

	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	float RandPositiveFloat();
	float RandFloat();
	void  RenderFloor();
	void  SpawnParticle(CParticle *pParticle);
	void  RenderParticle(int i32ParticleNo, bool bReflect);
	float Clamp(float input);
};

/*!****************************************************************************
 @Function		InitApplication
 @Return		bool		true if no error occurred
 @Description	Code in InitApplication() will be called by PVRShell once per
				run, before the rendering context is created.
				Used to initialize variables that are not dependant on it
				(e.g. external modules, loading meshes, etc.)
				If the rendering context is lost, InitApplication() will
				not be called again.
******************************************************************************/
bool OGLESParticles::InitApplication()
{
	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	// Get and set the load/release functions for loading external files.
	// In the majority of cases the PVRShell will return NULL function pointers implying that
	// nothing special is required to load external files.
	CPVRTResourceFile::SetLoadReleaseFunctions(PVRShellGet(prefLoadFileFunc), PVRShellGet(prefReleaseFileFunc));
	return true;
}

/*!****************************************************************************
 @Function		QuitApplication
 @Return		bool		true if no error occurred
 @Description	Code in QuitApplication() will be called by PVRShell once per
				run, just before exiting the program.
				If the rendering context is lost, QuitApplication() will
				not be called.
******************************************************************************/
bool OGLESParticles::QuitApplication()
{
	return true;
}

/*!****************************************************************************
 @Function		InitView
 @Return		bool		true if no error occurred
 @Description	Code in InitView() will be called by PVRShell upon
				initialization or after a change in the rendering context.
				Used to initialize variables that are dependant on the rendering
				context (e.g. textures, vertex buffers, etc.)
******************************************************************************/
bool OGLESParticles::InitView()
{
	PVRTMat4		mProjection;
	SPVRTContext	sContext;

	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	// Initialize Print3D textures
	if(m_Print3D.SetTextures(&sContext, PVRShellGet(prefWidth), PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D.\n");
		return false;
	}

	// Initialize Extensions
	m_Extensions.LoadExtensions();

	//	Load textures.
	if(PVRTTextureLoadFromPVR(c_szLightTexFile, &m_ui32TexName) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot load light texture.\n");
		return false;
	}

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(PVRTTextureLoadFromPVR(c_szFloorTexFile, &m_ui32FloorTexName) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot load floor texture.\n");
		return false;
	}

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	if(bRotate)
		glRotatef(90, 0, 0, 1);

	// Creates the projection matrix.
	mProjection = PVRTMat4::PerspectiveFovRH(45.0f*(PVRT_PIf/180.0f), (float)PVRShellGet(prefWidth)/(float)PVRShellGet(prefHeight), 10.0f, 1200.0f, PVRTMat4::OGL);

	glMultMatrixf(mProjection.f);

	//	Calculates the attenuation coefficient for the points drawn.
	double H = bRotate ? PVRShellGet(prefWidth) : PVRShellGet(prefHeight);
	double h = 2.0 / mProjection.f[5];
	double D0 = sqrt(2.0) * H / h;
	double k = 1.0/(1.0 + 2.0 * (1 / mProjection.f[5]) * (1 / mProjection.f[5]));

	m_fPointAttenuationCoef = (float)(1.0 / (D0 * D0) * k);

	//	Creates the model view matrix.
	m_mView = PVRTMat4::LookAtRH(m_fFrom, m_fTo, g_fUp);

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(m_mView.f);

	/*
		Pre-Set TexCoords since they never change.
		Pre-Set the Index Buffer.
	*/

	for(unsigned short i = 0; i < g_ui32MaxParticles; ++i)
	{
		m_sParticleVTXBuf[i*4+0].u = 0;
		m_sParticleVTXBuf[i*4+0].v = 0;

		m_sParticleVTXBuf[i*4+1].u = 1;
		m_sParticleVTXBuf[i*4+1].v = 0;

		m_sParticleVTXBuf[i*4+2].u = 0;
		m_sParticleVTXBuf[i*4+2].v = 1;

		m_sParticleVTXBuf[i*4+3].u = 1;
		m_sParticleVTXBuf[i*4+3].v = 1;

		m_ui16ParticleINDXBuf[i*6+0] = (i*4) + 0;
		m_ui16ParticleINDXBuf[i*6+1] = (i*4) + 1;
		m_ui16ParticleINDXBuf[i*6+2] = (i*4) + 2;
		m_ui16ParticleINDXBuf[i*6+3] = (i*4) + 2;
		m_ui16ParticleINDXBuf[i*6+4] = (i*4) + 1;
		m_ui16ParticleINDXBuf[i*6+5] = (i*4) + 3;
	}


	//	Create vertex buffers.
	glGenBuffers(1, &m_i32VertVboID);
	glGenBuffers(1, &m_i32ColAVboID);
	glGenBuffers(1, &m_i32ColBVboID);
	glGenBuffers(1, &m_i32QuadVboID);

	//	Preset the floor uvs and vertices as they never change.
	PVRTVec3 pos(0, 0, 0);

	float szby2 = 100;

	m_sQuadVTXBuf[0].x = pos.x - szby2;
	m_sQuadVTXBuf[0].y = pos.y;
	m_sQuadVTXBuf[0].z = pos.z - szby2;

	m_sQuadVTXBuf[1].x = pos.x - szby2;
	m_sQuadVTXBuf[1].y = pos.y;
	m_sQuadVTXBuf[1].z = pos.z + szby2;

	m_sQuadVTXBuf[2].x = pos.x + szby2;
	m_sQuadVTXBuf[2].y = pos.y;
	m_sQuadVTXBuf[2].z = pos.z - szby2;

	m_sQuadVTXBuf[3].x = pos.x + szby2;
	m_sQuadVTXBuf[3].y = pos.y;
	m_sQuadVTXBuf[3].z = pos.z + szby2;

	m_sQuadVTXBuf[0].u = 0;
	m_sQuadVTXBuf[0].v = 0;

	m_sQuadVTXBuf[1].u = 255;
	m_sQuadVTXBuf[1].v = 0;

	m_sQuadVTXBuf[2].u = 0;
	m_sQuadVTXBuf[2].v = 255;

	m_sQuadVTXBuf[3].u = 255;
	m_sQuadVTXBuf[3].v = 255;

	glBindBuffer(GL_ARRAY_BUFFER, m_i32QuadVboID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(SVtx) * 4, m_sQuadVTXBuf, GL_STATIC_DRAW);

	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occurred
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLESParticles::ReleaseView()
{
	// Release textures
	glDeleteTextures(1, &m_ui32TexName);
	glDeleteTextures(1, &m_ui32FloorTexName);

	// Release Print3D Textures
	m_Print3D.ReleaseTextures();
	return true;
}

/*!****************************************************************************
 @Function		RenderScene
 @Return		bool		true if no error occurred
 @Description	Main rendering loop function of the program. The shell will
				call this function every frame.
				eglSwapBuffers() will be performed by PVRShell automatically.
				PVRShell will also manage important OS events.
				Will also manage relevant OS events. The user has access to
				these events through an abstraction layer provided by PVRShell.
******************************************************************************/
bool OGLESParticles::RenderScene()
{
	int				i;
	PVRTMat4		mRotY;

	// Clear colour and depth buffers
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Enables depth testing
	glEnable(GL_DEPTH_TEST);

	// Modify per-frame variables controlling the particle movements.
	float fSpeedCtrl = (float) (PVRTFSIN(m_fRot*0.01f)+1.0f)/2.0f;
	float fStopNo = 0.8f;
	float fStep = 0.1f;

	if(fSpeedCtrl > fStopNo)
		fStep = 0.0f;

	// Generate particles as needed.
	if((m_i32NumParticles < (int) g_ui32MaxParticles) && (fSpeedCtrl <= fStopNo))
	{
		int num_to_gen = (int) (RandPositiveFloat()*(g_ui32MaxParticles/100.0));

		if(num_to_gen == 0)
			num_to_gen = 1;

		for(i = 0; (i < num_to_gen) && (m_i32NumParticles < (int) g_ui32MaxParticles); ++i)
			SpawnParticle(&m_Particles[m_i32NumParticles++]);
	}

	// Build rotation matrix around axis Y.
	mRotY = PVRTMat4::RotationY((m_fRot2*PVRT_PIf)/180.0f);

	for(i = 0; i < m_i32NumParticles; ++i)
	{
		// Transform particle with rotation matrix
		m_sParticleVTXPSBuf[i].x =	mRotY.f[ 0] * m_Particles[i].m_fPosition.x +
									mRotY.f[ 4] * m_Particles[i].m_fPosition.y +
									mRotY.f[ 8] * m_Particles[i].m_fPosition.z +
									mRotY.f[12];

		m_sParticleVTXPSBuf[i].y =	mRotY.f[ 1] * m_Particles[i].m_fPosition.x +
									mRotY.f[ 5] * m_Particles[i].m_fPosition.y +
									mRotY.f[ 9] * m_Particles[i].m_fPosition.z +
									mRotY.f[13];

		m_sParticleVTXPSBuf[i].z =	mRotY.f[ 2] * m_Particles[i].m_fPosition.x +
									mRotY.f[ 6] * m_Particles[i].m_fPosition.y +
									mRotY.f[10] * m_Particles[i].m_fPosition.z +
									mRotY.f[14];

		m_sParticleVTXPSBuf[i].fSize = m_Particles[i].m_fSize;

		m_sNormalColour[i].r  = (unsigned char) m_Particles[i].m_fColour.x;
		m_sNormalColour[i].g  = (unsigned char) m_Particles[i].m_fColour.y;
		m_sNormalColour[i].b  = (unsigned char) m_Particles[i].m_fColour.z;
		m_sNormalColour[i].a  = (unsigned char)255;

		m_sReflectColour[i].r  = (unsigned char) (m_Particles[i].m_fColour.x * g_fFactor);
		m_sReflectColour[i].g  = (unsigned char) (m_Particles[i].m_fColour.y * g_fFactor);
		m_sReflectColour[i].b  = (unsigned char) (m_Particles[i].m_fColour.z * g_fFactor);
		m_sReflectColour[i].a  = (unsigned char)255;
	}

	glBindBuffer(GL_ARRAY_BUFFER, m_i32VertVboID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(SVtxPointSprite)*m_i32NumParticles, m_sParticleVTXPSBuf,GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_i32ColAVboID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(SColors)*m_i32NumParticles, m_sNormalColour,GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_i32ColBVboID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(SColors)*m_i32NumParticles, m_sReflectColour,GL_DYNAMIC_DRAW);

	// clean up render states
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);

	//	Draw floor.

	// Save modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glRotatef(-m_fRot, 0.0f, 1.0f, 0.0f);

	// setup render states
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);

	// Set texture and texture environment
	glBindTexture(GL_TEXTURE_2D, m_ui32FloorTexName);
	glBlendFunc(GL_ONE, GL_ONE);

	// Render floor
	RenderFloor();

	// clean up render states
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);

	glPopMatrix();

	//	Render particles reflections.

	// set up render states
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);

	glDepthFunc(GL_ALWAYS);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glBindTexture(GL_TEXTURE_2D, m_ui32TexName);

	// Set model view matrix
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glScalef(1.0f, -1.0f, 1.0f);
	glTranslatef(0.0f, 0.01f, 0.0f);

	glEnable(GL_POINT_SPRITE_OES);

	if(((int)(m_i32NumParticles * 0.5f)) > 0)
       RenderParticle(((int)(m_i32NumParticles*0.5f)),true);

	glPopMatrix();

	//	Render particles.

	// Sets the model view matrix
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	if(m_i32NumParticles > 0)
        RenderParticle(m_i32NumParticles,false);

	glPopMatrix();

	glDisable(GL_POINT_SPRITE_OES);

	PVRTVec3 Force = PVRTVec3(0.0f, 0.0f, 0.0f);
	Force.x = 1000.0f*(float)PVRTFSIN(m_fRot*0.01f);

	for(i = 0; i < m_i32NumParticles; ++i)
	{
		/*
			Move the particle.
			If the particle exceeds its lifetime, create a new one in its place.
		*/
		if(m_Particles[i].Step(fStep, Force))
			SpawnParticle(&m_Particles[i]);
	}

	// clean up render states
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);

	// Increase rotation angles
	m_fRot += 1;
	m_fRot2 = m_fRot + 36;

	// Unbinds the vertex buffer if we are using OpenGL ES 1.1
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Display info text.
	m_Print3D.DisplayDefaultTitle("Particles", "Using point sprites", ePVRTPrint3DSDKLogo);
	m_Print3D.Flush();

	return true;
}

/*!****************************************************************************
 @Function		RandFloat
 @Return		float		random float from -1 to 1
 @Description	returns a random float in range -1 to 1.
******************************************************************************/
float OGLESParticles::RandFloat()
{
	return (rand()/(float)RAND_MAX) * 2.0f - 1.0f;
}

/*!****************************************************************************
 @Function		RandPositiveFloat
 @Return		float		random float from 0 to 1
 @Description	returns a random float in range 0 to 1.
******************************************************************************/
float OGLESParticles::RandPositiveFloat()
{
	return rand()/(float)RAND_MAX;
}

/*!****************************************************************************
 @Function		SpawnParticle
 @Output		pParticle	particle to initialize
 @Description	initializes the specified particle with randomly chosen parameters.
******************************************************************************/
void OGLESParticles::SpawnParticle(CParticle *pParticle)
{
	PVRTVec3 fParticleSource(0, 0, 0);
	PVRTVec3 fParticleSourceVariability(1, 0, 1);
	PVRTVec3 fParticleVelocity(0, 30, 0);
	PVRTVec3 fParticleVelocityVariability(4, 15, 4);
	float fParticleLifeTime = 8;
	float fParticleLifeTimeVariability = 1.0;

	float fParticleMass = 100;
	float fParticleMassVariability = 0;
	float fRndFloat;

	// Creates the particle position.
	PVRTVec3 fPos;
	fRndFloat = RandFloat();
	fPos.x = fParticleSource.x + fRndFloat * fParticleSourceVariability.x;
	fRndFloat = RandFloat();
	fPos.y = fParticleSource.y + fRndFloat * fParticleSourceVariability.y;
	fRndFloat = RandFloat();
	fPos.z = fParticleSource.z + fRndFloat * fParticleSourceVariability.z;

	// Creates the particle velocity.
	PVRTVec3 fVel;
	fRndFloat = RandFloat();
	fVel.x = fParticleVelocity.x + fRndFloat * fParticleVelocityVariability.x;
	fRndFloat = RandFloat();
	fVel.y = fParticleVelocity.y + fRndFloat * fParticleVelocityVariability.y;
	fRndFloat = RandFloat();
	fVel.z = fParticleVelocity.z + fRndFloat * fParticleVelocityVariability.z;

	// Creates the particle lifetime and fMass.
	float fLife = fParticleLifeTime + RandFloat() * fParticleLifeTimeVariability;
	float fMass = fParticleMass + RandFloat() * fParticleMassVariability;

	// Creates the particle from these characteristics.
	*pParticle = CParticle(fPos,fVel,fMass,fLife);

	// Creates the particle colours.
	PVRTVec3 fParticleInitialColour(0.6f*255.0f, 0.5f*255.0f, 0.5f*255.0f);
	PVRTVec3 fParticleInitialColourVariability(0.2f*255.0f, 0.2f*255.0f, 0.2f*255.0f);

	PVRTVec3 fParticleHalfwayColour(1.0f*255.0f, 0.0f, 0.0f);
	PVRTVec3 fParticleHalfwayColourVariability(0.8f*255.0f, 0.0f, 0.3f*255.0f);

	PVRTVec3 fParticleEndColour(0.0f, 0.0f, 0.0f);
	PVRTVec3 fParticleEndColourVariability(0.0f, 0.0f, 0.0f);

	float fRndValue = RandFloat();
	pParticle->m_fColour.x = pParticle->m_fInitialColour.x = Clamp(fParticleInitialColour.x + fParticleInitialColourVariability.x * fRndValue);
	pParticle->m_fColour.y = pParticle->m_fInitialColour.y = Clamp(fParticleInitialColour.y + fParticleInitialColourVariability.y * fRndValue);
	pParticle->m_fColour.z = pParticle->m_fInitialColour.z = Clamp(fParticleInitialColour.z + fParticleInitialColourVariability.z * fRndValue);

	fRndFloat = RandFloat();
	pParticle->m_fHalfwayColour.x = Clamp(fParticleHalfwayColour.x + fRndFloat * fParticleHalfwayColourVariability.x);
	fRndFloat = RandFloat();
	pParticle->m_fHalfwayColour.y = Clamp(fParticleHalfwayColour.y + fRndFloat * fParticleHalfwayColourVariability.y);
	fRndFloat = RandFloat();
	pParticle->m_fHalfwayColour.z = Clamp(fParticleHalfwayColour.z + fRndFloat * fParticleHalfwayColourVariability.z);

	fRndFloat = RandFloat();
	pParticle->m_fEndColor.x = Clamp(fParticleEndColour.x + fRndFloat * fParticleEndColourVariability.x);
	fRndFloat = RandFloat();
	pParticle->m_fEndColor.y = Clamp(fParticleEndColour.y + fRndFloat * fParticleEndColourVariability.y);
	fRndFloat = RandFloat();
	pParticle->m_fEndColor.z = Clamp(fParticleEndColour.z + fRndFloat * fParticleEndColourVariability.z);

	// Creates the particle size using a perturbation.
	float fParticleSize = 2.0f;
	float fParticleSizeVariation = 1.5f;
	fRndFloat = RandFloat();
	pParticle->m_fSize = fParticleSize + fRndFloat * fParticleSizeVariation;
}

/*!****************************************************************************
 @Function		RenderParticle
 @Input			NmbrOfParticles		number of particles to initialize
 @Input			bReflect			should we use the reflection colour ?
 @Description	Renders the specified set of particles, optionally using the
				reflection colour.
******************************************************************************/
void OGLESParticles::RenderParticle(int i32ParticleNo, bool bReflect)
{
	// If point sprites are available, use them to draw the particles.
	glBindBuffer(GL_ARRAY_BUFFER, m_i32VertVboID);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3,GL_FLOAT, sizeof(SVtxPointSprite), 0);

	glTexEnvf( GL_POINT_SPRITE_OES, GL_COORD_REPLACE_OES, GL_TRUE );
	glEnableClientState(GL_POINT_SIZE_ARRAY_OES);

	m_Extensions.glPointSizePointerOES(GL_FLOAT, sizeof(SVtxPointSprite),(GLvoid*) (sizeof(float)*3));

	float fCoefs[4] = { 0, 0, m_fPointAttenuationCoef, 0 };

	glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, fCoefs);

	glEnableClientState(GL_COLOR_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, bReflect ? m_i32ColBVboID : m_i32ColAVboID);

	glColorPointer(4, GL_UNSIGNED_BYTE, 0, 0);
	glDrawArrays(GL_POINTS, 0, i32ParticleNo);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_POINT_SIZE_ARRAY_OES);
	glDisableClientState(GL_COLOR_ARRAY);
}

/*!****************************************************************************
 @Function		Clamp
 @Input			X			number to Clamp
 @Return		float	Clamped number
 @Description	Clamps the argument to 0-255.
******************************************************************************/
float OGLESParticles::Clamp(float X)
{
	if(X < 0.0f)
		X = 0.0f;
	else if(X > 255.0f)
		X = 255.0f;

	return X;
}

/*!****************************************************************************
 @Function		RenderFloor
 @Description	Renders the floor as a quad.
******************************************************************************/
void OGLESParticles::RenderFloor()
{
	// Draw the floor using regular geometry for the quad.
	glBindBuffer(GL_ARRAY_BUFFER, m_i32QuadVboID);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(3,GL_FLOAT,sizeof(SVtx),0);
	glTexCoordPointer(2,GL_BYTE,sizeof(SVtx),(const GLvoid*) (3*sizeof(float)));

	glDrawArrays(GL_TRIANGLE_STRIP,0,4);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}


/*!****************************************************************************
 @Function		NewDemo
 @Return		PVRShell*		The demo supplied by the user
 @Description	This function must be implemented by the user of the shell.
				The user should return its PVRShell object defining the
				behaviour of the application.
******************************************************************************/
PVRShell* NewDemo()
{
	return new OGLESParticles();
}

/******************************************************************************
 End of file (OGLESParticles.cpp)
******************************************************************************/

