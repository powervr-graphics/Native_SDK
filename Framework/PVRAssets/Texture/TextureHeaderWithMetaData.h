/*!*********************************************************************************************************************
\file         PVRAssets/Texture/TextureHeaderWithMetaData.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Contains a class that wraps all information about a Texture, except for the actual data of it.
***********************************************************************************************************************/
#pragma once
#include "PVRAssets/AssetIncludes.h"
#include "PVRAssets/Texture/TextureHeader.h"

namespace pvr {
namespace assets {

/*!********************************************************************************************************************
\brief   This class contains all information a texture has (size, format etc.) and any metadata it may carry. It does
         NOT contain the actual image data the texture caries.
**********************************************************************************************************************/
class TextureHeaderWithMetaData : public TextureHeader
{
public:
	typedef std::map<uint32, std::map<uint32, TextureMetaData>/**/> MetaDataMapType; //!< The type of the metadata container

	/*!********************************************************************************************************************
	\brief    Default Constructor, Empty TextureHeaderWithMetadata.
	**********************************************************************************************************************/
	TextureHeaderWithMetaData();

	/*!*******************************************************************************************************************************
	\brief	Copy Constructor
	\param	rhs Copy from
	**********************************************************************************************************************************/
	TextureHeaderWithMetaData(const TextureHeaderWithMetaData& rhs);

	/*!*******************************************************************************************************************************
	\brief	Assignment operator
	\return	Return a reference to this
	\param	rhs Copy from
	**********************************************************************************************************************************/
	TextureHeaderWithMetaData& operator=(const TextureHeaderWithMetaData& rhs);

	/*!*******************************************************************************************************************************
	\brief	Construct this from the texture header and meta data
	\param	fileHeader File header to construct from
	\param	metaDataCount Number of meta data items in the \p metaData array
	\param	metaData An array of meta data
	**********************************************************************************************************************************/
	TextureHeaderWithMetaData(TextureHeader::Header fileHeader, uint32 metaDataCount, TextureMetaData* metaData);

protected:
	MetaDataMapType	m_metaDataMap; //Map of all the meta data stored for a texture.

public:

	/*!***********************************************************************
	\param[in]	axis The axis to examine.
	\return		Return the orientation of the axis.
	\brief	Get the data orientation for this texture.
	*************************************************************************/
	TextureMetaData::AxisOrientation getOrientation(TextureMetaData::Axis axis) const;

	/*!***********************************************************************
	\return	Return true if it is a bump map.
	\brief Check whether the texture is a bump map or not.
	*************************************************************************/
	bool isBumpMap() const;

	/*!***********************************************************************
	\return	Returns the bump map scale.
	\brief	Gets the bump map scaling value for this texture.
	\description If the texture is not a bump map, 0.0f is returned. If the  texture is a bump map but no
	               meta data is stored to specify its scale, then 1.0f is returned.
	*************************************************************************/
	float getBumpMapScale() const;

	/*!***********************************************************************
	\brief			Get the bump map order relative to rgba.
	\return			Returns bump map order relative to rgba.
	\description	Gets the bump map channel order relative to rgba. For
		example, an RGB texture with bumps mapped to XYZ returns
		'xyz'. A BGR texture with bumps in the order ZYX will also
		return 'xyz' as the mapping is the same: R=X, G=Y, B=Z.
		If the letter 'h' is present in the string, it means that
		the height map has been stored here.
		Other characters are possible if the bump map was created
		manually, but PVRTexLib will ignore these characters. They
		are returned simply for completeness.
	*************************************************************************/
	const std::string getBumpMapOrder() const;

	/*!***********************************************************************
	\return	Returns number of sub textures defined by meta data.
	\brief	Get the number of possible texture atlas members in
	the texture based on the w/h/d and the data size.
		*************************************************************************/
	int getNumberOfTextureAtlasMembers() const;

	/*!***********************************************************************
	\return	Returns a pointer directly to the texture atlas data.
	\brief	Get a pointer to the texture atlas data.
	*************************************************************************/
	const float* getTextureAtlasData() const;

	/*!***********************************************************************
	\return	Returns a string which contains cube map order.
	\brief	Get the cube map face order. Returned string will be in
		the form "ZzXxYy" with capitals representing positive and
			small letters representing negative. I.e. Z=Z-Positive, z=Z-Negative.
	*************************************************************************/
	const std::string getCubeMapOrder() const;

	/*!***********************************************************************
	\param[out]	borderWidth border width
	\param[out]	borderHeight border height
	\param[out]	borderDepth border depth
	\brief	 Get the border size in each dimension for this texture.
	*************************************************************************/
	void getBorder(uint32& borderWidth, uint32& borderHeight, uint32& borderDepth) const;

	/*!***********************************************************************
	\param[in]	fourCC Meta data's fourCC
	\param[in]	key Meta data's key
	\return	Return a copy of the meta data from the texture.
	\brief Get a block of meta data from the texture.
		If the meta data doesn't exist, a block with data size 0 will be returned.
	*************************************************************************/
	const TextureMetaData getMetaData(uint32 fourCC, uint32 key) const;

	/*!***********************************************************************
	\param[in]	fourCC The fourCC of the meta data
	\param[in]	key The ley of the meta data
	\return	Return a whether or not the meta data bock specified exists
	\brief	Check whether or not the specified meta data exists as
	part of this texture header.
	*************************************************************************/
	bool hasMetaData(uint32 fourCC, uint32 key) const;

	/*!***********************************************************************
	\return Return a direct pointer to the MetaData map.
	\brief	Get a pointer directly to the Meta Data Map, to allow users to read out data.
	*************************************************************************/
	const std::map<uint32, std::map<uint32, TextureMetaData>/**/>* getMetaDataMap() const;


	/*!***********************************************************************
	\param[in]	axisOrientation Enum specifying axis and orientation.
	\brief	Sets the data orientation for a given axis in this texture.
		*************************************************************************/
	void setOrientation(TextureMetaData::AxisOrientation axisOrientation);

	/*!***********************************************************************
	\param[in]	bumpScale	Floating point "height" value to scale the bump map.
	\param[in]	bumpOrder	Up to 4 character string, with values x,y,z,h in
		some combination. Not all values need to be present.
		Denotes channel order; x,y,z refer to the
		corresponding axes, h indicates presence of the
		original height map. It is possible to have only some
		of these values rather than all. For example if 'h'
		is present alone it will be considered a height map.
		The values should be presented in RGBA order, regardless
		of the texture format, so a zyxh order in a bgra texture
		should still be passed as 'xyzh'. Capitals are allowed.
		Any character stored here that is not one of x,y,z,h
		or a NULL character	will be ignored when PVRTexLib
		reads the data,	but will be preserved. This is useful
		if you wish to define a custom data channel for instance.
				In these instances PVRTexLib will assume it is simply color data.
	\brief	Sets a texture's bump map data.
	*************************************************************************/
	void setBumpMap(float bumpScale, std::string bumpOrder);

	/*!***********************************************************************
	\param[in]	textureAtlasData	Pointer to an array of atlas data.
	\param[in]	dataSize	Number of floats that the data pointer contains.
	\brief	Sets the texture atlas coordinate meta data for later display.
		It is up to the user to make sure that this texture atlas
		data actually makes sense in the context of the header. It is
		suggested that the "generateTextureAtlas" method in the tools
		is used to create a texture atlas, manually Setting one up is
		possible but should be done with care.
	*************************************************************************/
	void setTextureAtlas(const float32* const textureAtlasData, uint32 dataSize);

	/*!***********************************************************************
	\param[in]	cubeMapOrder	Up to 6 character string, with values
		x,X,y,Y,z,Z in some combination. Not all
		values need to be present. Denotes face
		order; Capitals refer to positive axis
		positions and small letters refer to
		negative axis positions. E.g. x=X-Negative,
		X=X-Positive. It is possible to have only
		some of these values rather than all, as
		long as they are NULL terminated.
		NB: Values past the 6th character are not read.
	\brief	Sets a texture's bump map data.
	*************************************************************************/
	void setCubeMapOrder(std::string cubeMapOrder);

	/*!***********************************************************************
	\param[in] borderWidth Border width
	\param[in] borderHeight Border height
	\param[in] borderDepth Border depth
	\brief	Sets a texture's border size data. This value is subtracted
		from the current texture height/width/depth to Get the valid
		texture data.
	*************************************************************************/
	void setBorder(uint32 borderWidth, uint32 borderHeight, uint32 borderDepth);

	/*!***********************************************************************
	\param[in]		metaData	Meta data block to be added.
	\description	Adds an arbitrary piece of meta data.
	*************************************************************************/
	void addMetaData(const TextureMetaData& metaData);

	/*!***********************************************************************
	\param[in]	fourCC Metadata's fourCC
	\param[in]	key Metadata's key
	\brief Removes a specified piece of meta data, if it exists.
	*************************************************************************/
	void removeMetaData(uint32 fourCC, uint32 key);
};
}
}
