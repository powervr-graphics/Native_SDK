/*!*********************************************************************************************************************
\file         PVRAssets/SamplerDescription.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        The creation parameters for a sampler.
***********************************************************************************************************************/
#pragma once
#include "PVRCore/Types.h"
namespace pvr {
namespace assets {

/*!*********************************************************************************************************************
\brief Object describing the state of a sampler. Also used to construct a full-blown Sampler object.
***********************************************************************************************************************/
struct SamplerCreateParam
{

	/*!*********************************************************************************************************************
	\brief Default constructor.
	\brief Creates a Sampler with following configs. Can be edited after construction
           magnification filter: nearest, minifictation filter: nearest, mipmap filter: linear,wrap UVW: Repeat
           no comparison, no lod-bias, no anisotropy. 
           
	***********************************************************************************************************************/
	SamplerCreateParam() :
		magnificationFilter(SamplerFilter::Linear), minificationFilter(SamplerFilter::Nearest),
		mipMappingFilter(SamplerFilter::Linear),wrapModeU(SamplerWrap::Repeat), 
		wrapModeV(SamplerWrap::Repeat), wrapModeW(SamplerWrap::Repeat), compareMode(ComparisonMode::None),
		anisotropyMaximum(0.0f), lodBias(0.0f), lodMinimum(-1000.0f), lodMaximum(1000.0f), 
		borderColor(BorderColor::TransparentBlack){}

	/*!*********************************************************************************************************************
	\brief Constructor that sets the filters. Set the filtering mode explicitly. Wrap. 
           Creates a Sampler with wrap:Repeat, no comparison, no lod bias, no anisotropy. Can be edited after construction.
	\param[in] miniFilter  Minification filter: Nearest or Linear
	\param[in] magniFilter  Magnification filter: Nearest or Linear
	\param[in] mipMapFilter Mipmap interpolation filter: Nearest or Linear
	***********************************************************************************************************************/
	SamplerCreateParam(SamplerFilter::Enum magniFilter, SamplerFilter::Enum miniFilter,
		SamplerFilter::Enum mipMapFilter = SamplerFilter::Linear) :
		magnificationFilter(magniFilter), minificationFilter(miniFilter), 
		mipMappingFilter(mipMapFilter),wrapModeU(SamplerWrap::Repeat), wrapModeV(SamplerWrap::Repeat), 
		wrapModeW(SamplerWrap::Repeat), compareMode(ComparisonMode::None),anisotropyMaximum(0.0f),
		lodBias(0.0f), lodMinimum(-1000.0f), lodMaximum(1000.0f), borderColor(BorderColor::TransparentBlack)
	{}

	SamplerFilter::Enum magnificationFilter; //!< Texture Magnification interpolation filter. Nearest or Linear. Default Nearest.
	SamplerFilter::Enum minificationFilter; //!< Texture Minification interpolation filter. Nearest or Linear. Default Nearest.
	SamplerFilter::Enum mipMappingFilter; //!< Texture Mipmap interpolation filter. Nearest, Linear or None (for no mipmaps). Default Linear.
	SamplerWrap::Enum wrapModeU; //!< Texture Wrap mode, U (x) direction (Repeat, Mirror, MirrorRepeat, Border, Clamp). Default Repeat.
	SamplerWrap::Enum wrapModeV; //!< Texture Wrap mode, V (y) direction (Repeat, Mirror, MirrorRepeat, Border, Clamp). Default Repeat.
	SamplerWrap::Enum wrapModeW; //!< Texture Wrap mode, W (z) direction (Repeat, Mirror, MirrorRepeat, Border, Clamp). Default Repeat.
	ComparisonMode::Enum compareMode; //!< Comparison mode for comparison samplers. Default None.
	float32 anisotropyMaximum; //!< Texture anisotropic filtering. Default 0.
	float32 lodBias;           //!< Texture level-of-detail bias (bias of mipmap to select). Default 0.
	float32 lodMinimum;        //!< Texture minimum level-of-detail (mipmap). Default 0.
	float32 lodMaximum;        //!< Texture maximum level-of-detail (mipmap). Default 0.
	BorderColor::Enum borderColor; //!< Texture border color. Only used with Border wrap mode.
};

}
}
