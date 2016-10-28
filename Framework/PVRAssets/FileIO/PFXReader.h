/*!*********************************************************************************************************************
\file         PVRAssets\FileIO\PFXReader.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         An AssetReader that parses and reads PFX effect files into pvr::assets::Effect objects.
***********************************************************************************************************************/
#pragma once
#include "PVRAssets/AssetIncludes.h"
#include "PVRAssets/AssetReader.h"
#include "PVRCore/Stream.h"
#include "PVRAssets/SkipGraph.h"
#include "PVRAssets/Effect.h"
namespace pvr {
namespace assets {
// Forward Declarations
class PFXParserReadContext;
class ContextAPI;


/*!*****************************************************************************************************************
\brief   Helper function for pfx.
*******************************************************************************************************************/
void pfxCreateStringCopy(char8** ppDst, const char8* pSrc);


/*!*****************************************************************************************************************
\brief   Struct for storing PFX file header data.
*******************************************************************************************************************/
struct PFXParserHeader
{
	string      Version;
	string      Description;
	string      Copyright;
};

/*!*****************************************************************************************************************
\brief   Struct for storing PFX data from the texture block.
*******************************************************************************************************************/
struct PFXParserTexture
{
	StringHash name;
	StringHash fileName;
	types::SamplerFilter minFilter, magFilter, mipFilter;
	types::SamplerWrap wrapS, wrapT, wrapR; // either GL_CLAMP or GL_REPEAT
	uint32 width, height;
	uint64 flags;
	bool renderToTexture;
};

/*!*****************************************************************************************************************
\brief   Stores effect texture information.
*******************************************************************************************************************/
struct PFXParserEffectTexture
{
	StringHash      name;     //< Name of texture.
	uint32        number;     //< Texture number to set
};



/*!*****************************************************************************************************************
\brief   Stores effect information.
*******************************************************************************************************************/
struct PfxParserEffect
{
	StringHash            name;
	string                annotation;
	StringHash            vertexShaderName;
	StringHash            fragmentShaderName;
	std::vector<EffectSemantic>   uniforms;
	std::vector<EffectSemantic>   attributes;
	std::vector<PFXParserEffectTexture> textures;
	std::vector<EffectTargetPair>       targets;

	PfxParserEffect();
};

/*!*****************************************************************************************************************
\brief   Stores render pass information.
*******************************************************************************************************************/
struct PfxRenderPass
{
	types::EffectPassType renderPassType;     //< Type of pass.
	types::EffectPassView viewType;       //< View type.
	uint64        formatFlags;      //< Surface Type.
	PfxParserEffect*  effect;         //< Matched pass. Needed but determined from effect block.
	PFXParserTexture* texture;        //< The RTT target for this pass.
	string        nodeName;       //< POD Camera name.
	string        semanticName;     //< Name of this pass.

	PfxRenderPass();
	/*!******************************************************************************
	\brief  Name of this pass.
	\return const string& pass name
	********************************************************************************/
	const string& toString()const { return semanticName; }
};


/*!*****************************************************************************************************************
\brief   Struct for storing PFX data from the shader block.
*******************************************************************************************************************/
struct PFXParserShader
{
	StringHash    name;
	bool      useFileName;
	string      glslFile;
	string      glslBinFile;
	string      glslCode;
	string      glslBin;
	uint32      firstLineNumPos;  // Line number in the text file where this code began; use to correct line-numbers in compiler errors
	uint32      lastLineNumPos; // The final line number of the GLSL block.
};

/*!*****************************************************************************************************************
\brief PFX reader.
*******************************************************************************************************************/
class PfxReader : public AssetReader<Effect>
{
public:
	/*!*****************************************************************************************************************
	\brief  Constructor.
	*******************************************************************************************************************/
	PfxReader();

	/*!*****************************************************************************************************************
	\brief  Destructor.
	*******************************************************************************************************************/
	~PfxReader();

	/*!*******************************************************************************************************************************
	\brief  Check if there any assets left to load
	\return Return true if there is assets left to read, else false
	**********************************************************************************************************************************/
	virtual bool hasAssetsLeftToLoad() { return false; }


	/*!*******************************************************************************************************************************
	\brief  Check if this PFXReader supports multiple assets
	\return Return true if multiple assets supported
	**********************************************************************************************************************************/
	virtual bool canHaveMultipleAssets() { return false; }

	/*!*******************************************************************************************************************************
	\brief  Get list of supported file extensions
	\return Return list of file extensions
	**********************************************************************************************************************************/
	virtual std::vector<std::string>  getSupportedFileExtensions()
	{
		std::vector<string> extensions;
		extensions.push_back("pod");
		return extensions;
	}

	/*!*******************************************************************************************************************************
	\brief  Get this PFXReader name
	\return Return a string of this PFXReader name
	**********************************************************************************************************************************/
	virtual std::string getReaderName() { return "PowerVR assets::PfxReader"; }


	/*!*******************************************************************************************************************************
	\brief  Get this PFXReader version
	\return Return a string of this PFXReader version
	**********************************************************************************************************************************/
	virtual std::string getReaderVersion() { return "1.0"; }

	/*!*****************************************************************************************************************
	\param[in]    pfxSource The PFX data
	\param[out]   returnError error string
	\return       true for success parsing file, error if file doesn't exist or is invalid
	\brief        Parses a PFX script from memory.
	*******************************************************************************************************************/
	bool parseFromMemory(const tchar* const pfxSource, string& returnError);

	/*!*****************************************************************************************************************
	\param[in]      pfxFile   PFX file name
	\param[out]     returnError Error string
	\return       Result::Success For success parsing file, error if file doesn't exist or is invalid
	\brief          Reads the PFX file and calls the parser.
	*******************************************************************************************************************/
	bool parseFromFile(Stream::ptr_type pfxFile, string& returnError);

	/*!*****************************************************************************************************************
	 \brief     Allows the current viewport size to be set. This value
	      is used for calculating relative texture resolutions.
	 \param[in] uiWidth       New viewport width
	 \param[in] uiHeight      New viewport height
	 \return  bool        True on success
	 *******************************************************************************************************************/
	bool setViewportSize(uint32 uiWidth, uint32 uiHeight);

	/*!*****************************************************************************************************************
	\param[in]  TextureName   The name of the texture to find
	\param[in]  uiEffect    The effect block to look for the texture in
	\return   Index in to the effect.Texture array.
	\brief      Returns the index in to the texture array within the effect
	      block where the given texture resides.
	*******************************************************************************************************************/
	uint32 findTextureIndex(const StringHash& TextureName, uint32 uiEffect) const;

	/*!*****************************************************************************************************************
	\param[out] aRequiredRenderPasses Dynamic array of required render passes
	\param[in]  aszActiveEffecstrings Dynamic array containing names of active
	effects in the application
	\return   success of failure
	\brief      Takes an array of strings containing the names of active
	effects for this PFX in a given application and then outputs
	an array of the render passes the application needs to perform that is sorted
	into the order they need to be executed (where [0] is the first to be executed,
	and [n] is the last).
	In addition to determining the order of dependent passes
	(such as POSTPROCESS render passes), this function should check if
	CAMERA passes are referenced by active EFFECT blocks and use this information
	to strip redundant passes.
	*******************************************************************************************************************/
	bool retrieveRenderPassDependencies(std::vector<PfxRenderPass*>& aRequiredRenderPasses,
	                                    std::vector<StringHash>& aszActiveEffecstrings);

	/*!*****************************************************************************************************************
	\brief  Returns the number of render passes within this PFX.
	\return The number of render passes required
	*******************************************************************************************************************/
	uint32 getNumberRenderPasses() const {  return (uint32)m_renderPasses.size();}


	/*!*****************************************************************************************************************
	\brief  Returns the given render pass.
	\param[in]  uiIndex The render pass index.
	\return   A given render pass.
	*******************************************************************************************************************/
	const PfxRenderPass& getRenderPass(uint32 uiIndex) const
	{
		assertion(uiIndex < getNumberRenderPasses(), "PfxReader::getRenderpass index out of range");
		return m_renderPasses[uiIndex];
	}

	/*!*****************************************************************************************************************
	\return Number of fragment shaders.
	\brief  Returns the number of fragment shaders referenced in the PFX.
	*******************************************************************************************************************/
	uint32 getNumberFragmentShaders() const { return (uint32)m_fragmentShaders.size(); }

	/*!*****************************************************************************************************************
	\param[in]  uiIndex   The index of this shader.
	\return   The PFX fragment shader.
	\brief      Returns a given fragment shader.
	*******************************************************************************************************************/
	const PFXParserShader& getFragmentShader(uint32 uiIndex) const
	{
		assertion(uiIndex < getNumberFragmentShaders());
		return m_fragmentShaders[uiIndex];
	}

	/*!*****************************************************************************************************************
	\return     Number of vertex shaders.
	\brief            Returns the number of vertex shaders referenced in the PFX.
	*******************************************************************************************************************/
	uint32 getNumberVertexShaders() const { return (uint32)m_vertexShaders.size();}

	/*!*****************************************************************************************************************
	\param[in]  uiIndex   The index of this shader.
	\return   The PFX vertex shader.
	\brief    Returns a given vertex shader.
	*******************************************************************************************************************/
	const PFXParserShader& getVertexShader(uint32 uiIndex) const
	{
		assertion(uiIndex < getNumberVertexShaders());
		return m_vertexShaders[uiIndex];
	}

	/*!*****************************************************************************************************************
	\return     Number of effects.
	\brief            Returns the number of effects referenced in the PFX.
	*******************************************************************************************************************/
	uint32 getNumberEffects() const
	{
		return (uint32)m_effects.size();
	}

	/*!*****************************************************************************************************************
	\param[in]    Name    Name of the effect.
	\return     int32
	\brief            Returns the index of the given string. Returns -1 on failure.
	*******************************************************************************************************************/
	int32 getEffectId(const StringHash& Name) const;

	/*!*****************************************************************************************************************
	\param[in]    Name    Name of the texture.
	\return     int32
	\brief            Returns the index of the given texture. Returns -1 on failure.
	*******************************************************************************************************************/
	int32 findTextureByName(const StringHash& Name) const;

	//!\cond NO_DOXYGEN
	class CompareShaderNameToName
	{
		CompareShaderNameToName& operator=(const CompareShaderNameToName&);
		const std::string& name;
	public:
		CompareShaderNameToName(const std::string& name) : name(name) {}
		inline bool operator()(const PFXParserShader& shader)
		{
			return shader.name == name;
		}
	};
	//!\endcond

	/*!*******************************************************************************************************************************
	\brief  Find vertex shader by name
	\return Return index to vertex shader if found, else return -1
	\param  name Name of the vertex shader
	**********************************************************************************************************************************/
	int32 findVertexShader(const std::string& name)const
	{
		std::vector<PFXParserShader>::const_iterator found = std::find_if(m_vertexShaders.begin(), m_vertexShaders.end(),
		    CompareShaderNameToName(name));
		return (found != m_vertexShaders.end() ? (uint32)(found - m_vertexShaders.begin()) : -1);
	}

	/*!*****************************************************************************************************************
	\return     Number of effects.
	\brief        Returns the number of textures referenced in the PFX.
	*******************************************************************************************************************/
	uint32 getNumberTextures() const {  return (uint32)m_textures.size(); }

	/*!*****************************************************************************************************************
	\param[in]    uiIndex   The index of this texture
	\return     The PFX texture.
	\brief            Returns a given texture.
	*******************************************************************************************************************/
	const PFXParserTexture* getTexture(uint32 uiIndex) const;

	/*!*****************************************************************************************************************
	\return     The filename for this PFX file
	\brief        Returns the PFX file name associated with this object.
	*******************************************************************************************************************/
	const string& getPFXFileName() const  { return m_fileName;  }

	/*!*****************************************************************************************************************
	\return   Return  an array of post process names
	\brief        Get a list of post process effect names.
	*******************************************************************************************************************/
	const std::vector<string>& getPostProcessNames() const {  return m_postProcessNames;}

	/*!*****************************************************************************************************************
	\return     Return the effect name
	\param[in]      effectId Id of the effect
	\brief        Get the name of the effect
	*******************************************************************************************************************/
	const char* getEffectName(uint32 effectId)const { return getParserEffect(effectId).name.c_str();}

	/*!*****************************************************************************************************************
	\return     Return true if the effect found, else return false
	\param[in]      effectId Id of the Effect
	\param[out]     outEffect Returned Effect
	\brief        Get the effect of a given effect id
	*******************************************************************************************************************/
	bool getEffect(assets::Effect& outEffect, uint32 effectId) const
	{
		return readEffect(outEffect, effectId);
	}

	/*!*****************************************************************************************************************
	\return     Return true if the effect found, else return false
	\param[in]      effectName Name of the Effect
	\param[out]     outEffect Returned Effect
	\brief        Get the effect of a given effect name
	*******************************************************************************************************************/
	bool getEffect(Effect& outEffect, const std::string& effectName) const
	{
		pvr::int32 id = getEffectId(effectName);
		if (id != -1)
		{
			return readEffect(outEffect, id);
		}
		else
		{
			return false;
		}
	}

public:
	enum { ViewportSize = 0xAAAA };
private:
	PFXParserHeader m_header;
	mutable std::vector<PFXParserTexture*> m_textures;
	mutable std::vector<PFXParserShader> m_fragmentShaders;
	std::vector<PFXParserShader> m_vertexShaders;
	std::vector<PfxParserEffect> m_effects;
	std::vector<PfxRenderPass> m_renderPasses;
	string m_fileName;
	PFXParserReadContext* m_context;
	std::vector<string> m_postProcessNames;
	uint32 m_viewportWidth;
	uint32 m_viewportHeight;
	uint32 m_currentEffect;
	SkipGraphRoot<PfxRenderPass*> m_renderPassSkipGraph;

	/*!*****************************************************************************************************************
	\param[in]    uiIndex   The index of this effect.
	\return     The PFX effect.
	\brief            Returns a given effect.
	*******************************************************************************************************************/
	const PfxParserEffect& getParserEffect(uint32 uiIndex) const
	{
		assertion(uiIndex < getNumberEffects());
		return m_effects[uiIndex];
	}

	bool readNextAsset(Effect& asset);
	bool readEffect(Effect& asset, uint32 id) const;

	bool parse(string& returnError);

	void reduceWhitespace(char8* line);

	bool getEndTag(const char8* tagName, int32 startLine, int32* endLine);

	string findParameter(tchar* sourceString, const string& parameterTag, const string& delimiter);

	bool readStringToken(tchar* source, string& output, string& errorStr, int32 line, const tchar* caller);

	bool parseHeader(int32 startLine, int32 endLine, string& errorOut);

	bool parseTextures(int32 startLine, int32 endLine, string&  errorOut);

	bool parseTexture(int32 startLine, int32 endLine, string&    returnError);

	bool parseTarget(int32 startLine, int32 endLine, string&    returnError);

	bool parseGenericSurface(int32 startLine, int32 endLine, PFXParserTexture& params, std::vector<StringHash>& knownCmds,
	                         const char8* caller, string&    returnError);

	bool parseShader(int32 startLine, int32 endLine, string& returnError, PFXParserShader& shader,
	                 const tchar* const blockName);
	bool parseSemantic(EffectSemantic& semantic, const int32 startLine, string& returnError);

	bool parseEffect(PfxParserEffect& effect, const int32 startLine, const int32 endLine, string& returnError);

	bool determineRenderPassDependencies(string& errorOut);

	void addRenderPassNodeDependencies(std::vector<PfxRenderPass*>& requiredRenderPasses,
	                                   SkipGraphNode<PfxRenderPass*>& renderPassNode);
};

}//namespace assets
}//namespace pvr
