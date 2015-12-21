/*!*********************************************************************************************************************
\file         PVRAssets/PVRShamanSemantics.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Provides a list of Semantics that are directly understood by PVR Shaman and the POD loading code.
***********************************************************************************************************************/
#pragma once
#include "PVRAssets/Effect.h"
namespace pvr {
namespace assets {

/*!*****************************************************************************************************************
\brief         Struct to convert a semantic string to a number.
               The application supplies an array of these so PVRTPFX can translate semantic strings to numbers.
*******************************************************************************************************************/
struct SemanticNameIdPair
{
	string semanticName;	/*!< String containing semantic */
	uint32 semanticIndex;	/*!< Application-defined semantic value */

	bool operator==(const SemanticNameIdPair& rhs) { return (semanticIndex == rhs.semanticIndex); }
};


/*!*****************************************************************************************************************
\brief         Struct which contains list of shaman semantics and its name
*******************************************************************************************************************/
class PVRShamanSemantics
{
public:
	enum Enum
	{
		Position,					/*!< POSITION */
		Normal,						/*!< NORMAL */
		Tangent,					/*!< TANGENT */
		Binormal,					/*!< BINORMAL */
		UV,							/*!< UV */
		VertexColor,				/*!< VERTEXCOLOR */
		BoneIndex,					/*!< BONEINDEX */
		BoneWeight,					/*!< BONEWEIGHT */

		World,						/*!< WORLD */
		WorldI,						/*!< WORLDI */
		WorldIT,					/*!< WORLDIT */
		View,						/*!< VIEW */
		ViewI,						/*!< VIEWI */
		ViewIT,						/*!< VIEWIT */
		Projection,					/*!< PROJECTION */
		ProjectionI,				/*!< PROJECTIONI */
		ProjectionIT,				/*!< PROJECTIONIT */
		WorldView,					/*!< WORLDVIEW */
		WorldViewI,					/*!< WORLDVIEWI */
		WorldViewIT,				/*!< WORLDVIEWIT */
		WorldViewProjection,		/*!< WORLDVIEWPROJECTION */
		WorldViewProjectionI,		/*!< WORLDVIEWPROJECTIONI */
		WorldViewProjectionIT,		/*!< WORLDVIEWPROJECTIONIT */
		ViewProjection,				/*!< VIEWPROJECTION */
		ViewProjectionI,			/*!< VIEWPROJECTIONI */
		ViewProjectionIT,			/*!< VIEWPROJECTIONIT */
		Object,						/*!< OBJECT */
		ObjectI,					/*!< OBJECTI */
		ObjectIT,					/*!< OBJECTIT */
		UnpackMatrix,				/*!< UNPACKMATRIX */

		BoneCount,					/*!< BONECOUNT */
		BoneMatrixArray,			/*!< BONEMATRIXARRAY */
		BoneMatrixArrayIT,			/*!< BONEMATRIXARRAYIT */

		MaterialOpacity,			/*!< MATERIALOPACITY */
		MaterialShininess,			/*!< MATERIALSHININESS */
		MaterialColorAmbient,		/*!< MATERIALCOLORAMBIENT */
		MaterialColorDiffuse,		/*!< MATERIALCOLORDIFFUSE */
		MaterialColorSpecular,		/*!< MATERIALCOLORSPECULAR */

		LightColor,					/*!< LIGHTCOLOR */
		LightPosModel,				/*!< LIGHTPOSMODEL */
		LightPosWorld,				/*!< LIGHTPOSWORLD */
		LightPosEye,				/*!< LIGHTPOSEYE */
		LightDirModel,				/*!< LIGHTDIRMODEL */
		LightDirWorld,				/*!< LIGHTDIRWORLD */
		LightDirEye,				/*!< LIGHTDIREYE */
		LightAttenuation,			/*!< LIGHTATTENUATION */
		LightFallOff,				/*!< LIGHTFALLOFF */

		EyePosModel,				/*!< EYEPOSMODEL */
		EyePosWorld,				/*!< EYEPOSWORLD */
		Texture,					/*!< TEXTURE */
		Animation,					/*!< ANIMATION */

		ViewportPixelSize,			/*!< VIEWPORTPIXELSIZE */
		ViewportClipping,			/*!< VIEWPORTCLIPPING */
		Time,						/*!< TIME */
		TimeCos,					/*!< TIMECOS */
		TimeSin,					/*!< TIMESIN */
		TimeTan,					/*!< TIMETAN */
		Time2Pi,					/*!< TIME2PI */
		Time2PiCos,					/*!< TIME2PICOS */
		Time2PiSin,					/*!< TIME2PISIN */
		Time2PiTan,					/*!< TIME2PITAN */
		Random,						/*!< RANDOM */

		Count,				/*!< Semantic number */
	};

	static const SemanticNameIdPair Mapping[PVRShamanSemantics::Count];

	/*!*********************************************************************************************************************
	\brief Get map of shaman semantics.
	\return Return a map of shaman seantics and its name
	***********************************************************************************************************************/
	static const std::map<const char*, PVRShamanSemantics::Enum>& getShamanSemanticsMap();

	/*!*********************************************************************************************************************
	\brief Get the data type expected by PVRShaman (or provided by POD code) for a specific semantic index.
	\param[in] shamanSemanticIndex index of the shaman semantic 
	\return Retunr semantic data type
	***********************************************************************************************************************/
	static SemanticDataType::Enum getSemanticDataType(PVRShamanSemantics::Enum shamanSemanticIndex)
	{
		static const SemanticDataType::Enum map[] =
		{
			SemanticDataType::Vec4,//Position,					/*!< POSITION */
			SemanticDataType::Vec3,//Normal,						/*!< NORMAL */
			SemanticDataType::Vec3,//Tangent,					/*!< TANGENT */
			SemanticDataType::Vec3,//Binormal,					/*!< BINORMAL */
			SemanticDataType::Vec2,//UV,							/*!< UV */
			SemanticDataType::Vec3,//VertexColor,				/*!< VERTEXCOLOR */
			SemanticDataType::Int1,//BoneIndex,					/*!< BONEINDEX */
			SemanticDataType::Float,//BoneWeight,					/*!< BONEWEIGHT */

			SemanticDataType::Mat4,//	World,						/*!< WORLD */
			SemanticDataType::Mat4,//	WorldI,						/*!< WORLDI */
			SemanticDataType::Mat4,//	WorldIT,					/*!< WORLDIT */
			SemanticDataType::Mat4,//	View,						/*!< VIEW */
			SemanticDataType::Mat4,//	ViewI,						/*!< VIEWI */
			SemanticDataType::Mat4,//	ViewIT,						/*!< VIEWIT */
			SemanticDataType::Mat4,//	Projection,					/*!< PROJECTION */
			SemanticDataType::Mat4,//	ProjectionI,				/*!< PROJECTIONI */
			SemanticDataType::Mat4,//	ProjectionIT,				/*!< PROJECTIONIT */
			SemanticDataType::Mat4,//	WorldView,					/*!< WORLDVIEW */
			SemanticDataType::Mat4,//	WorldViewI,					/*!< WORLDVIEWI */
			SemanticDataType::Mat4,//	WorldViewIT,				/*!< WORLDVIEWIT */
			SemanticDataType::Mat4,//	WorldViewProjection,		/*!< WORLDVIEWPROJECTION */
			SemanticDataType::Mat4,//	WorldViewProjectionI,		/*!< WORLDVIEWPROJECTIONI */
			SemanticDataType::Mat4,//	WorldViewProjectionIT,		/*!< WORLDVIEWPROJECTIONIT */
			SemanticDataType::Mat4,//	ViewProjection,				/*!< VIEWPROJECTION */
			SemanticDataType::Mat4,//	ViewProjectionI,			/*!< VIEWPROJECTIONI */
			SemanticDataType::Mat4,//	ViewProjectionIT,			/*!< VIEWPROJECTIONIT */
			SemanticDataType::Mat4,//	Object,						/*!< OBJECT */
			SemanticDataType::Mat4,//	ObjectI,					/*!< OBJECTI */
			SemanticDataType::Mat4,//	ObjectIT,					/*!< OBJECTIT */
			SemanticDataType::Mat4,//	UnpackMatrix,				/*!< UNPACKMATRIX */

			SemanticDataType::Int1,//	BoneCount,					/*!< BONECOUNT */
			SemanticDataType::Mat4,//	BoneMatrixArray,			/*!< BONEMATRIXARRAY */
			SemanticDataType::Mat4,//	BoneMatrixArrayIT,			/*!< BONEMATRIXARRAYIT */

			SemanticDataType::Float,//	MaterialOpacity,			/*!< MATERIALOPACITY */
			SemanticDataType::Float,//	MaterialShininess,			/*!< MATERIALSHININESS */
			SemanticDataType::Vec4,//	MaterialColorAmbient,		/*!< MATERIALCOLORAMBIENT */
			SemanticDataType::Vec4,//	MaterialColorDiffuse,		/*!< MATERIALCOLORDIFFUSE */
			SemanticDataType::Vec4,//	MaterialColorSpecular,		/*!< MATERIALCOLORSPECULAR */

			SemanticDataType::Vec3,//	LightColor,					/*!< LIGHTCOLOR */
			SemanticDataType::Vec3,//	LightPosModel,				/*!< LIGHTPOSMODEL */
			SemanticDataType::Vec3,//	LightPosWorld,				/*!< LIGHTPOSWORLD */
			SemanticDataType::Vec3,//	LightPosEye,				/*!< LIGHTPOSEYE */
			SemanticDataType::Vec3,//	LightDirModel,				/*!< LIGHTDIRMODEL */
			SemanticDataType::Vec3,//	LightDirWorld,				/*!< LIGHTDIRWORLD */
			SemanticDataType::Vec3,//	LightDirEye,				/*!< LIGHTDIREYE */
			SemanticDataType::Float,//	LightAttenuation,			/*!< LIGHTATTENUATION */
			SemanticDataType::Float,//	LightFallOff,				/*!< LIGHTFALLOFF */

			SemanticDataType::Vec3,//	EyePosModel,				/*!< EYEPOSMODEL */
			SemanticDataType::Vec3,//	EyePosWorld,				/*!< EYEPOSWORLD */
			SemanticDataType::Int1,//	Texture,					/*!< TEXTURE */
			SemanticDataType::Int1,//	Animation,					/*!< ANIMATION */

			SemanticDataType::Int1,//	ViewportPixelSize,			/*!< VIEWPORTPIXELSIZE */
			SemanticDataType::Int1,//	ViewportClipping,			/*!< VIEWPORTCLIPPING */
			SemanticDataType::Float,//	Time,						/*!< TIME */
			SemanticDataType::Float,//	TimeCos,					/*!< TIMECOS */
			SemanticDataType::Float,//	TimeSin,					/*!< TIMESIN */
			SemanticDataType::Float,//	TimeTan,					/*!< TIMETAN */
			SemanticDataType::Float,//	Time2Pi,					/*!< TIME2PI */
			SemanticDataType::Float,//	Time2PiCos,					/*!< TIME2PICOS */
			SemanticDataType::Float,//	Time2PiSin,					/*!< TIME2PISIN */
			SemanticDataType::Float,//	Time2PiTan,					/*!< TIME2PITAN */
			SemanticDataType::Float,//	Random,						/*!< RANDOM */
		};
		return map[shamanSemanticIndex];
	}

	/*!*********************************************************************************************************************
	\brief Return a index to the shaman semantic.
	\param[in] shamanSemanticName shaman semantic name
	\return The index of a PVR Shaman understood semantic.
	***********************************************************************************************************************/
	static PVRShamanSemantics::Enum getSemanticIndex(const char* shamanSemanticName)
	{
		return getShamanSemanticsMap().at(shamanSemanticName);
	}

};
}
}
