/*!*********************************************************************************************************************
\file         PVRAssets/Model.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains the class representing an entire Scene, or Model.
***********************************************************************************************************************/
#pragma once
#include "PVRAssets/AssetIncludes.h"
#include "PVRAssets/Asset.h"
#include "PVRAssets/Model/Camera.h"
#include "PVRAssets/Model/Animation.h"
#include "PVRAssets/Model/Light.h"
#include "PVRAssets/Model/Mesh.h"
#include "PVRAssets/FileIO/PODReader.h"

/*!*********************************************************************************************************************
\brief 			Main namespace of the PowerVR Framework.
***********************************************************************************************************************/
namespace pvr {
class Stream;
/*!*********************************************************************************************************************
\brief 			Main namespace of the PVRAssets library.
***********************************************************************************************************************/
namespace assets {

class Model;
class Mesh;
class Camera;
class Light;
typedef RefCountedResource<Model> ModelHandle;
typedef RefCountedResource<Mesh> MeshHandle;
typedef RefCountedResource<Camera> CameraHandle;
typedef RefCountedResource<Light> LightHandle;

/*!*********************************************************************************************************************
\brief 			The Model class represents an entire Scene, or Model. It is mainly a Node structure, allowing various different
                kinds of data to be stored in the Nodes.
                The class contains a tree-like structure of Nodes. Each Node can be a Mesh node (containing a Mesh), Camera
                node or Light node. The tree-structure assumes transformational hierarchy (as usual).
                Transformations are expressed through Animation objects (a static transform is an animation with a single frame)
                There is an implicit order in the nodes - First in the array the Mesh nodes will be laid out, then Camera and
                Light nodes.
***********************************************************************************************************************/
class Model : public Asset<Model>
{
public:

	const FreeValue* getModelSemantic(const StringHash& semantic) const
	{
		auto it = m_data.semantics.find(semantic);
		if (it == m_data.semantics.end())
		{
			return NULL;
		}
		return &it->second;
	}
	const RefCountedResource<void>& getUserDataPtr() const
	{
		return this->m_data.userDataPtr;
	}
	RefCountedResource<void> getUserDataPtr()
	{
		return this->m_data.userDataPtr;
	}
	void setUserDataPtr(const RefCountedResource<void>& ptr)
	{
		m_data.userDataPtr = ptr;
	}
	typedef assets::Mesh Mesh;
	/*!*********************************************************************************************************************
	\brief 			The Node represents a Mesh, Camera or Light.
	                  A Node has its own parenting, material, animation and custom user data. The tree-structure assumes
	                transformational hierarchy (as usual), so parent transformations should be applied to children.
	                Transformations are expressed through Animation objects (a static transform is an animation with a single
	                frame).
	                Note: Node ID and MeshID can sometimes be confusing: They will always be the same (when a MeshID makes sense)
	                because Meshes are always laid out first in the internal list of Nodes.
	***********************************************************************************************************************/
	class Node
	{
	public:
		/*!************************************************************************************************************
		\brief Raw internal structure of the Node.
		***************************************************************************************************************/
		struct InternalData
		{
			StringHash name;		//!< Name of object
			int32 objectIndex;		//!< Index into mesh, light or camera array, depending on which object list contains this Node
			int32 materialIndex;	//!< Index of material used on this mesh
			int32 parentIndex;		//!< Index into Node array; recursively apply ancestor's transforms after this instance's.
			Animation animation;
			UCharBuffer	userData;

			InternalData() : objectIndex(-1), materialIndex(-1), parentIndex(-1) {}
		};

	public:
		/*!**************************************************************************************************************
		\brief Get which Mesh, Camera or Light this object refers to.
		\return The index of the Mesh, Camera or Light array of this node (depending on its type)
		****************************************************************************************************************/
		int32 getObjectId() const {	return m_data.objectIndex; 	}

		/*!***************************************************************************************************************
		\brief Get this Node's name.
		\return The name of this object
		*****************************************************************************************************************/
		const StringHash& getName() const { return m_data.name; }

		/*!***************************************************************************************************************
		\brief Get this Node's parent id.
		\return The ID of this Node's parent Node.
		*****************************************************************************************************************/
		int32 getParentID() const { return m_data.parentIndex; }

		/*!***************************************************************************************************************
		\brief Get this Node's material id.
		\return The ID of this Node's Material
		*****************************************************************************************************************/
		int32 getMaterialIndex() const { return m_data.materialIndex; }

		/*!***************************************************************************************************************
		\brief Get this Node's animation.
		\return The animation of this Node
		*****************************************************************************************************************/
		const Animation& getAnimation() const { return m_data.animation; }

		/*!***************************************************************************************************************
		\brief Get this Node's user data.
		\return The user data of this Node
		*****************************************************************************************************************/
		const byte* getUserData() const { return m_data.userData.data(); }

		/*!***************************************************************************************************************
		\brief Get the size of this Node's user data.
		\return Return The size in bytes of the user data of this Node
		*****************************************************************************************************************/
		uint32 getUserDataSize() const { return (uint32)m_data.userData.size(); }

		/*!***************************************************************************************************************
		\brief Set mesh id. Must correlate with the actual position of this node in the data.
		\param index The id to set the index of this node.
		*****************************************************************************************************************/
		void setIndex(int32 index) { m_data.objectIndex = index; }

		/*!***************************************************************************************************************
		\brief Set the name of this node.
		\param name The string to set this node's name to.
		*****************************************************************************************************************/
		void setName(const StringHash& name) { m_data.name = name; }

		/*!***************************************************************************************************************
		\brief Set the parent of this node.
		\param parentID the ID of this node's parent
		*****************************************************************************************************************/
		void setParentID(int32 parentID) { m_data.parentIndex = parentID; }

		/*!***************************************************************************************************************
		\brief Set the animation of this node.
		\param animation The animation of this node. A copy of the animation object will be created and stored directly.
		*****************************************************************************************************************/
		void setAnimation(const Animation& animation) { m_data.animation = animation; }

		/*!***************************************************************************************************************
		\brief Set the user data of this node. A bit copy of the data will be made.
		\param size The size, in bytes, of the data
		\param data A pointer from which (size) data will be copied.
		*****************************************************************************************************************/
		void setUserData(uint32 size, const byte* data)
		{
			m_data.userData.resize(size);
			memcpy(m_data.userData.data(), data, size);
		}

		/*!***************************************************************************************************************
		\brief Get a reference to the internal data of this object. Handle with care.
		\return Return a reference to the internal data of this object.
		*****************************************************************************************************************/
		InternalData& getInternalData() { return m_data; }

		/*!***************************************************************************************************************
		\brief Get a const reference to the data object carrying all internal data of this model.
		*****************************************************************************************************************/
		const InternalData& getInternalData() const { return m_data; }

	private:
		InternalData m_data;
	};

	/*!******************************************************************************************************************
	\brief Internal class which stores Texture information of the model (name).
	********************************************************************************************************************/
	class Texture
	{
	public:
		/*!***************************************************************************************************************
		\brief Get the name of the texture.
		    \return Return the texture name
		*****************************************************************************************************************/
		const pvr::StringHash& getName() const { return name; }

		/*!***************************************************************************************************************
		\brief Get a reference to the name of the texture.
		    \return Return reference to the texture name
		*****************************************************************************************************************/
		pvr::StringHash& getName() { return name; }

		/*!***************************************************************************************************************
		\brief Set the name of the texture.
		\param name The string to set this texture name to.
		*****************************************************************************************************************/
		void setName(const StringHash& name) { this->name = name; }

	private:
		pvr::StringHash name;
	};

	/*!*********************************************************************************************************************
	\brief Class which stores model material info.
	***********************************************************************************************************************/
	class Material
	{
	public:
		enum BlendFunction
		{
			BlendFuncZero = 0,              //!< BlendFunction (Zero)
			BlendFuncOne,                   //!< BlendFunction (One)
			BlendFuncFactor,                //!< BlendFunction (Factor)
			BlendFuncOneMinusBlendFactor,   //!< BlendFunction (One Minus Blend Factor)

			BlendFuncSrcColor = 0x0300,     //!< BlendFunction (source Color)
			BlendFuncOneMinusSrcColor,      //!< BlendFunction (One Minus Source Color)
			BlendFuncSrcAlpha,              //!< BlendFunction (Source Alpha)
			BlendFuncOneMinusSrcAlpha,      //!< BlendFunction (One Minus Source Alpha)
			BlendFuncDstAlpha,              //!< BlendFunction (Destination Alpha)
			BlendFuncOneMinusDstAlpha,      //!< BlendFunction (One Minus Destination Alpha)
			BlendFuncDstColor,              //!< BlendFunction (Destination Alpha)
			BlendFuncOneMinusDstColor,      //!< BlendFunction (One Minus Destination Color)
			BlendFuncSrcAlphaSaturate,      //!< BlendFunction (Source Alpha Saturate)

			BlendFuncConstantColor = 0x8001,//!< BlendFunction (Constant Color)
			BlendFuncOneMinusConstantColor, //!< BlendFunction (One Minus Constant Color)
			BlendFuncConstantAlpha,         //!< BlendFunction (Constant Alpha)
			BlendFuncOneMinusConstantAlpha  //!< BlendFunction (One Minus Constant Alpha)
		};

		enum BlendOperation
		{
			BlendOpAdd = 0x8006,			//!< Blend Operation (Add)
			BlendOpMin,                     //!< Blend Operation (Min)
			BlendOpMax,                     //!< Blend Operation (Max)
			BlendOpSubtract = 0x800A,       //!< Blend Operation (Subtract)
			BlendOpReverseSubtract          //!< Blend Operation (Reverse Subtract)
		};

		/*!************************************************************************************************************
		\brief Raw internal structure of the Material.
		***************************************************************************************************************/
		struct InternalData
		{
			StringHash name;					//!<Name of the material
			int32 diffuseTextureIndex;			//!<Texture index (in the Model) of a texture to use for Diffuse
			int32 ambientTextureIndex;			//!<Texture index (in the Model) of a texture to use for Ambient
			int32 specularColorTextureIndex;	//!<Texture index (in the Model) of a texture to use for Specular
			int32 specularLevelTextureIndex;	//!<Texture index (in the Model) of a texture to use for Specular level
			int32 bumpMapTextureIndex;			//!<Texture index (in the Model) of a texture to use for NormalMap
			int32 emissiveTextureIndex;			//!<Texture index (in the Model) of a texture to use for Emissive
			int32 glossinessTextureIndex;		//!<Texture index (in the Model) of a texture to use for Glossiness
			int32 opacityTextureIndex;			//!<Texture index (in the Model) of a texture to use for Opacity
			int32 reflectionTextureIndex;		//!<Texture index (in the Model) of a texture to use for Reflection
			int32 refractionTextureIndex;		//!<Texture index (in the Model) of a texture to use for Refraction
			float32 opacity;					//!<Opacity (alpha)
			glm::vec3 ambient;					//!<Ambient color
			glm::vec3 diffuse;					//!<Diffuse color
			glm::vec3 specular;					//!<Specular color
			float32 shininess;					//!<Shininess color
			StringHash effectFile;				//!<Effect filename if using an effect
			StringHash effectName;				//!<Effect name (in the filename) if using an effect

			BlendFunction	blendSrcRGB;		//!<Blend function for Source Color
			BlendFunction	blendSrcA;			//!<Blend function for Source Alpha
			BlendFunction	blendDstRGB;		//!<Blend function for Destination Color
			BlendFunction	blendDstA;			//!<Blend function for Destination Alpha
			BlendOperation	blendOpRGB;			//!<Blend operation for Color
			BlendOperation	blendOpA;			//!<Blend operation for Alpha
			float32 blendColor[4];				//!<Blend color (RGBA)
			float32 blendFactor[4];				//!<Blend factor (RGBA)

			uint32	flags;						//!<Material flags

			UCharBuffer userData;				//!<Raw user data

			/*!***************************************************************************************************************
			\brief Default constructor.
			*****************************************************************************************************************/
			InternalData() : diffuseTextureIndex(-1), ambientTextureIndex(-1), specularColorTextureIndex(-1), specularLevelTextureIndex(-1),
				bumpMapTextureIndex(-1), emissiveTextureIndex(-1), glossinessTextureIndex(-1), opacityTextureIndex(-1), reflectionTextureIndex(-1),
				refractionTextureIndex(-1), opacity(1), shininess(0), blendSrcRGB(BlendFuncOne), blendSrcA(BlendFuncOne), blendDstRGB(BlendFuncZero),
				blendDstA(BlendFuncZero), blendOpRGB(BlendOpAdd), blendOpA(BlendOpAdd), flags(0)
			{
				ambient[0] = ambient[1] = ambient[2] = 0.0f;
				diffuse[0] = diffuse[1] = diffuse[2] = 1.0f;
				specular[0] = specular[1] = specular[2] = 0.0f;
				memset(&blendColor[0], 0, sizeof(blendColor[0]));
				memset(&blendFactor[0], 0, sizeof(blendFactor[0]));
			}

		};

	public:
		/*!***************************************************************************************************************
		        \brief Get material attribute.
		        \return Material attribute
		*****************************************************************************************************************/
		bool hasSemantic(const StringHash& semantic) const
		{
			static bool first_run_warning = true;
			if (first_run_warning)
			{
				Log(Log.Warning, "pvr::assets::Model::Material::hasSemantic : Only Texture semantic is supported. If other semantics are tested,"
				    "it will always return false. This may cause issues with Pipeline Creation in the render manager.");
				first_run_warning = false;
			}
			return getTextureIndex(semantic) != -1;
		}

		/*!***************************************************************************************************************
		\brief Get material ambient.
		    \return Material ambient
		*****************************************************************************************************************/
		const glm::vec3& getAmbient() const { return m_data.ambient; }

		/*!***************************************************************************************************************
		\brief Get material diffuse.
		    \return Material diffuse
		*****************************************************************************************************************/
		const glm::vec3& getDiffuse() const { return m_data.diffuse; }

		/*!***************************************************************************************************************
		\brief Get material specular.
		    \return Material specular
		*****************************************************************************************************************/
		const glm::vec3& getSpecular() const { return m_data.specular; }

		/*!***************************************************************************************************************
		\brief Get material shininess.
		    \return  Material shininess
		*****************************************************************************************************************/
		float32 getShininess() const { return m_data.shininess; }

		/*!***************************************************************************************************************
		\brief Set material effect name.
		    \param[in] name Material effect name
		*****************************************************************************************************************/
		void setEffectName(const StringHash& name) { m_data.effectName = name; }

		/*!***************************************************************************************************************
		\brief Set material effect file name.
		    \param[in] name Effect file name
		*****************************************************************************************************************/
		void setEffectFile(const StringHash& name) { m_data.effectFile = name; }


		/*!***************************************************************************************************************
		\brief Get the diffuse color texture's index in the scene.
		\return Return the diffuse texture index
		*****************************************************************************************************************/
		int32 getTextureIndex(const StringHash& semantic) const
		{

			switch (semantic.getHash())
			{
			case HashCompileTime<'D', 'I', 'F', 'F', 'U', 'S', 'E', 'M', 'A', 'P'>::value: return getDiffuseTextureIndex();
			case HashCompileTime<'D', 'I', 'F', 'F', 'U', 'S', 'E', 'T', 'E', 'X'>::value: return getDiffuseTextureIndex();
			case HashCompileTime<'D', 'I', 'F', 'F', 'U', 'S', 'E', 'T', 'E', 'X', 'T', 'U', 'R', 'E'>::value: return getDiffuseTextureIndex();
			case HashCompileTime<'N', 'O', 'R', 'M', 'A', 'L', 'M', 'A', 'P'>::value: return getBumpMapTextureIndex();
			case HashCompileTime<'N', 'O', 'R', 'M', 'A', 'L', 'T', 'E', 'X'>::value: return getBumpMapTextureIndex();
			case HashCompileTime<'N', 'O', 'R', 'M', 'A', 'L', 'T', 'E', 'X', 'T', 'U', 'R', 'E'>::value: return getBumpMapTextureIndex();
			case HashCompileTime<'S', 'P', 'E', 'C', 'U', 'L', 'A', 'R', 'C', 'O', 'L', 'O', 'R', 'M', 'A', 'P'>::value: return getSpecularColorTextureIndex();
			case HashCompileTime<'S', 'P', 'E', 'C', 'U', 'L', 'A', 'R', 'C', 'O', 'L', 'O', 'R', 'T', 'E', 'X'>::value: return getSpecularColorTextureIndex();
			case HashCompileTime<'S', 'P', 'E', 'C', 'U', 'L', 'A', 'R', 'C', 'O', 'L', 'O', 'R', 'T', 'E', 'X', 'T', 'U', 'R', 'E'>::value: return getSpecularColorTextureIndex();
			case HashCompileTime<'S', 'P', 'E', 'C', 'U', 'L', 'A', 'R', 'L', 'E', 'V', 'E', 'L', 'M', 'A', 'P'>::value: return getSpecularColorTextureIndex();
			case HashCompileTime<'S', 'P', 'E', 'C', 'U', 'L', 'A', 'R', 'L', 'E', 'V', 'E', 'L', 'T', 'E', 'X'>::value: return getSpecularColorTextureIndex();
			case HashCompileTime<'S', 'P', 'E', 'C', 'U', 'L', 'A', 'R', 'L', 'E', 'V', 'E', 'L', 'T', 'U', 'R', 'E'>::value: return getSpecularColorTextureIndex();
			case HashCompileTime<'E', 'M', 'I', 'S', 'S', 'I', 'V', 'E', 'M', 'A', 'P'>::value: return getSpecularColorTextureIndex();
			case HashCompileTime<'G', 'L', 'O', 'S', 'S', 'I', 'N', 'E', 'S', 'S', 'M', 'A', 'P'>::value: return getSpecularColorTextureIndex();
			case HashCompileTime<'O', 'P', 'A', 'C', 'I', 'T', 'Y', 'M', 'A', 'P'>::value: return getSpecularColorTextureIndex();
			case HashCompileTime<'R', 'E', 'F', 'L', 'E', 'C', 'T', 'I', 'O', 'N', 'M', 'A', 'P'>::value: return getSpecularColorTextureIndex();
			case HashCompileTime<'R', 'E', 'F', 'R', 'A', 'C', 'T', 'I', 'O', 'N', 'M', 'A', 'P'>::value: return getSpecularColorTextureIndex();
			}
			return -1;
		}

		/*!***************************************************************************************************************
		\brief Get the diffuse color texture's index in the scene.
		\return Return the diffuse texture index
		*****************************************************************************************************************/
		int32 getDiffuseTextureIndex() const { return m_data.diffuseTextureIndex; }


		/*!***************************************************************************************************************
		\brief Return the ambient color texture's index in the scene.
		*****************************************************************************************************************/
		int32 getAmbientTextureIndex() const { return m_data.ambientTextureIndex; }

		/*!***************************************************************************************************************
		\brief Get the specular color texture's index in the scene.
		    \return Return the specular color texture index
		*****************************************************************************************************************/
		int32 getSpecularColorTextureIndex() const { return m_data.specularColorTextureIndex; }

		/*!***************************************************************************************************************
		\brief Get the specular level texture's index in the scene.
		    \return Return the specular level texture index
		*****************************************************************************************************************/
		int32 getSpecularLevelTextureIndex() const { return m_data.specularLevelTextureIndex; }

		/*!***************************************************************************************************************
		\brief Get bumpmap texture index.
		    \return Return the bumpmap texture index
		*****************************************************************************************************************/
		int32 getBumpMapTextureIndex() const { return m_data.bumpMapTextureIndex; }

		/*!***************************************************************************************************************
		\brief Get emissive texture's index in the scene
		    \return Return the emissive texture index
		*****************************************************************************************************************/
		int32 getEmissiveTextureIndex() const { return m_data.emissiveTextureIndex; }

		/*!***************************************************************************************************************
		\brief Get glossiness texture's index in the scene.
		    \return Return the glossiness texture index
		*****************************************************************************************************************/
		int32 getGlossinessTextureIndex() const { return m_data.glossinessTextureIndex; }

		/*!***************************************************************************************************************
		\brief Get opacity texture's index in the scene.
		    \return Return the opacity texture index
		*****************************************************************************************************************/
		int32 getOpacityTextureIndex() const { return m_data.opacityTextureIndex; }

		/*!***************************************************************************************************************
		\brief Get reflection texture's index in the scene.
		    \return Return the reflection texture index
		*****************************************************************************************************************/
		int32 getReflectionTextureIndex() const { return m_data.reflectionTextureIndex; }

		/*!***************************************************************************************************************
		\brief Return refraction texture's index in the scene.
		    \return Return the refraction texture index
		*****************************************************************************************************************/
		int32 getRefractionTextureIndex() const { return m_data.refractionTextureIndex; }

		/*!***************************************************************************************************************
		\brief Get this material name.
		\return return the material name
		*****************************************************************************************************************/
		const StringHash& getName() const { return m_data.name; }

		/*!***************************************************************************************************************
		\brief Get this material opacity.
		    \return Return the material opacity
		*****************************************************************************************************************/
		float32 getOpacity() const { return m_data.opacity; }

		/*!***************************************************************************************************************
		\brief Get this material effect file name.
		    \return Retuurn Material effect file name
		*****************************************************************************************************************/
		const StringHash& getEffectFile() const { return m_data.effectFile; }

		/*!***************************************************************************************************************
		\brief Get this material effect name.
		    \return Return material effect name
		*****************************************************************************************************************/
		const StringHash& getEffectName() const { return m_data.effectName; }

		/*!***************************************************************************************************************
		\brief Get the blend function for Source Color.
		    \return Return source color blend function
		*****************************************************************************************************************/
		BlendFunction getBlendSrcRGB() const { return m_data.blendSrcRGB; }

		/*!***************************************************************************************************************
		\brief Get the blend function for Source Alpha.
		    \return Return source alpha blend function
		*****************************************************************************************************************/
		BlendFunction getBlendSrcA() const { return m_data.blendSrcA; }

		/*!***************************************************************************************************************
		\brief Get the blend function for Destination Color.
		    \return Return destination color blend function
		*****************************************************************************************************************/
		BlendFunction getBlendDstRGB() const { return m_data.blendDstRGB; }

		/*!***************************************************************************************************************
		\brief Get the blend function for Destination Alpha.
		    \return Return destination alpha blend function
		*****************************************************************************************************************/
		BlendFunction getBlendDstA() const { return m_data.blendDstA; }

		/*!***************************************************************************************************************
		\brief Get the blend operation for Color.
		    \return Return the color's blend operator
		*****************************************************************************************************************/
		BlendOperation getBlendOpRGB() const { return m_data.blendOpRGB; }

		/*!***************************************************************************************************************
		\brief Return the blend operation for Alpha.
		    \return Return the alpha's blend operator
		*****************************************************************************************************************/
		BlendOperation getBlendOpA() const { return m_data.blendOpA; }

		/*!***************************************************************************************************************
		\brief Get the blend color.
		    \return Return blend color
		*****************************************************************************************************************/
		const float32* getBlendColor() const { return m_data.blendColor; }

		/*!***************************************************************************************************************
		\brief Return the blend factor.
		    \return Return blend factor
		*****************************************************************************************************************/
		const float32* getBlendFactor() const { return m_data.blendFactor; }

		/*!***************************************************************************************************************
		\brief Return a reference to the material's internal data structure. Handle with care.
		    \return Return reference to the internal data
		*****************************************************************************************************************/
		InternalData& getInternalData() { return m_data; }

	private:
		//uint32	flags;
		UCharBuffer userData;
		InternalData m_data;
	};

public:

	/*!*********************************************************************************************************************
	\brief Struct containing the internal data of the Model.
	***********************************************************************************************************************/
	struct InternalData
	{
		pvr::ContiguousMap<StringHash, FreeValue> semantics; //

		float32	clearColor[3];			//!< Background color
		float32	ambientColor[3];		//!< Ambient color

		std::vector<Mesh> meshes;		//!< Mesh array. Any given mesh can be referenced by multiple Nodes.
		std::vector<Camera> cameras;	//!< Camera array. Any given mesh can be referenced by multiple Nodes.
		std::vector<Light> lights;		//!< Light array. Any given mesh can be referenced by multiple Nodes.
		std::vector<Texture> textures;	//!< Textures in this Model
		std::vector<Material> materials;//!< Materials in this Model
		std::vector<Node> nodes;		//!< Nodes array. The nodes are sorted thus: First Mesh Nodes, then Light Nodes, then Camera nodes.

		uint32 numMeshNodes;			//!< Number of items in the nodes array which are Meshes
		uint32 numLightNodes;			//!< Number of items in the nodes array which are Meshes
		uint32 numCameraNodes;			//!< Number of items in the nodes array which are Meshes

		uint32 numFrames;				//!< Number of frames of animation
		float32 currentFrame;			//!< Current frame in the animation
		uint32 FPS;						//!< The frames per second the animation should be played at

		UCharBuffer userData;			//!< Custom raw data stored by the user

		float32	units;					//!< Unit scaling
		uint32 flags;					//!< Flags
		RefCountedResource<void> userDataPtr; //!< Can be used to store any kind of data that the user wraps in a refcounted resource

		/*!***************************************************************************************************************
		\brief ctor.
		*****************************************************************************************************************/
		InternalData() : numMeshNodes(0), numLightNodes(0), numCameraNodes(0), numFrames(0), currentFrame(0), FPS(30), units(1), flags(0)
		{
			memset(clearColor, 0, sizeof(clearColor));
			memset(ambientColor, 0, sizeof(ambientColor));
		}
	};
public:
	void releaseVertexData()
	{
		for (uint32 i = 0; i < getNumMeshes(); ++i)
		{
			releaseVertexData(i);
		}
	}

	void releaseVertexData(uint32 meshId)
	{
		Mesh& mesh = getMesh(meshId);
		for (pvr::uint32 i = 0; i < mesh.getNumDataElements(); ++i)
		{
			mesh.removeData(i);
		}
		mesh.getFaces().setData(0, 0);
	}

	/*!*********************************************************************************************************************
	\brief Return the world-space position of a light. Corresponds to the Model's current frame of animation.
	\param lightId The node for which to return the world matrix.
	\return Return The world matrix of (nodeId).
	***********************************************************************************************************************/
	glm::vec3 getLightPosition(uint32 lightId) const;

	/*!*********************************************************************************************************************
	\brief Return the model-to-world matrix of a node. Corresponds to the Model's current frame of animation.
	         This version will store a copy of the matrix in an internal cache so that repeated calls for it will use the cached
	       copy of it. Will also store the cached versions of all parents of this node, or use cached versions of them if they
	       exist. Use this if you have long hierarchies and/or repeated calls per frame.
	\param nodeId The node for which to return the world matrix.
	\return Return The world matrix of (nodeId).
	***********************************************************************************************************************/
	glm::mat4x4 getWorldMatrix(uint32 nodeId) const;

	/*!*********************************************************************************************************************
	\brief Return the model-to-world matrix of a node. Corresponds to the Model's current frame of animation.
	         This version will not use caching and will recalculate the matrix. Faster if the matrix is only used a few times.
	\param nodeId The node for which to return the world matrix
	\return return The world matrix of (nodeId)
	***********************************************************************************************************************/
	glm::mat4x4 getWorldMatrixNoCache(uint32 nodeId) const;

	/*!*********************************************************************************************************************
	\brief Return the model-to-world matrix of a specified bone. Corresponds to the Model's current frame of animation.
	         This version will use caching.
	\param skinNodeID The node for which to return the world matrix
	\param boneId The bone for which to return the world matrix
	\return Return The world matrix of (nodeId, boneID)
	***********************************************************************************************************************/
	glm::mat4x4 getBoneWorldMatrix(uint32 skinNodeID, uint32 boneId) const;

	/*!*********************************************************************************************************************
	\brief Initialize the cache. Call this after changing the model data. It is automatically called by PODReader when reading a POD
	       file.
	***********************************************************************************************************************/
	void initCache();

	/*!*********************************************************************************************************************
	\brief Release the memory of the cache.
	***********************************************************************************************************************/
	void destroyCache();

	/*!*********************************************************************************************************************
	\brief Modify a node's transformation then flush the cache. No effect if cache is uninitialized
	***********************************************************************************************************************/
	void flushCache();

	/*!*********************************************************************************************************************
	\brief Get the clear color (background) (float array R,G,B,A).
	***********************************************************************************************************************/
	const float32* getBackgroundColor() const { return m_data.clearColor; }

	/*!*********************************************************************************************************************
	\brief Get the number of distinct camera objects.
	         May be different than  the actual number of Camera Instances (Nodes).
	\return Return The number of distinct camera objects.
	***********************************************************************************************************************/
	uint32 getNumCameras() const { return (uint32)m_data.cameras.size(); }

	/*!*********************************************************************************************************************
	\brief Get the number of Camera nodes in this model
	\return Return The number of Camera instances (Nodes) in this Model.
	***********************************************************************************************************************/
	uint32 getNumCameraNodes() const { return getNumCameras(); /* Will be changed at a future revision */}

	/*!*********************************************************************************************************************
	\brief Get a Camera from this model
	\param cameraIndex The index of the camera. Valid values (0 to getNumCameras()-1)
	\return Return the camera
	***********************************************************************************************************************/
	const Camera& getCamera(uint32 cameraIndex) const
	{
		assertion(cameraIndex < getNumCameras() ,  "Invalid camera index");
		return m_data.cameras[cameraIndex];
	}
	/*!*********************************************************************************************************************
	\brief Get a Camera from this model
	\param cameraIndex The index of the camera. Valid values (0 to getNumCameras()-1)
	\return Return the camera
	***********************************************************************************************************************/
	Camera& getCamera(uint32 cameraIndex)
	{
		assertion(cameraIndex < getNumCameras() ,  "Invalid camera index");
		return m_data.cameras[cameraIndex];
	}

	/*!*********************************************************************************************************************
	\brief Get a specific CameraNode.
	\param[in] cameraNodeIndex The Index of a Camera Node. It is not the same as the NodeID. Valid values: (0 .. getNumCameraNodes()-1)
	\return Return The Camera Node
	***********************************************************************************************************************/
	const Node& getCameraNode(uint32 cameraNodeIndex) const
	{
		assertion(cameraNodeIndex < getNumCameraNodes() ,  "Invalid camera node index");
		// Camera nodes are after the mesh and light nodes in the array
		return getNode(getNodeIdFromCameraId(cameraNodeIndex));
	}

	/*!*********************************************************************************************************************
	\brief Get the (global) Node Index of a specific CameraNode.
	\param[in] cameraNodeIndex The Index of a Camera Node that will be used to calculate the NodeID. Valid values: (0 to getNumCameraNodes()-1)
	\return Retunr The Node index of the specified camera node. Normally, it is the same as getNumMeshes + getNumLights + cameraNodeIndex
	***********************************************************************************************************************/
	uint32 getNodeIdFromCameraId(uint32 cameraNodeIndex) const
	{
		// Camera nodes are after the mesh and light nodes in the array
		assertion(cameraNodeIndex < getNumCameraNodes() ,  "Invalid camera node index");
		return getNumMeshes() + getNumLights() + cameraNodeIndex;
	}

	/*!*********************************************************************************************************************
	\brief Get the number of distinct Light objects. May be different than  the actual number of Light Instances (Nodes).
	\return Return The number of distinct Light objects in this Model.
	***********************************************************************************************************************/
	uint32 getNumLights() const { return (uint32)m_data.lights.size(); }

	/*!*********************************************************************************************************************
	\brief Get the number of Light nodes.
	\return Return The number of Light instances (Nodes) in this Model.
	***********************************************************************************************************************/
	uint32 getNumLightNodes() const { return getNumLights(); /* Will be changed at a future revision */}

	/*!*********************************************************************************************************************
	\brief Get the light object with the specific Light Index.
	\param lightIndex The index of the light. Valid values (0..getNumLights()-1)
	\return Return the light
	***********************************************************************************************************************/
	const Light& getLight(uint32 lightIndex) const
	{
		assertion(lightIndex < getNumLights() ,  "Invalid light index");
		return m_data.lights[lightIndex];
	}
	/*!*********************************************************************************************************************
	\brief Get the light object with the specific Light Index.
	\param lightIndex The index of the light. Valid values (0..getNumLights()-1)
	\return Return the light
	***********************************************************************************************************************/
	Light& getLight(uint32 lightIndex)
	{
		assertion(lightIndex < getNumLights(),  "Invalid light index");
		return m_data.lights[lightIndex];
	}

	/*!*********************************************************************************************************************
	\brief Get a specific Light Node.
	\param[in] lightNodeIndex The Index of the Light Node. It is not the same as the NodeID. Valid values: (0 to getNumLightNodes()-1)
	\return Return the light node
	***********************************************************************************************************************/
	const Node& getLightNode(uint32 lightNodeIndex) const
	{
		assertion(lightNodeIndex < getNumLights(),  "Invalid light node index");
		// Light nodes are after the mesh nodes in the array
		return getNode(getNodeIdFromLightNodeId(lightNodeIndex));
	}

	/*!*********************************************************************************************************************
	\brief Get the GLOBAL index of a specific Light Node.
	\param[in] lightNodeIndex The Index of the Light Node. It is not the same as the NodeID. Valid values: (0 to getNumLightNodes()-1)
	\return Return the Node index of the same index. It is the same as getNumMeshNodes() + lightNodeIndex.
	***********************************************************************************************************************/
	uint32 getNodeIdFromLightNodeId(uint32 lightNodeIndex) const
	{
		assertion(lightNodeIndex < getNumLightNodes(),  "Invalid light node index");
		// Light nodes are after the mesh nodes in the array
		return getNumMeshNodes() + lightNodeIndex;
	}

	/*!*********************************************************************************************************************
	\brief Get the number of distinct Mesh objects. Unless each Mesh appears at exactly one Node,
	         may be different than the actual number of Mesh instances.
	\return Return The number of different Mesh objects in this Model.
	***********************************************************************************************************************/
	uint32 getNumMeshes() const { return (uint32)m_data.meshes.size(); }

	/*!*********************************************************************************************************************
	\brief Get the number of Mesh nodes.
	\return Return The number of Mesh instances (Nodes) in this Model.
	***********************************************************************************************************************/
	uint32 getNumMeshNodes() const { return m_data.numMeshNodes; }

	/*!*********************************************************************************************************************
	\brief Get the Mesh object with the specific Mesh Index. Constant overload.
	\param meshIndex The index of the Mesh. Valid values (0..getNumMeshes()-1)
	\return The mesh with id \p meshIndex. Const ref.
	***********************************************************************************************************************/
	const Mesh& getMesh(uint32 meshIndex) const { return m_data.meshes[meshIndex]; }

	/*!*********************************************************************************************************************
	\brief Get the Mesh object with the specific Mesh Index.
	\param index The index of the Mesh. Valid values (0..getNumMeshes()-1)
	\return Return the mesh from this model
	***********************************************************************************************************************/
	Mesh& getMesh(uint32 index)
	{
		assertion(index < getNumMeshes() ,  "Invalid mesh index");
		return m_data.meshes[index];
	}

	/*!*********************************************************************************************************************
	\brief Get a specific Mesh Node.
	\param[in] meshIndex The Index of the Mesh Node. For meshes, it is the same as the NodeID. Valid values: (0 to getNumMeshNodes()-1)
	\return Return he Mesh Node from this model
	***********************************************************************************************************************/
	const Node& getMeshNode(uint32 meshIndex) const
	{
		assertion(meshIndex < getNumMeshNodes() ,  "Invalid mesh index");
		// Mesh nodes are at the start of the array
		return getNode(meshIndex);
	}

	/*!*********************************************************************************************************************
	\brief Get the (global) Node Index of a specific MeshNode. This function is provided for completion, as NodeID == MeshNodeID
	\param[in] meshNodeIndex The Index of a Mesh Node that will be used to calculate the NodeID. Valid values: (0 to getNumMeshNodes()-1)
	\return Return the Node index of the specified Mesh node. This function just returns the meshNodeIndex (but is harmless and inlined).
	***********************************************************************************************************************/
	uint32 getNodeIdForMeshNodeId(uint32 meshNodeIndex) const
	{
		// Camera nodes are after the mesh and light nodes in the array
		return meshNodeIndex;
	}

	/*!*********************************************************************************************************************
	\brief Get an iterator to the beginning of the meshes.
	  \return Return an iterator
	***********************************************************************************************************************/
	std::vector<Mesh>::iterator beginMeshes() { return m_data.meshes.begin(); }

	/*!*********************************************************************************************************************
	\brief Get an iterator past the end of the meshes.
	  \return Return an iterator
	***********************************************************************************************************************/
	std::vector<Mesh>::iterator endMeshes() { return m_data.meshes.end(); }

	/*!*********************************************************************************************************************
	\brief Get a const_iterator to the beginning of the meshes.
	***********************************************************************************************************************/
	std::vector<Mesh>::const_iterator beginMeshes() const { return m_data.meshes.begin(); }

	/*!*********************************************************************************************************************
	\brief Get a const_iterator past the end of the meshes.
	***********************************************************************************************************************/
	std::vector<Mesh>::const_iterator endMeshes() const { return m_data.meshes.end(); }

	/*!*********************************************************************************************************************
	\brief Get the total number of nodes (Meshes, Cameras, Lights, others (helpers etc)).
	  \return Return number of nodes in this model
	***********************************************************************************************************************/
	uint32 getNumNodes() const { return (uint32)m_data.nodes.size(); }

	/*!*********************************************************************************************************************
	\brief Get the node with the specified index.
	\param index The index of the node to get
	\return Return The Node from this scene
	***********************************************************************************************************************/
	const Node& getNode(uint32 index) const { return m_data.nodes[index]; }

	/*!*********************************************************************************************************************
	\brief Get the node with the specified index.
	\param index The index of the node to get
	\return Return The Node from this scene
	***********************************************************************************************************************/
	Node& getNode(uint32 index) { return m_data.nodes[index]; }

	/*!*********************************************************************************************************************
	\brief Get the number of distinct Textures in the scene.
	  \return Return number of distinct textures
	***********************************************************************************************************************/
	uint32 getNumTextures() const { return (uint32)m_data.textures.size(); }

	/*!*********************************************************************************************************************
	\brief Get the texture with the specified index.
	  \param[in] index The index of the texture to get
	  \return Return a texture from this scene
	***********************************************************************************************************************/
	const Texture& getTexture(uint32 index) const { return m_data.textures[index]; }

	/*!*********************************************************************************************************************
	\brief Get the number of distinct Materials in the scene.
	  \return Return number of materials in this scene
	***********************************************************************************************************************/
	uint32 getNumMaterials() const { return (uint32)m_data.materials.size(); }

	/*!*********************************************************************************************************************
	\brief Get the material with the specified index.
	\param[in] index The index of material to get
	\return Return a material from this scene
	***********************************************************************************************************************/
	const Material& getMaterial(uint32 index) const { return m_data.materials[index]; }

	/*!*********************************************************************************************************************
	\brief Get the material with the specified index.
	\param[in] index The index of material to get
	\return Return a material from this scene
	***********************************************************************************************************************/
	Material& getMaterial(uint32 index) { return m_data.materials[index]; }

	/*!*********************************************************************************************************************
	\brief Get the total number of frames in the scene.
	  \return Return the number of frames in this model
	***********************************************************************************************************************/
	uint32 getNumFrames() const { return m_data.numFrames ? m_data.numFrames : 1; }

	/*!*********************************************************************************************************************
	\brief Set the current frame. Affects future animation calls (getWorldMatrix etc.).
	\param frame The current frame. Can be fractional, in which case interpolation will normally be performed
	\return Return true on success, false if out of bounds.
	***********************************************************************************************************************/
	bool setCurrentFrame(float32 frame);

	/*!*********************************************************************************************************************
	\brief Get the current frame of the scene.
	  \return Return the current frame
	***********************************************************************************************************************/
	float32 getCurrentFrame();

	/*!*********************************************************************************************************************
	\brief Set the expected FPS of the animation.
	  \param fps FPS of the animation
	***********************************************************************************************************************/
	void setFPS(uint32 fps) { m_data.FPS = fps; }

	/*!*********************************************************************************************************************
	\brief Get the expected FPS of the animation.
	***********************************************************************************************************************/
	uint32 getFPS()const { return m_data.FPS; }

	/*!*********************************************************************************************************************
	\brief Set custom user data.
	\param[in] size The size, in bytes, of the data.
	\param[in] data Pointer to the raw data. (size) bytes will be copied as-is from this pointer.
	***********************************************************************************************************************/
	void setUserData(uint32 size, const byte* data);

	/*!*********************************************************************************************************************
	\brief Only used for custom model creation. Allocate an number of cameras.
	  \param[in] count Number of camera to allocate in this scene
	***********************************************************************************************************************/
	void allocCameras(uint32 count);

	/*!*********************************************************************************************************************
	\brief Only used for custom model creation. Allocate a number of lights.
	  \param[in] count number of lights to allocate in this scene
	  ***********************************************************************************************************************/
	void allocLights(uint32 count);

	/*!*********************************************************************************************************************
	\brief Only used for custom model creation. Allocate a number of meshes.
	  \param[in] count number of meshes to allocate in this scene
	  ***********************************************************************************************************************/
	void allocMeshes(uint32 count);

	/*!*********************************************************************************************************************
	\brief Only used for custom model creation. Allocate a number of nodes.
	  \param[in] count number of nodes to allocate in this scene
	***********************************************************************************************************************/
	void allocNodes(uint32 count);

	/*!*********************************************************************************************************************
	\brief Get a reference to the internal data of this Model. Handle with care.
	  \return Return internal data
	***********************************************************************************************************************/
	InternalData& getInternalData() { return m_data; }

	/*!*********************************************************************************************************************
	\brief Get the properties of a camera. This is additional info on the class (remarks or documentation).
	\param[in] cameraIdx The index of the camera.
	\param[out] fov Camera field of view.
	\param[out] from Camera position in world.
	\param[out] to Camera target point in world.
	\param[out] up Camera tilt up (roll) vector in world.
	\description If cameraIdx >= number of cameras, an error will be logged and this function will have no effect.
	***********************************************************************************************************************/
	void getCameraProperties(int32 cameraIdx, float32& fov, glm::vec3& from, glm::vec3& to, glm::vec3& up) const;

	/*!*********************************************************************************************************************
	\brief Get the properties of a camera.
	\param[in] cameraIdx The index of the camera.
	\param[out] fov Camera field of view in world.
	\param[out] from Camera position in world.
	\param[out] to Camera target point in world.
	\param[out] up Camera tilt up (roll) vector in world.
	\param[out] nearClip Camera near clipping plane distance
	\param[out] farClip Camera far clipping plane distance
	\description If cameraIdx >= number of cameras, an error will be logged and this function will have no effect.
	***********************************************************************************************************************/
	void getCameraProperties(int32 cameraIdx, float32& fov, glm::vec3& from, glm::vec3& to, glm::vec3& up, float& nearClip, float& farClip) const;

	/*!*********************************************************************************************************************
	\brief Get the direction of a spot or directional light.
	\param[in] lightIdx index of the light.
	\param[out] direction The direction of the light.
	\description If lightIdx >= number of lights, an error will be logged and this function will have no effect.
	***********************************************************************************************************************/
	void getLightDirection(int32 lightIdx, glm::vec3& direction) const;

	/*!*********************************************************************************************************************
	\brief Get the position of a point or spot light.
	\param[in] lightIdx light index.
	\param[out] position The position of the light.
	\return False if \p lightIdx does not exist
	\description If lightIdx >= number of lights, an error will be logged and this function will have no effect.
	***********************************************************************************************************************/
	void getLightPosition(int32 lightIdx, glm::vec3& position) const;

	/*!*********************************************************************************************************************
	\brief Get the position of a point or spot light.
	\param[in] lightIdx light index.
	\param[out] position The position of the light.
	\description If lightIdx >= number of lights, an error will be logged and this function will have no effect.
	***********************************************************************************************************************/
	void getLightPosition(int32 lightIdx, glm::vec4& position) const;

	/*!*********************************************************************************************************************
	\brief Free the resources of this model.
	***********************************************************************************************************************/
	void destroy()
	{
		m_data = InternalData();
		initCache();
	}
private:
	InternalData m_data;

	struct Cache
	{
		float32 frameFraction;
		uint32	frame;

		std::vector<float32> cachedFrame;				// Cache indicating the frames at which the matrix cache was filled
		std::vector<glm::mat4x4> worldMatrixFrameN;			// Cache of world matrices for the frame described in fCachedFrame
		std::vector<glm::mat4x4> worldMatrixFrameZero;		// Cache of frame 0 matrices

#ifdef DEBUG
		int64 total, frameNCacheHit, frameZeroCacheHit;
		float	frameHitPerc, frameZeroHitPerc;
#endif

		Cache() : frameFraction(0), frame(0)
		{
#ifdef DEBUG
			total = frameNCacheHit = frameZeroCacheHit = 0;
			frameHitPerc = frameZeroHitPerc = 0;
#endif
		}
	};
	mutable Cache m_cache;
};

typedef Model::Material Material;
typedef Model::Node Node;
typedef Model::Mesh::VertexAttributeData VertexAttributeData;

typedef RefCountedResource<Node> NodeHandle;
typedef RefCountedResource<Material> MaterialHandle;

inline MeshHandle getMeshHandle(ModelHandle model, int meshId)
{
	MeshHandle handle;
	handle.shareRefCountFrom(model, &model->getMesh(meshId));
	return handle;
}
inline MaterialHandle getMaterialHandle(ModelHandle model, int materialId)
{
	MaterialHandle handle;
	handle.shareRefCountFrom(model, &model->getMaterial(materialId));
	return handle;
}
inline LightHandle getLightHandle(ModelHandle model, int lightId)
{
	LightHandle handle;
	handle.shareRefCountFrom(model, &model->getLight(lightId));
	return handle;
}
inline CameraHandle getCameraHandle(ModelHandle model, int cameraId)
{
	CameraHandle handle;
	handle.shareRefCountFrom(model, &model->getCamera(cameraId));
	return handle;
}
inline NodeHandle getNodeHandle(ModelHandle model, int nodeId)
{
	NodeHandle handle;
	handle.shareRefCountFrom(model, &model->getNode(nodeId));
	return handle;
}
}
}
