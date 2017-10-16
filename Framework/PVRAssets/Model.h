/*!
\brief Contains the class representing an entire Scene, or Model.
\file PVRAssets/Model.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRAssets/AssetIncludes.h"
#include "PVRAssets/Model/Camera.h"
#include "PVRAssets/Model/Animation.h"
#include "PVRAssets/Model/Light.h"
#include "PVRAssets/Model/Mesh.h"
#include "PVRAssets/FileIO/PODReader.h"

/// <summary>Main namespace of the PowerVR Framework.</summary>
namespace pvr {
class Stream;
/// <summary>Main namespace of the PVRAssets library.</summary>
namespace assets {

class Model;
class Mesh;
class Camera;
class Light;
typedef RefCountedResource<Model> ModelHandle;
typedef RefCountedResource<Mesh> MeshHandle;
typedef RefCountedResource<Camera> CameraHandle;
typedef RefCountedResource<Light> LightHandle;

/// <summary>The Model class represents an entire Scene, or Model. It is mainly a Node structure, allowing various
/// different kinds of data to be stored in the Nodes. The class contains a tree-like structure of Nodes. Each Node
/// can be a Mesh node (containing a Mesh), Camera node or Light node. The tree-structure assumes transformational
/// hierarchy (as usual). Transformations are expressed through Animation objects (a static transform is an
/// animation with a single frame) There is an implicit order in the nodes - First in the array the Mesh nodes will
/// be laid out, then Camera and Light nodes.</summary>
class Model : public Asset<Model>
{
public:

	const FreeValue* getModelSemantic(const StringHash& semantic) const
	{
		auto it = _data.semantics.find(semantic);
		if (it == _data.semantics.end())
		{
			return NULL;
		}
		return &it->second;
	}
	const RefCountedResource<void>& getUserDataPtr() const
	{
		return this->_data.userDataPtr;
	}
	RefCountedResource<void> getUserDataPtr()
	{
		return this->_data.userDataPtr;
	}
	void setUserDataPtr(const RefCountedResource<void>& ptr)
	{
		_data.userDataPtr = ptr;
	}
	typedef assets::Mesh Mesh;
	/// <summary>The Node represents a Mesh, Camera or Light. A Node has its own parenting, material, animation and
	/// custom user data. The tree-structure assumes transformational hierarchy (as usual), so parent transformations
	/// should be applied to children. Transformations are expressed through Animation objects (a static transform is
	/// an animation with a single frame). Note: Node ID and MeshID can sometimes be confusing: They will always be
	/// the same (when a MeshID makes sense) because Meshes are always laid out first in the internal list of Nodes.
	/// </summary>
	class Node
	{
	public:
		/// <summary>Raw internal structure of the Node.</summary>
		struct InternalData
		{
			StringHash name;    //!< Name of object
			int32 objectIndex;    //!< Index into mesh, light or camera array, depending on which object list contains this Node
			int32 materialIndex;  //!< Index of material used on this mesh
			int32 parentIndex;    //!< Index into Node array; recursively apply ancestor's transforms after this instance's.
			Animation animation;
			UCharBuffer userData;

			InternalData() : objectIndex(-1), materialIndex(-1), parentIndex(-1) {}
		};

	public:
		/// <summary>Get which Mesh, Camera or Light this object refers to.</summary>
		/// <returns>The index of the Mesh, Camera or Light array of this node (depending on its type)</returns>
		int32 getObjectId() const { return _data.objectIndex; }

		/// <summary>Get this Node's name.</summary>
		/// <returns>The name of this object</returns>
		const StringHash& getName() const { return _data.name; }

		/// <summary>Get this Node's parent id.</summary>
		/// <returns>The ID of this Node's parent Node.</returns>
		int32 getParentID() const { return _data.parentIndex; }

		/// <summary>Get this Node's material id.</summary>
		/// <returns>The ID of this Node's Material</returns>
		int32 getMaterialIndex() const { return _data.materialIndex; }

		void setMaterialIndex(uint32 materialId){ _data.materialIndex = materialId;  }
		
		/// <summary>Get this Node's animation.</summary>
		/// <returns>The animation of this Node</returns>
		const Animation& getAnimation() const { return _data.animation; }

		/// <summary>Get this Node's user data.</summary>
		/// <returns>The user data of this Node</returns>
		const byte* getUserData() const { return _data.userData.data(); }

		/// <summary>Get the size of this Node's user data.</summary>
		/// <returns>Return The size in bytes of the user data of this Node</returns>
		uint32 getUserDataSize() const { return (uint32)_data.userData.size(); }

		/// <summary>Set mesh id. Must correlate with the actual position of this node in the data.</summary>
		/// <param name="index">The id to set the index of this node.</param>
		void setIndex(int32 index) { _data.objectIndex = index; }

		/// <summary>Set the name of this node.</summary>
		/// <param name="name">The string to set this node's name to.</param>
		void setName(const StringHash& name) { _data.name = name; }

		/// <summary>Set the parent of this node.</summary>
		/// <param name="parentID">the ID of this node's parent</param>
		void setParentID(int32 parentID) { _data.parentIndex = parentID; }

		/// <summary>Set the animation of this node.</summary>
		/// <param name="animation">The animation of this node. A copy of the animation object will be created and stored
		/// directly.</param>
		void setAnimation(const Animation& animation) { _data.animation = animation; }

		/// <summary>Set the user data of this node. A bit copy of the data will be made.</summary>
		/// <param name="size">The size, in bytes, of the data</param>
		/// <param name="data">A pointer from which (size) data will be copied.</param>
		void setUserData(uint32 size, const byte* data)
		{
			_data.userData.resize(size);
			memcpy(_data.userData.data(), data, size);
		}

		/// <summary>Get a reference to the internal data of this object. Handle with care.</summary>
		/// <returns>Return a reference to the internal data of this object.</returns>
		InternalData& getInternalData() { return _data; }

		/// <summary>Get a const reference to the data object carrying all internal data of this model.</summary>
		const InternalData& getInternalData() const { return _data; }

	private:
		InternalData _data;
	};

	/// <summary>Internal class which stores Texture information of the model (name).</summary>
	class Texture
	{
	public:
		/// <summary>Get the name of the texture.</summary>
		/// <returns>Return the texture name</returns>
		const pvr::StringHash& getName() const { return name; }

		/// <summary>Get a reference to the name of the texture.</summary>
		/// <returns>Return reference to the texture name</returns>
		pvr::StringHash& getName() { return name; }

		/// <summary>Set the name of the texture.</summary>
		/// <param name="name">The string to set this texture name to.</param>
		void setName(const StringHash& name) { this->name = name; }

	private:
		pvr::StringHash name;
	};

	/// <summary>Class which stores model material info.</summary>
	class Material
	{
	public:
		Material() : _defaultSemantics(*this) {}

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
			BlendOpAdd = 0x8006,      //!< Blend Operation (Add)
			BlendOpMin,                     //!< Blend Operation (Min)
			BlendOpMax,                     //!< Blend Operation (Max)
			BlendOpSubtract = 0x800A,       //!< Blend Operation (Subtract)
			BlendOpReverseSubtract          //!< Blend Operation (Reverse Subtract)
		};

		//"BLENDFUNCTION", BlendFunction
		//"BLENDOP", BlendOperation
		//"DIFFUSETEXTURE",
		//"SPECULARTEXTURE",
		//"BUMPMAPTEXTURE",
		//"EMISSIVETEXTURE",
		//"GLOSSINESSTEXTURE",
		//"OPACITYTEXTURE",
		//"REFLECTIONTEXTURE",
		//"REFRACTIONTEXTURE",
		//"OPACITY",
		//"AMBIENTCOLOR",
		//"DIFFUSECOLOR",
		//"SPECULARCOLOR",
		//"SHININESS",
		//"EFFECTFILE",
		//"EFFECTNAME",
		//"BLENDFUNCSRCCOLOR"
		//"BLENDFUNCSRCALPHA"
		//"BLENDFUNCDSTCOLOR"
		//"BLENDFUNCDSTALPHA"
		//"BLENDCOLOR"
		//"BLENDFACTOR"
		//"FLAGS"
		//"USERDATA"

		/// <summary>Raw internal structure of the Material.</summary>
		struct InternalData
		{
			std::map<StringHash, FreeValue> materialSemantics;
			std::map<StringHash, uint32> textureIndexes;

			StringHash name;          //!<Name of the material
			StringHash effectFile;        //!<Effect filename if using an effect
			StringHash effectName;        //!<Effect name (in the filename) if using an effect

			UCharBuffer userData;       //!<Raw user data

			/// <summary>Default constructor.</summary>
		};

		class DefaultMaterialSemantics
		{
		public:
			DefaultMaterialSemantics(const Material& material): material(&material) {}

			/// <summary>Get material ambient.</summary>
			/// <returns>Material ambient</returns>
			glm::vec3 getAmbient() const
			{
				return material->getMaterialAttributeWithDefault<glm::vec3>("AMBIENT", glm::vec3(0.f, 0.f, 0.f));
			}

			/// <summary>Get material diffuse.</summary>
			/// <returns>Material diffuse</returns>
			glm::vec3 getDiffuse() const
			{
				return material->getMaterialAttributeWithDefault<glm::vec3>("DIFFUSE", glm::vec3(1.f, 1.f, 1.f));
			}

			/// <summary>Get material specular.</summary>
			/// <returns>Material specular</returns>
			glm::vec3 getSpecular() const
			{
				return material->getMaterialAttributeWithDefault<glm::vec3>("SPECULAR", glm::vec3(0.f, 0.f, 0.f));
			}

			/// <summary>Get material shininess.</summary>
			/// <returns>Material shininess</returns>
			float32 getShininess() const
			{
				return material->getMaterialAttributeWithDefault<float32>("SHININESS", 0.f);
			}

			/// <summary>Get the diffuse color texture's index in the scene.</summary>
			/// <returns>Return the diffuse texture index</returns>
			int32 getDiffuseTextureIndex() const
			{
				static const StringHash diffuseTexSemantic("DIFFUSEMAP");
				return material->getTextureIndex(diffuseTexSemantic);
			}


			/// <summary>Return the ambient color texture's index in the scene.</summary>
			int32 getAmbientTextureIndex() const
			{
				static const StringHash diffuseTexSemantic("AMBIENTMAP");
				return material->getTextureIndex(diffuseTexSemantic);
			}

			/// <summary>Get the specular color texture's index in the scene.</summary>
			/// <returns>Return the specular color texture index</returns>
			int32 getSpecularColorTextureIndex() const
			{
				static const StringHash diffuseTexSemantic("SPECULARCOLORMAP");
				return material->getTextureIndex(diffuseTexSemantic);
			}


			/// <summary>Get the specular level texture's index in the scene.</summary>
			/// <returns>Return the specular level texture index</returns>
			int32 getSpecularLevelTextureIndex() const
			{
				static const StringHash diffuseTexSemantic("SPECULARLEVELMAP");
				return material->getTextureIndex(diffuseTexSemantic);
			}


			/// <summary>Get bumpmap texture index.</summary>
			/// <returns>Return the bumpmap texture index</returns>
			int32 getBumpMapTextureIndex() const
			{
				static const StringHash diffuseTexSemantic("NORMALMAP");
				return material->getTextureIndex(diffuseTexSemantic);
			}

			/// <summary>Get emissive texture's index in the scene</summary>
			/// <returns>Return the emissive texture index</returns>
			int32 getEmissiveTextureIndex() const
			{
				static const StringHash diffuseTexSemantic("EMISSIVEMAP");
				return material->getTextureIndex(diffuseTexSemantic);
			}

			/// <summary>Get glossiness texture's index in the scene.</summary>
			/// <returns>Return the glossiness texture index</returns>
			int32 getGlossinessTextureIndex() const
			{
				static const StringHash diffuseTexSemantic("GLOSSINESSMAP");
				return material->getTextureIndex(diffuseTexSemantic);
			}


			/// <summary>Get opacity texture's index in the scene.</summary>
			/// <returns>Return the opacity texture index</returns>
			int32 getOpacityTextureIndex() const
			{
				static const StringHash diffuseTexSemantic("OPACITYMAP");
				return material->getTextureIndex(diffuseTexSemantic);
			}


			/// <summary>Get reflection texture's index in the scene.</summary>
			/// <returns>Return the reflection texture index</returns>
			int32 getReflectionTextureIndex() const
			{
				static const StringHash diffuseTexSemantic("REFLECTIONMAP");
				return material->getTextureIndex(diffuseTexSemantic);
			}

			/// <summary>Return refraction texture's index in the scene.</summary>
			/// <returns>Return the refraction texture index</returns>
			int32 getRefractionTextureIndex() const
			{
				static const StringHash diffuseTexSemantic("REFRACTIONMAP");
				return material->getTextureIndex(diffuseTexSemantic);
			}

			/// <summary>Get this material opacity.</summary>
			/// <returns>Return the material opacity</returns>
			float32 getOpacity() const
			{
				return material->getMaterialAttributeWithDefault<float32>("OPACITY", 1.f);
			}


			/// <summary>Get the blend function for Source Color.</summary>
			/// <returns>Return source color blend function</returns>
			BlendFunction getBlendSrcRGB() const
			{
				return material->getMaterialAttributeWithDefault<BlendFunction>("BLENDSRCCOLOR", BlendFunction::BlendFuncOne);
			}

			/// <summary>Get the blend function for Source Alpha.</summary>
			/// <returns>Return source alpha blend function</returns>
			BlendFunction getBlendSrcA() const
			{
				return material->getMaterialAttributeWithDefault<BlendFunction>("BLENDSRCALPHA", BlendFunction::BlendFuncOne);
			}

			/// <summary>Get the blend function for Destination Color.</summary>
			/// <returns>Return destination color blend function</returns>
			BlendFunction getBlendDstRGB() const
			{
				return material->getMaterialAttributeWithDefault<BlendFunction>("BLENDDSTCOLOR", BlendFunction::BlendFuncZero);
			}

			/// <summary>Get the blend function for Destination Alpha.</summary>
			/// <returns>Return destination alpha blend function</returns>
			BlendFunction getBlendDstA() const
			{
				return material->getMaterialAttributeWithDefault<BlendFunction>("BLENDDSTALPHA", BlendFunction::BlendFuncZero);
			}

			/// <summary>Get the blend operation for Color.</summary>
			/// <returns>Return the color's blend operator</returns>
			BlendOperation getBlendOpRGB() const
			{
				return material->getMaterialAttributeWithDefault<BlendOperation>("BLENDOPCOLOR", BlendOperation::BlendOpAdd);
			}

			/// <summary>Return the blend operation for Alpha.</summary>
			/// <returns>Return the alpha's blend operator</returns>
			BlendOperation getBlendOpA() const
			{
				return material->getMaterialAttributeWithDefault<BlendOperation>("BLENDOPALPHA", BlendOperation::BlendOpAdd);
			}

			/// <summary>Get the blend color.</summary>
			/// <returns>Return blend color</returns>
			glm::vec4 getBlendColor() const
			{
				return material->getMaterialAttributeWithDefault<glm::vec4>("BLENDCOLOR", glm::vec4(0.f, 0.f, 0.f, 0.f));
			}

			/// <summary>Return the blend factor.</summary>
			/// <returns>Return blend factor</returns>
			glm::vec4 getBlendFactor() const
			{
				return material->getMaterialAttributeWithDefault<glm::vec4>("BLENDFACTOR", glm::vec4(0.f, 0.f, 0.f, 0.f));
			}

		private:
			const Material* material;
		};

	public:
		const DefaultMaterialSemantics& defaultSemantics() const
		{
			return _defaultSemantics;
		}
		
		void setMaterialAttribute(const StringHash& attrib, const FreeValue& val)
		{
			_data.materialSemantics[attrib] = val;
		}

		const FreeValue* getMaterialAttribute(const StringHash& attrib) const
		{
			auto it = _data.materialSemantics.find(attrib);
			if (it != _data.materialSemantics.end())
			{
				return &it->second;
			}
			return NULL;
		}

		template<typename Type>
		const Type getMaterialAttributeWithDefault(const StringHash& attrib, const Type& defaultAttrib) const
		{
			auto* val = getMaterialAttribute(attrib);
			if (val)
			{
				return val->interpretValueAs<Type>();
			}
			return defaultAttrib;
		}

		template<typename Type>
		const Type* getMaterialAttributeAs(const StringHash& attrib) const
		{
			auto* val = getMaterialAttribute(attrib);
			if (val)
			{
				return &val->interpretValueAs<Type>();
			}
			return NULL;
		}

		bool hasSemantic(const StringHash& semantic) const
		{
			return hasMaterialTexture(semantic) || hasMaterialAttribute(semantic);
		}

		/// <summary>Get material attribute.</summary>
		/// <returns>Material attribute</returns>
		bool hasMaterialTexture(const StringHash& semantic) const
		{
			return getTextureIndex(semantic) != -1;
		}
		/// <summary>Get material attribute.</summary>
		/// <returns>Material attribute</returns>
		bool hasMaterialAttribute(const StringHash& semantic) const
		{
			return getMaterialAttribute(semantic) != NULL;
		}


		/// <summary>Set material effect name.</summary>
		/// <param name="name">Material effect name</param>
		void setEffectName(const StringHash& name) { _data.effectName = name; }

		/// <summary>Set material effect file name.</summary>
		/// <param name="name">Effect file name</param>
		void setEffectFile(const StringHash& name) { _data.effectFile = name; }


		/// <summary>Get the diffuse color texture's index in the scene.</summary>
		/// <returns>Return the diffuse texture index</returns>
		int32 getTextureIndex(const StringHash& semantic) const
		{
			auto it = _data.textureIndexes.find(semantic);
			return (it == _data.textureIndexes.end()) ? -1 : (int32)it->second;
		}


		/// <summary>Get this material name.</summary>
		/// <returns>return the material name</returns>
		const StringHash& getName() const
		{
			return this->_data.name;
		}

		/// <summary>Get this material effect file name.</summary>
		/// <returns>Retuurn Material effect file name</returns>
		const StringHash& getEffectFile() const { return _data.effectFile; }

		/// <summary>Get this material effect name.</summary>
		/// <returns>Return material effect name</returns>
		const StringHash& getEffectName() const { return _data.effectName; }

		/// <summary>Return a reference to the material's internal data structure. Handle with care.</summary>
		/// <returns>Return reference to the internal data</returns>
		InternalData& getInternalData() { return _data; }

	private:
		//uint32  flags;
		UCharBuffer userData;
		InternalData _data;
		DefaultMaterialSemantics _defaultSemantics;
	};

public:

	/// <summary>Struct containing the internal data of the Model.</summary>
	struct InternalData
	{
		pvr::ContiguousMap<StringHash, FreeValue> semantics; //

		float32 clearColor[3];      //!< Background color
		float32 ambientColor[3];    //!< Ambient color

		std::vector<Mesh> meshes;   //!< Mesh array. Any given mesh can be referenced by multiple Nodes.
		std::vector<Camera> cameras;  //!< Camera array. Any given mesh can be referenced by multiple Nodes.
		std::vector<Light> lights;    //!< Light array. Any given mesh can be referenced by multiple Nodes.
		std::vector<Texture> textures;  //!< Textures in this Model
		std::vector<Material> materials;//!< Materials in this Model
		std::vector<Node> nodes;    //!< Nodes array. The nodes are sorted thus: First Mesh Nodes, then Light Nodes, then Camera nodes.

		uint32 numMeshNodes;      //!< Number of items in the nodes array which are Meshes
		uint32 numLightNodes;     //!< Number of items in the nodes array which are Meshes
		uint32 numCameraNodes;      //!< Number of items in the nodes array which are Meshes

		uint32 numFrames;       //!< Number of frames of animation
		float32 currentFrame;     //!< Current frame in the animation
		uint32 FPS;           //!< The frames per second the animation should be played at

		UCharBuffer userData;     //!< Custom raw data stored by the user

		float32 units;          //!< Unit scaling
		uint32 flags;         //!< Flags
		RefCountedResource<void> userDataPtr; //!< Can be used to store any kind of data that the user wraps in a refcounted resource

		/// <summary>ctor.</summary>
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

	/// <summary>Return the world-space position of a light. Corresponds to the Model's current frame of animation.
	/// </summary>
	/// <param name="lightId">The node for which to return the world matrix.</param>
	/// <returns>Return The world matrix of (nodeId).</returns>
	glm::vec3 getLightPosition(uint32 lightId) const;

	/// <summary>Return the model-to-world matrix of a node. Corresponds to the Model's current frame of animation. This
	/// version will store a copy of the matrix in an internal cache so that repeated calls for it will use the cached
	/// copy of it. Will also store the cached versions of all parents of this node, or use cached versions of them if
	/// they exist. Use this if you have long hierarchies and/or repeated calls per frame.</summary>
	/// <param name="nodeId">The node for which to return the world matrix.</param>
	/// <returns>Return The world matrix of (nodeId).</returns>
	glm::mat4x4 getWorldMatrix(uint32 nodeId) const;

	/// <summary>Return the model-to-world matrix of a node. Corresponds to the Model's current frame of animation. This
	/// version will not use caching and will recalculate the matrix. Faster if the matrix is only used a few times.
	/// </summary>
	/// <param name="nodeId">The node for which to return the world matrix</param>
	/// <returns>return The world matrix of (nodeId)</returns>
	glm::mat4x4 getWorldMatrixNoCache(uint32 nodeId) const;

	/// <summary>Return the model-to-world matrix of a specified bone. Corresponds to the Model's current frame of
	/// animation. This version will use caching.</summary>
	/// <param name="skinNodeID">The node for which to return the world matrix</param>
	/// <param name="boneId">The bone for which to return the world matrix</param>
	/// <returns>Return The world matrix of (nodeId, boneID)</returns>
	glm::mat4x4 getBoneWorldMatrix(uint32 skinNodeID, uint32 boneId) const;

	/// <summary>Transform a custom matrix with a node's parent's transformation. Allows a custom matrix to be applied to a
	/// node, while honoring the hierarchical transformations applied by its parent hierarchy.</summary>
	/// <param name="nodeId">The node whose parents will be applied to the transformation.</param>
	/// <param name="localMatrix">The matrix to transform</param>
	/// <returns>Return The localMatrix transformation, modified by the hierarchical transformations of the node.
	/// </returns>
	/// <remarks>This function can be used to implement custom procedural animation/kinematics schemes, in which case
	/// some nodes may need to have their animations customly defined, but must still honor their parents'
	/// transformations.</remarks>
	glm::mat4x4 toWorldMatrix(uint32 nodeId, const glm::mat4& localMatrix) const
	{
		int32 parentID = _data.nodes[nodeId].getParentID();
		if (parentID < 0)
		{
			return localMatrix;
		}
		else
		{
			internal::optimizedMat4 parentWorld = internal::optimizedMat4(getWorldMatrix(parentID));
			return parentWorld * localMatrix;
		}
	}

	/// <summary>Return the model-to-world matrix of a node, **relative to its parent node**. In order to get the actual
	/// model-to-world matrix of the node, call getWorldMatrix (which will multiply the local matrix by the parent't
	/// matrix).</summary>
	/// <param name="nodeId">The node for which to return the local matrix.</param>
	/// <returns>Return The locatmatrix of (nodeId).</returns>
	/// <remarks>You can use this to get the transformation of a node relative to its parent hierarchies. May be
	/// useful for implementing custom (e.g. procedural) animation/kinematics.</remarks>
	glm::mat4x4 getLocalMatrix(uint32 nodeId) const
	{
		const Node& node = _data.nodes[nodeId];
		return internal::optimizedMat4(node.getAnimation().getTransformationMatrix(_cache.frame, _cache.frameFraction));
	}



	/// <summary>Initialize the cache. Call this after changing the model data. It is automatically called by PODReader when
	/// reading a POD file.</summary>
	void initCache();

	/// <summary>Release the memory of the cache.</summary>
	void destroyCache();

	/// <summary>Modify a node's transformation then flush the cache. No effect if cache is uninitialized</summary>
	void flushCache();

	/// <summary>Get the clear color (background) (float array R,G,B,A).</summary>
	const float32* getBackgroundColor() const { return _data.clearColor; }

	/// <summary>Get the number of distinct camera objects. May be different than the actual number of Camera
	/// Instances (Nodes).</summary>
	/// <returns>Return The number of distinct camera objects.</returns>
	uint32 getNumCameras() const { return (uint32)_data.cameras.size(); }

	/// <summary>Get the number of Camera nodes in this model</summary>
	/// <returns>Return The number of Camera instances (Nodes) in this Model.</returns>
	uint32 getNumCameraNodes() const { return getNumCameras(); /* Will be changed at a future revision */ }

	/// <summary>Get a Camera from this model</summary>
	/// <param name="cameraIndex">The index of the camera. Valid values (0 to getNumCameras()-1)</param>
	/// <returns>Return the camera</returns>
	const Camera& getCamera(uint32 cameraIndex) const
	{
		assertion(cameraIndex < getNumCameras(), "Invalid camera index");
		return _data.cameras[cameraIndex];
	}
	/// <summary>Get a Camera from this model</summary>
	/// <param name="cameraIndex">The index of the camera. Valid values (0 to getNumCameras()-1)</param>
	/// <returns>Return the camera</returns>
	Camera& getCamera(uint32 cameraIndex)
	{
		assertion(cameraIndex < getNumCameras(), "Invalid camera index");
		return _data.cameras[cameraIndex];
	}

	/// <summary>Get a specific CameraNode.</summary>
	/// <param name="cameraNodeIndex">The Index of a Camera Node. It is not the same as the NodeID. Valid values: (0
	/// .. getNumCameraNodes()-1)</param>
	/// <returns>Return The Camera Node</returns>
	const Node& getCameraNode(uint32 cameraNodeIndex) const
	{
		assertion(cameraNodeIndex < getNumCameraNodes(), "Invalid camera node index");
		// Camera nodes are after the mesh and light nodes in the array
		return getNode(getNodeIdFromCameraId(cameraNodeIndex));
	}

	/// <summary>Get the (global) Node Index of a specific CameraNode.</summary>
	/// <param name="cameraNodeIndex">The Index of a Camera Node that will be used to calculate the NodeID. Valid
	/// values: (0 to getNumCameraNodes()-1)</param>
	/// <returns>Retunr The Node index of the specified camera node. Normally, it is the same as getNumMeshes +
	/// getNumLights + cameraNodeIndex</returns>
	uint32 getNodeIdFromCameraId(uint32 cameraNodeIndex) const
	{
		// Camera nodes are after the mesh and light nodes in the array
		assertion(cameraNodeIndex < getNumCameraNodes(), "Invalid camera node index");
		return getNumMeshes() + getNumLights() + cameraNodeIndex;
	}

	/// <summary>Get the number of distinct Light objects. May be different than the actual number of Light Instances
	/// (Nodes).</summary>
	/// <returns>Return The number of distinct Light objects in this Model.</returns>
	uint32 getNumLights() const { return (uint32)_data.lights.size(); }

	/// <summary>Get the number of Light nodes.</summary>
	/// <returns>Return The number of Light instances (Nodes) in this Model.</returns>
	uint32 getNumLightNodes() const { return getNumLights(); /* Will be changed at a future revision */ }

	/// <summary>Get the light object with the specific Light Index.</summary>
	/// <param name="lightIndex">The index of the light. Valid values (0..getNumLights()-1)</param>
	/// <returns>Return the light</returns>
	const Light& getLight(uint32 lightIndex) const
	{
		assertion(lightIndex < getNumLights(), "Invalid light index");
		return _data.lights[lightIndex];
	}
	/// <summary>Get the light object with the specific Light Index.</summary>
	/// <param name="lightIndex">The index of the light. Valid values (0..getNumLights()-1)</param>
	/// <returns>Return the light</returns>
	Light& getLight(uint32 lightIndex)
	{
		assertion(lightIndex < getNumLights(), "Invalid light index");
		return _data.lights[lightIndex];
	}

	/// <summary>Get a specific Light Node.</summary>
	/// <param name="lightNodeIndex">The Index of the Light Node. It is not the same as the NodeID. Valid values: (0
	/// to getNumLightNodes()-1)</param>
	/// <returns>Return the light node</returns>
	const Node& getLightNode(uint32 lightNodeIndex) const
	{
		assertion(lightNodeIndex < getNumLights(), "Invalid light node index");
		// Light nodes are after the mesh nodes in the array
		return getNode(getNodeIdFromLightNodeId(lightNodeIndex));
	}

	/// <summary>Get the GLOBAL index of a specific Light Node.</summary>
	/// <param name="lightNodeIndex">The Index of the Light Node. It is not the same as the NodeID. Valid values: (0
	/// to getNumLightNodes()-1)</param>
	/// <returns>Return the Node index of the same index. It is the same as getNumMeshNodes() + lightNodeIndex.
	/// </returns>
	uint32 getNodeIdFromLightNodeId(uint32 lightNodeIndex) const
	{
		assertion(lightNodeIndex < getNumLightNodes(), "Invalid light node index");
		// Light nodes are after the mesh nodes in the array
		return getNumMeshNodes() + lightNodeIndex;
	}

	/// <summary>Get the number of distinct Mesh objects. Unless each Mesh appears at exactly one Node, may be
	/// different than the actual number of Mesh instances.</summary>
	/// <returns>Return The number of different Mesh objects in this Model.</returns>
	uint32 getNumMeshes() const { return (uint32)_data.meshes.size(); }

	/// <summary>Get the number of Mesh nodes.</summary>
	/// <returns>Return The number of Mesh instances (Nodes) in this Model.</returns>
	uint32 getNumMeshNodes() const { return _data.numMeshNodes; }

	/// <summary>Get the Mesh object with the specific Mesh Index. Constant overload.</summary>
	/// <param name="meshIndex">The index of the Mesh. Valid values (0..getNumMeshes()-1)</param>
	/// <returns>The mesh with id <paramref name="meshIndex."/>Const ref.</returns>
	const Mesh& getMesh(uint32 meshIndex) const { return _data.meshes[meshIndex]; }

	/// <summary>Get the Mesh object with the specific Mesh Index.</summary>
	/// <param name="index">The index of the Mesh. Valid values (0..getNumMeshes()-1)</param>
	/// <returns>Return the mesh from this model</returns>
	Mesh& getMesh(uint32 index)
	{
		assertion(index < getNumMeshes(), "Invalid mesh index");
		return _data.meshes[index];
	}

	/// <summary>Get a specific Mesh Node.</summary>
	/// <param name="meshIndex">The Index of the Mesh Node. For meshes, it is the same as the NodeID. Valid values: (0
	/// to getNumMeshNodes()-1)</param>
	/// <returns>Return he Mesh Node from this model</returns>
	const Node& getMeshNode(uint32 meshIndex) const
	{
		assertion(meshIndex < getNumMeshNodes(), "Invalid mesh index");
		// Mesh nodes are at the start of the array
		return getNode(meshIndex);
	}

	Node& getMeshNode(uint32 meshIndex)
	{
		assertion(meshIndex < getNumMeshNodes(), "Invalid mesh index");
		// Mesh nodes are at the start of the array
		return getNode(meshIndex);
	}

	void connectMeshWithMeshNode(uint32 mesh, uint32 meshNode)
	{
		getMeshNode(meshNode).setIndex(mesh);
	}

	/// <summary>Connect mesh to number of mesh nodes</summary>
	/// <param name="meshId">The mesh id</param>
	/// <param name="beginMeshNodeId">Begin mesh node id (inclusive)</param>
	/// <param name="endMeshNodeId">End mesh node id (inclusive)</param>
	void connectMeshWithMeshNodes(uint32 meshId,uint32 beginMeshNodeId, uint32 endMeshNodeId)
	{
		for(uint32  i = beginMeshNodeId; i <= endMeshNodeId; ++i)
		{
			connectMeshWithMeshNode(meshId, i);
		}
	}

	/// <summary>Assign material id to number of mesh nodes</summary>
	/// <param name="materialIndex">Material id</param>
	/// <param name="beginMeshNodeId">Begin mesh node id (inclusive)</param>
	/// <param name="endMeshNodeId">end mesh node id (inclusive)</param>
	void assignMaterialToMeshNodes(uint32 materialIndex,uint32 beginMeshNodeId, uint32 endMeshNodeId)
	{
		for(uint32  i = beginMeshNodeId; i <= endMeshNodeId; ++i)
		{
			getMeshNode(i).setMaterialIndex(materialIndex);
		}
	}
	
	/// <summary>Get the (global) Node Index of a specific MeshNode. This function is provided for completion, as
	/// NodeID == MeshNodeID</summary>
	/// <param name="meshNodeIndex">The Index of a Mesh Node that will be used to calculate the NodeID. Valid values:
	/// (0 to getNumMeshNodes()-1)</param>
	/// <returns>Return the Node index of the specified Mesh node. This function just returns the meshNodeIndex (but is
	/// harmless and inlined).</returns>
	uint32 getNodeIdForMeshNodeId(uint32 meshNodeIndex) const
	{
		debug_assertion(meshNodeIndex < getNumMeshNodes(), "invalid mesh node index");
		// Camera nodes are after the mesh and light nodes in the array
		return meshNodeIndex;
	}

	/// <summary>Get an iterator to the beginning of the meshes.</summary>
	/// <returns>Return an iterator</returns>
	std::vector<Mesh>::iterator beginMeshes() { return _data.meshes.begin(); }

	/// <summary>Get an iterator past the end of the meshes.</summary>
	/// <returns>Return an iterator</returns>
	std::vector<Mesh>::iterator endMeshes() { return _data.meshes.end(); }

	/// <summary>Get a const_iterator to the beginning of the meshes.</summary>
	std::vector<Mesh>::const_iterator beginMeshes() const { return _data.meshes.begin(); }

	/// <summary>Get a const_iterator past the end of the meshes.</summary>
	std::vector<Mesh>::const_iterator endMeshes() const { return _data.meshes.end(); }

	/// <summary>Get the total number of nodes (Meshes, Cameras, Lights, others (helpers etc)).</summary>
	/// <returns>Return number of nodes in this model</returns>
	uint32 getNumNodes() const { return (uint32)_data.nodes.size(); }

	/// <summary>Get the node with the specified index.</summary>
	/// <param name="index">The index of the node to get</param>
	/// <returns>Return The Node from this scene</returns>
	const Node& getNode(uint32 index) const { return _data.nodes[index]; }

	/// <summary>Get the node with the specified index.</summary>
	/// <param name="index">The index of the node to get</param>
	/// <returns>Return The Node from this scene</returns>
	Node& getNode(uint32 index) { return _data.nodes[index]; }

	/// <summary>Get the number of distinct Textures in the scene.</summary>
	/// <returns>Return number of distinct textures</returns>
	uint32 getNumTextures() const { return (uint32)_data.textures.size(); }

	/// <summary>Get the texture with the specified index.</summary>
	/// <param name="index">The index of the texture to get</param>
	/// <returns>Return a texture from this scene</returns>
	const Texture& getTexture(uint32 index) const { return _data.textures[index]; }

	/// <summary>Get the number of distinct Materials in the scene.</summary>
	/// <returns>Return number of materials in this scene</returns>
	uint32 getNumMaterials() const { return (uint32)_data.materials.size(); }

	/// <summary>Get the material with the specified index.</summary>
	/// <param name="index">The index of material to get</param>
	/// <returns>Return a material from this scene</returns>
	const Material& getMaterial(uint32 index) const { return _data.materials[index]; }

	uint32 addMaterial(const Material& material)
	{ 
		_data.materials.push_back(material); 
		return (uint32)(_data.materials.size() - 1);
	}
	
	/// <summary>Get the material with the specified index.</summary>
	/// <param name="index">The index of material to get</param>
	/// <returns>Return a material from this scene</returns>
	Material& getMaterial(uint32 index) { return _data.materials[index]; }

	/// <summary>Get the total number of frames in the scene. The total number of usable animated frames is limited to
	/// exclude (numFrames - 1) but include any partial number up to (numFrames - 1). Example: If there are 100 frames
	/// of animation, the highest frame number allowed is 98, since that will blend between frames 98 and 99. (99
	/// being of course the 100th frame.)</summary>
	/// <returns>Return the number of frames in this model</returns>
	uint32 getNumFrames() const { return _data.numFrames ? _data.numFrames : 1; }

	/// <summary>Set the current frame. Affects future animation calls (getWorldMatrix etc.).</summary>
	/// <param name="frame">The current frame. Can be fractional, in which case interpolation will normally be
	/// performed</param>
	/// <returns>Return true on success, false if out of bounds.</returns>
	bool setCurrentFrame(float32 frame);

	/// <summary>Get the current frame of the scene.</summary>
	/// <returns>Return the current frame</returns>
	float32 getCurrentFrame();

	/// <summary>Set the expected FPS of the animation.</summary>
	/// <param name="fps">FPS of the animation</param>
	void setFPS(uint32 fps) { _data.FPS = fps; }

	/// <summary>Get the expected FPS of the animation.</summary>
	uint32 getFPS()const { return _data.FPS; }

	/// <summary>Set custom user data.</summary>
	/// <param name="size">The size, in bytes, of the data.</param>
	/// <param name="data">Pointer to the raw data. (size) bytes will be copied as-is from this pointer.</param>
	void setUserData(uint32 size, const byte* data);

	/// <summary>Only used for custom model creation. Allocate an number of cameras.</summary>
	/// <param name="count">Number of camera to allocate in this scene</param>
	void allocCameras(uint32 count);

	/// <summary>Only used for custom model creation. Allocate a number of lights.</summary>
	/// <param name="count">number of lights to allocate in this scene</param>
	void allocLights(uint32 count);

	/// <summary>Only used for custom model creation. Allocate a number of meshes.</summary>
	/// <param name="count">number of meshes to allocate in this scene</param>
	void allocMeshes(uint32 count);

	/// <summary>Only used for custom model creation. Allocate a number of nodes.</summary>
	/// <param name="count">number of nodes to allocate in this scene</param>
	void allocNodes(uint32 count);

	/// <summary>Get a reference to the internal data of this Model. Handle with care.</summary>
	/// <returns>Return internal data</returns>
	InternalData& getInternalData() { return _data; }

	/// <summary>Get the properties of a camera. This is additional info on the class (remarks or documentation).
	/// </summary>
	/// <param name="cameraIdx">The index of the camera.</param>
	/// <param name="fov">Camera field of view.</param>
	/// <param name="from">Camera position in world.</param>
	/// <param name="to">Camera target point in world.</param>
	/// <param name="up">Camera tilt up (roll) vector in world.</param>
	/// <remarks>If cameraIdx >= number of cameras, an error will be logged and this function will have no effect.
	/// </remarks>
	void getCameraProperties(int32 cameraIdx, float32& fov, glm::vec3& from, glm::vec3& to, glm::vec3& up) const;

	/// <summary>Get the properties of a camera.</summary>
	/// <param name="cameraIdx">The index of the camera.</param>
	/// <param name="fov">Camera field of view in world.</param>
	/// <param name="from">Camera position in world.</param>
	/// <param name="to">Camera target point in world.</param>
	/// <param name="up">Camera tilt up (roll) vector in world.</param>
	/// <param name="nearClip">Camera near clipping plane distance</param>
	/// <param name="farClip">Camera far clipping plane distance</param>
	/// <remarks>If cameraIdx >= number of cameras, an error will be logged and this function will have no effect.
	/// </remarks>
	void getCameraProperties(int32 cameraIdx, float32& fov, glm::vec3& from, glm::vec3& to, glm::vec3& up, float& nearClip, float& farClip) const;

	/// <summary>Get the direction of a spot or directional light.</summary>
	/// <param name="lightIdx">index of the light.</param>
	/// <param name="direction">The direction of the light.</param>
	/// <remarks>If lightIdx >= number of lights, an error will be logged and this function will have no effect.
	/// </remarks>
	void getLightDirection(int32 lightIdx, glm::vec3& direction) const;

	/// <summary>Get the position of a point or spot light.</summary>
	/// <param name="lightIdx">light index.</param>
	/// <param name="position">The position of the light.</param>
	/// <returns>False if <paramref name="lightIdx"/>does not exist</returns>
	/// <remarks>If lightIdx >= number of lights, an error will be logged and this function will have no effect.
	/// </remarks>
	void getLightPosition(int32 lightIdx, glm::vec3& position) const;

	/// <summary>Get the position of a point or spot light.</summary>
	/// <param name="lightIdx">light index.</param>
	/// <param name="position">The position of the light.</param>
	/// <remarks>If lightIdx >= number of lights, an error will be logged and this function will have no effect.
	/// </remarks>
	void getLightPosition(int32 lightIdx, glm::vec4& position) const;

	/// <summary>Free the resources of this model.</summary>
	void destroy()
	{
		_data = InternalData();
		initCache();
	}
	void allocMeshNodes(uint32 no);
private:
	InternalData _data;

	struct Cache
	{
		float32 frameFraction;
		uint32  frame;

#ifdef DEBUG
		int64 total, frameNCacheHit, frameZeroCacheHit;
		float frameHitPerc, frameZeroHitPerc;
#endif
		std::vector<float32> cachedFrame;       // Cache indicating the frames at which the matrix cache was filled
		std::vector<glm::mat4x4> worldMatrixFrameN;     // Cache of world matrices for the frame described in fCachedFrame
		std::vector<glm::mat4x4> worldMatrixFrameZero;    // Cache of frame 0 matrices



		Cache() : frameFraction(0), frame(0)
		{
#ifdef DEBUG
			total = frameNCacheHit = frameZeroCacheHit = 0;
			frameHitPerc = frameZeroHitPerc = 0;
#endif
		}
	};
	mutable Cache _cache;
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
