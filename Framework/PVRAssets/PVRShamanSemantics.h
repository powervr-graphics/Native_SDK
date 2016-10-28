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
	static types::SemanticDataType getSemanticDataType(PVRShamanSemantics::Enum shamanSemanticIndex)
	{
		static const types::SemanticDataType map[] =
		{
			types::SemanticDataType::Vec4,//Position,					/*!< POSITION */
			types::SemanticDataType::Vec3,//Normal,						/*!< NORMAL */
			types::SemanticDataType::Vec3,//Tangent,					/*!< TANGENT */
			types::SemanticDataType::Vec3,//Binormal,					/*!< BINORMAL */
			types::SemanticDataType::Vec2,//UV,							/*!< UV */
			types::SemanticDataType::Vec3,//VertexColor,				/*!< VERTEXCOLOR */
			types::SemanticDataType::Int1,//BoneIndex,					/*!< BONEINDEX */
			types::SemanticDataType::Float,//BoneWeight,					/*!< BONEWEIGHT */

			types::SemanticDataType::Mat4,//	World,						/*!< WORLD */
			types::SemanticDataType::Mat4,//	WorldI,						/*!< WORLDI */
			types::SemanticDataType::Mat4,//	WorldIT,					/*!< WORLDIT */
			types::SemanticDataType::Mat4,//	View,						/*!< VIEW */
			types::SemanticDataType::Mat4,//	ViewI,						/*!< VIEWI */
			types::SemanticDataType::Mat4,//	ViewIT,						/*!< VIEWIT */
			types::SemanticDataType::Mat4,//	Projection,					/*!< PROJECTION */
			types::SemanticDataType::Mat4,//	ProjectionI,				/*!< PROJECTIONI */
			types::SemanticDataType::Mat4,//	ProjectionIT,				/*!< PROJECTIONIT */
			types::SemanticDataType::Mat4,//	WorldView,					/*!< WORLDVIEW */
			types::SemanticDataType::Mat4,//	WorldViewI,					/*!< WORLDVIEWI */
			types::SemanticDataType::Mat4,//	WorldViewIT,				/*!< WORLDVIEWIT */
			types::SemanticDataType::Mat4,//	WorldViewProjection,		/*!< WORLDVIEWPROJECTION */
			types::SemanticDataType::Mat4,//	WorldViewProjectionI,		/*!< WORLDVIEWPROJECTIONI */
			types::SemanticDataType::Mat4,//	WorldViewProjectionIT,		/*!< WORLDVIEWPROJECTIONIT */
			types::SemanticDataType::Mat4,//	ViewProjection,				/*!< VIEWPROJECTION */
			types::SemanticDataType::Mat4,//	ViewProjectionI,			/*!< VIEWPROJECTIONI */
			types::SemanticDataType::Mat4,//	ViewProjectionIT,			/*!< VIEWPROJECTIONIT */
			types::SemanticDataType::Mat4,//	Object,						/*!< OBJECT */
			types::SemanticDataType::Mat4,//	ObjectI,					/*!< OBJECTI */
			types::SemanticDataType::Mat4,//	ObjectIT,					/*!< OBJECTIT */
			types::SemanticDataType::Mat4,//	UnpackMatrix,				/*!< UNPACKMATRIX */

			types::SemanticDataType::Int1,//	BoneCount,					/*!< BONECOUNT */
			types::SemanticDataType::Mat4,//	BoneMatrixArray,			/*!< BONEMATRIXARRAY */
			types::SemanticDataType::Mat4,//	BoneMatrixArrayIT,			/*!< BONEMATRIXARRAYIT */

			types::SemanticDataType::Float,//	MaterialOpacity,			/*!< MATERIALOPACITY */
			types::SemanticDataType::Float,//	MaterialShininess,			/*!< MATERIALSHININESS */
			types::SemanticDataType::Vec4,//	MaterialColorAmbient,		/*!< MATERIALCOLORAMBIENT */
			types::SemanticDataType::Vec4,//	MaterialColorDiffuse,		/*!< MATERIALCOLORDIFFUSE */
			types::SemanticDataType::Vec4,//	MaterialColorSpecular,		/*!< MATERIALCOLORSPECULAR */

			types::SemanticDataType::Vec3,//	LightColor,					/*!< LIGHTCOLOR */
			types::SemanticDataType::Vec3,//	LightPosModel,				/*!< LIGHTPOSMODEL */
			types::SemanticDataType::Vec3,//	LightPosWorld,				/*!< LIGHTPOSWORLD */
			types::SemanticDataType::Vec3,//	LightPosEye,				/*!< LIGHTPOSEYE */
			types::SemanticDataType::Vec3,//	LightDirModel,				/*!< LIGHTDIRMODEL */
			types::SemanticDataType::Vec3,//	LightDirWorld,				/*!< LIGHTDIRWORLD */
			types::SemanticDataType::Vec3,//	LightDirEye,				/*!< LIGHTDIREYE */
			types::SemanticDataType::Float,//	LightAttenuation,			/*!< LIGHTATTENUATION */
			types::SemanticDataType::Float,//	LightFallOff,				/*!< LIGHTFALLOFF */

			types::SemanticDataType::Vec3,//	EyePosModel,				/*!< EYEPOSMODEL */
			types::SemanticDataType::Vec3,//	EyePosWorld,				/*!< EYEPOSWORLD */
			types::SemanticDataType::Int1,//	Texture,					/*!< TEXTURE */
			types::SemanticDataType::Int1,//	Animation,					/*!< ANIMATION */

			types::SemanticDataType::Int1,//	ViewportPixelSize,			/*!< VIEWPORTPIXELSIZE */
			types::SemanticDataType::Int1,//	ViewportClipping,			/*!< VIEWPORTCLIPPING */
			types::SemanticDataType::Float,//	Time,						/*!< TIME */
			types::SemanticDataType::Float,//	TimeCos,					/*!< TIMECOS */
			types::SemanticDataType::Float,//	TimeSin,					/*!< TIMESIN */
			types::SemanticDataType::Float,//	TimeTan,					/*!< TIMETAN */
			types::SemanticDataType::Float,//	Time2Pi,					/*!< TIME2PI */
			types::SemanticDataType::Float,//	Time2PiCos,					/*!< TIME2PICOS */
			types::SemanticDataType::Float,//	Time2PiSin,					/*!< TIME2PISIN */
			types::SemanticDataType::Float,//	Time2PiTan,					/*!< TIME2PITAN */
			types::SemanticDataType::Float,//	Random,						/*!< RANDOM */
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
