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
/// <summary>A reference counted wrapper for a Model</summary>
typedef RefCountedResource<Model> ModelHandle;
/// <summary>A reference counted wrapper for a Mesh. </summary>
typedef RefCountedResource<Mesh> MeshHandle;
/// <summary>A reference counted wrapper for a Camera</summary>
typedef RefCountedResource<Camera> CameraHandle;
/// <summary>A reference counted wrapper for a Light</summary>
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
	/// <summary>Return the value of a Model-wide semantic as a FreeValue, null if it does not exist.</summary>
	/// <param name="semantic">The semantic name to retrieve</param>
	/// <returns>A pointer to a FreeValue containing the value of the semantic. If the semantic does not exist,
	/// return NULL</returns>
	const FreeValue* getModelSemantic(const StringHash& semantic) const
	{
		auto it = _data.semantics.find(semantic);
		if (it == _data.semantics.end())
		{
			return NULL;
		}
		return &it->second;
	}

	/// <summary>Get a pointer to the UserData of this model, if such data exist.</summary>
	/// <returns>A pointer to the UserData, as a Reference Counted Void pointer. Cast to appropriate (ref counted)type</returns>
	const RefCountedResource<void>& getUserDataPtr() const
	{
		return this->_data.userDataPtr;
	}

	/// <summary>Get a pointer to the UserData of this model.</summary>
	/// <returns>A pointer to the UserData, as a Reference Counted Void pointer. Cast to appropriate (ref counted)type</returns>
	RefCountedResource<void> getUserDataPtr()
	{
		return this->_data.userDataPtr;
	}

	/// <summary>Set the UserData of this model (wrap the data into a RefCountedResource and cast to Ref Counted void pointer.</summary>
	/// <param name="ptr">The UserData. Must be wrapped in an appropriate RefCountedResource, and then cast into a RefCountedResource to void</param>
	void setUserDataPtr(const RefCountedResource<void>& ptr)
	{
		_data.userDataPtr = ptr;
	}

	/// <summary> Brings the Mesh class name into this class.</summary>
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
			int32_t objectIndex;    //!< Index into mesh, light or camera array, depending on which object list contains this Node
			int32_t materialIndex;  //!< Index of material used on this mesh
			int32_t parentIndex;    //!< Index into Node array; recursively apply ancestor's transforms after this instance's.
			Animation animation;    //!< The animation this node uses
			UInt8Buffer userData;   //!< Optionally, user data

			InternalData() : objectIndex(-1), materialIndex(-1), parentIndex(-1) {}
		};

	public:
		/// <summary>Get which Mesh, Camera or Light this object refers to.</summary>
		/// <returns>The index of the Mesh, Camera or Light array of this node (depending on its type)</returns>
		int32_t getObjectId() const { return _data.objectIndex; }

		/// <summary>Get this Node's name.</summary>
		/// <returns>The name of this object</returns>
		const StringHash& getName() const { return _data.name; }

		/// <summary>Get this Node's parent id.</summary>
		/// <returns>The ID of this Node's parent Node.</returns>
		int32_t getParentID() const { return _data.parentIndex; }

		/// <summary>Get this Node's material id.</summary>
		/// <returns>The ID of this Node's Material</returns>
		int32_t getMaterialIndex() const { return _data.materialIndex; }

		/// <summary>Associate a material with this node (Assign a material id to this node)</summary>
		/// <param name="materialId">The material ID to associate with</param>
		void setMaterialIndex(uint32_t materialId) { _data.materialIndex = materialId;  }

		/// <summary>Get this Node's animation.</summary>
		/// <returns>The animation of this Node</returns>
		const Animation& getAnimation() const { return _data.animation; }

		/// <summary>Get this Node's user data.</summary>
		/// <returns>The user data of this Node</returns>
		const uint8_t* getUserData() const { return _data.userData.data(); }

		/// <summary>Get the size of this Node's user data.</summary>
		/// <returns>Return The size in bytes of the user data of this Node</returns>
		uint32_t getUserDataSize() const { return static_cast<uint32_t>(_data.userData.size()); }

		/// <summary>Set mesh id. Must correlate with the actual position of this node in the data.</summary>
		/// <param name="index">The id to set the index of this node.</param>
		void setIndex(int32_t index) { _data.objectIndex = index; }

		/// <summary>Set the name of this node.</summary>
		/// <param name="name">The std::string to set this node's name to.</param>
		void setName(const StringHash& name) { _data.name = name; }

		/// <summary>Set the parent of this node.</summary>
		/// <param name="parentID">the ID of this node's parent</param>
		void setParentID(int32_t parentID) { _data.parentIndex = parentID; }

		/// <summary>Set the animation of this node.</summary>
		/// <param name="animation">The animation of this node. A copy of the animation object will be created and stored
		/// directly.</param>
		void setAnimation(const Animation& animation) { _data.animation = animation; }

		/// <summary>Set the user data of this node. A bit copy of the data will be made.</summary>
		/// <param name="size">The size, in bytes, of the data</param>
		/// <param name="data">A pointer from which (size) data will be copied.</param>
		void setUserData(uint32_t size, const char* data)
		{
			_data.userData.resize(size);
			memcpy(_data.userData.data(), data, size);
		}

		/// <summary>Get a reference to the internal data of this object. Handle with care.</summary>
		/// <returns>Return a reference to the internal data of this object.</returns>
		InternalData& getInternalData() { return _data; }

		/// <summary>Get a const reference to the data object carrying all internal data of this model.</summary>
		/// <returns>Return a reference to the internal data of this object.</returns>
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
		const pvr::StringHash& getName() const { return _name; }

		/// <summary>Get a reference to the name of the texture.</summary>
		/// <returns>Return reference to the texture name</returns>
		pvr::StringHash& getName() { return _name; }

		/// <summary>Set the name of the texture.</summary>
		/// <param name="name">The std::string to set this texture name to.</param>
		void setName(const StringHash& name) { this->_name = name; }

	private:
		pvr::StringHash _name;
	};

	/// <summary>Class which stores model material info.</summary>
	class Material
	{
	public:
		/// <summary> Constructor. Non initializing </summary>
		Material() : _defaultSemantics(*this) {}

		/// <summary> A blend function</summary>
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

		/// <summary>A blend operation</summary>
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
			std::map<StringHash, FreeValue> materialSemantics; //!< storage for the per-material semantics
			std::map<StringHash, uint32_t> textureIndices; //!< Map of texture (semantic) names to indexes

			StringHash name;          //!<Name of the material
			StringHash effectFile;        //!<Effect filename if using an effect
			StringHash effectName;        //!<Effect name (in the filename) if using an effect

			UInt8Buffer userData;       //!<Raw user data
		};

		/// <summary> This class is provided for convenient compile-time access of the semantic values.</summary>
		class DefaultMaterialSemantics
		{
		public:
			/// <summary> Constructor from a parent material (in order to initialize the material reference). </summary>
			/// <param name="material"> The material that will be the parent of this object.</param>
			DefaultMaterialSemantics(const Material& material): material(&material) {}

			/// <summary>Get material ambient (semantic "AMBIENT")</summary>
			/// <returns>Material ambient</returns>
			glm::vec3 getAmbient() const
			{
				return material->getMaterialAttributeWithDefault<glm::vec3>("AMBIENT", glm::vec3(0.f, 0.f, 0.f));
			}

			/// <summary>Get material diffuse (semantic "DIFFUSE")</summary>
			/// <returns>Material diffuse</returns>
			glm::vec3 getDiffuse() const
			{
				return material->getMaterialAttributeWithDefault<glm::vec3>("DIFFUSE", glm::vec3(1.f, 1.f, 1.f));
			}

			/// <summary>Get material specular (semantic "SPECULAR")</summary>
			/// <returns>Material specular</returns>
			glm::vec3 getSpecular() const
			{
				return material->getMaterialAttributeWithDefault<glm::vec3>("SPECULAR", glm::vec3(0.f, 0.f, 0.f));
			}

			/// <summary>Get material shininess (semantic "SHININESS")</summary>
			/// <returns>Material shininess</returns>
			float getShininess() const
			{
				return material->getMaterialAttributeWithDefault<float>("SHININESS", 0.f);
			}

			/// <summary>Get the diffuse color texture's index (semantic "DIFFUSEMAP", return-1 if not exist)</summary>
			/// <returns>Return the diffuse texture index, if exists, otherwise return -1</returns>
			int32_t getDiffuseTextureIndex() const
			{
				static const StringHash diffuseTexSemantic("DIFFUSEMAP");
				return material->getTextureIndex(diffuseTexSemantic);
			}


			/// <summary>Return the ambient color texture's index (semantic "AMBIENTMAP", return-1 if not exist)</summary>
			/// <returns>Return the Ambient texture index, if exists</returns>
			int32_t getAmbientTextureIndex() const
			{
				static const StringHash diffuseTexSemantic("AMBIENTMAP");
				return material->getTextureIndex(diffuseTexSemantic);
			}

			/// <summary>Get the specular color texture's index (semantic "SPECULARCOLORMAP", return-1 if not exist)</summary>
			/// <returns>Return the specular color texture index</returns>
			int32_t getSpecularColorTextureIndex() const
			{
				static const StringHash diffuseTexSemantic("SPECULARCOLORMAP");
				return material->getTextureIndex(diffuseTexSemantic);
			}


			/// <summary>Get the specular level texture's index (semantic "SPECULARLEVELMAP", return-1 if not exist)</summary>
			/// <returns>Return the specular level texture index</returns>
			int32_t getSpecularLevelTextureIndex() const
			{
				static const StringHash diffuseTexSemantic("SPECULARLEVELMAP");
				return material->getTextureIndex(diffuseTexSemantic);
			}


			/// <summary>Get bumpmap texture index (semantic "NORMALMAP", return-1 if not exist)</summary>
			/// <returns>Return the bumpmap texture index</returns>
			int32_t getBumpMapTextureIndex() const
			{
				static const StringHash diffuseTexSemantic("NORMALMAP");
				return material->getTextureIndex(diffuseTexSemantic);
			}

			/// <summary>Get emissive texture's index  (semantic "EMISSIVEMAP", return-1 if not exist)</summary>
			/// <returns>Return the emissive texture index</returns>
			int32_t getEmissiveTextureIndex() const
			{
				static const StringHash diffuseTexSemantic("EMISSIVEMAP");
				return material->getTextureIndex(diffuseTexSemantic);
			}

			/// <summary>Get glossiness texture's index  (semantic "GLOSSINESSMAP", return-1 if not exist)</summary>
			/// <returns>Return the glossiness texture index</returns>
			int32_t getGlossinessTextureIndex() const
			{
				static const StringHash diffuseTexSemantic("GLOSSINESSMAP");
				return material->getTextureIndex(diffuseTexSemantic);
			}


			/// <summary>Get opacity texture's index  (semantic "OPACITYMAP", return-1 if not exist)</summary>
			/// <returns>Return the opacity texture index</returns>
			int32_t getOpacityTextureIndex() const
			{
				static const StringHash diffuseTexSemantic("OPACITYMAP");
				return material->getTextureIndex(diffuseTexSemantic);
			}


			/// <summary>Get reflection texture's index (semantic "REFLECTIONMAP", return-1 if not exist)</summary>
			/// <returns>Return the reflection texture index</returns>
			int32_t getReflectionTextureIndex() const
			{
				static const StringHash diffuseTexSemantic("REFLECTIONMAP");
				return material->getTextureIndex(diffuseTexSemantic);
			}

			/// <summary>Return refraction texture's index (semantic "REFRACTIONMAP", return-1 if not exist)</summary>
			/// <returns>Return the refraction texture index</returns>
			int32_t getRefractionTextureIndex() const
			{
				static const StringHash diffuseTexSemantic("REFRACTIONMAP");
				return material->getTextureIndex(diffuseTexSemantic);
			}

			/// <summary>Get this material opacity (semantic "OPACITY")</summary>
			/// <returns>Return the material opacity</returns>
			float getOpacity() const
			{
				return material->getMaterialAttributeWithDefault<float>("OPACITY", 1.f);
			}


			/// <summary>Get the blend function for Source Color (semantic "BLENDSRCCOLOR")</summary>
			/// <returns>Return source color blend function</returns>
			BlendFunction getBlendSrcRGB() const
			{
				return material->getMaterialAttributeWithDefault<BlendFunction>("BLENDSRCCOLOR", BlendFunction::BlendFuncOne);
			}

			/// <summary>Get the blend function for Source Alpha (semantic "BLENDSRCALPHA")</summary>
			/// <returns>Return source alpha blend function</returns>
			BlendFunction getBlendSrcA() const
			{
				return material->getMaterialAttributeWithDefault<BlendFunction>("BLENDSRCALPHA", BlendFunction::BlendFuncOne);
			}

			/// <summary>Get the blend function for Destination Color (semantic "BLENDDSTCOLOR")</summary>
			/// <returns>Return destination color blend function</returns>
			BlendFunction getBlendDstRGB() const
			{
				return material->getMaterialAttributeWithDefault<BlendFunction>("BLENDDSTCOLOR", BlendFunction::BlendFuncZero);
			}

			/// <summary>Get the blend function for Destination Alpha (semantic "BLENDDSTALPHA")</summary>
			/// <returns>Return destination alpha blend function</returns>
			BlendFunction getBlendDstA() const
			{
				return material->getMaterialAttributeWithDefault<BlendFunction>("BLENDDSTALPHA", BlendFunction::BlendFuncZero);
			}

			/// <summary>Get the blend operation for Color (semantic "BLENDOPCOLOR")</summary>
			/// <returns>Return the color's blend operator</returns>
			BlendOperation getBlendOpRGB() const
			{
				return material->getMaterialAttributeWithDefault<BlendOperation>("BLENDOPCOLOR", BlendOperation::BlendOpAdd);
			}

			/// <summary>Return the blend operation for Alpha (semantic "BLENDOPALPHA")</summary>
			/// <returns>Return the alpha's blend operator</returns>
			BlendOperation getBlendOpA() const
			{
				return material->getMaterialAttributeWithDefault<BlendOperation>("BLENDOPALPHA", BlendOperation::BlendOpAdd);
			}

			/// <summary>Get the blend color (semantic "BLENDCOLOR")</summary>
			/// <returns>Return blend color</returns>
			glm::vec4 getBlendColor() const
			{
				return material->getMaterialAttributeWithDefault<glm::vec4>("BLENDCOLOR", glm::vec4(0.f, 0.f, 0.f, 0.f));
			}

			/// <summary>Return the blend factor (semantic "BLENDFACTOR")</summary>
			/// <returns>Return blend factor</returns>
			glm::vec4 getBlendFactor() const
			{
				return material->getMaterialAttributeWithDefault<glm::vec4>("BLENDFACTOR", glm::vec4(0.f, 0.f, 0.f, 0.f));
			}

		private:
			const Material* material;
		};

	public:
		/// <summary> Get a Default Semantics adapter for this object. This is just a convenience object to access
		/// the default semantics through compile time functions.</summary>
		/// <returns>The default semantics for this object.</returns>
		const DefaultMaterialSemantics& defaultSemantics() const
		{
			return _defaultSemantics;
		}

		/// <summary> Set a material attribute by Semantic Name. Any semantic name may be passed, but some of them
		/// may be additionally accessed through Default Semantics.</summary>
		/// <param name="semantic">The semantic to set</param>
		/// <param name="value">The value to set</param>
		void setMaterialAttribute(const StringHash& semantic, const FreeValue& value)
		{
			_data.materialSemantics[semantic] = value;
		}

		/// <summary> Retrieve a material attribute by Semantic Name. If it does not exist, NULL will be returned.</summary>
		/// <param name="semantic">The semantic to retrieve</param>
		/// <returns> A pointer to the value of the semantic with name <paramRef name="semantic"/>. If the semantic
		/// does not exist, returns null </returns>
		const FreeValue* getMaterialAttribute(const StringHash& semantic) const
		{
			auto it = _data.materialSemantics.find(semantic);
			if (it != _data.materialSemantics.end())
			{
				return &it->second;
			}
			return NULL;
		}

		/// <summary> Retrieve a material attribute value, by Semantic Name, as a specific type.
		/// If it does not exist, the default value will be returned.</summary>
		/// <typeparam name="Type">The type that the value will be interpreded as</param>
		/// <param name="semantic">The semantic name of the value to retrieve</param>
		/// <param name="defaultAttrib">The default value. This will be returned if semantic does not exist</param>
		/// <returns> The value of the semantic with name <paramRef name="semantic"/>. If the semantic
		/// does not exist, returns the default value</returns>
		template<typename Type>
		const Type getMaterialAttributeWithDefault(const StringHash& semantic, const Type& defaultAttrib) const
		{
			auto* val = getMaterialAttribute(semantic);
			if (val)
			{
				return val->interpretValueAs<Type>();
			}
			return defaultAttrib;
		}

		/// <summary> Retrieve a material attribute value, by Semantic Name, as a specific type.
		/// If it does not exist, null will be returned.</summary>
		/// <typeparam name="Type">The type that the value will be interpreded as</param>
		/// <param name="semantic">The semantic name of the value to retrieve</param>
		/// <returns> A pointer to the value of the semantic with name <paramRef name="semantic"/>. If the semantic
		/// does not exist, returns Null</returns>
		template<typename Type>
		const Type* getMaterialAttributeAs(const StringHash& semantic) const
		{
			auto* val = getMaterialAttribute(semantic);
			if (val)
			{
				return &val->interpretValueAs<Type>();
			}
			return NULL;
		}

		/// <summary>Query if a semantic exists (Either texture or attribute)</summary>
		/// <param name="semantic">The semantic name to check.</param>
		/// <returns>True if either a texture or a material attribute with the specified
		/// semantic exists, otherwise false</returns>
		bool hasSemantic(const StringHash& semantic) const
		{
			return hasMaterialTexture(semantic) || hasMaterialAttribute(semantic);
		}

		/// <summary>Check if a material texture with the specified semantic exists.</summary>
		/// <param name="semantic">The semantic of the material texture to check.</param>
		/// <returns>True if the material texture exists, otherwise false</returns>
		bool hasMaterialTexture(const StringHash& semantic) const
		{
			return getTextureIndex(semantic) != -1;
		}
		/// <summary>Check if a material attribute with the specified semantic exists.</summary>
		/// <param name="semantic">The semantic of the material attribute to check.</param>
		/// <returns>True if the material attribute exists, otherwise false</returns>
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

		/// <summary>Find a texture with the specified semantic. If it exists, returns its index otherwise -1.</summary>
		/// <param name="semantic">The semantic of the texture to retrieve.</param>
		/// <returns>If the index with this semantic exists, return its index. Otherwise, return -1.</returns>
		int32_t getTextureIndex(const StringHash& semantic) const
		{
			auto it = _data.textureIndices.find(semantic);
			return (it == _data.textureIndices.end()) ? -1 : static_cast<int32_t>(it->second);
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
		UInt8Buffer userData;
		InternalData _data;
		DefaultMaterialSemantics _defaultSemantics;
	};

public:

	/// <summary>Struct containing the internal data of the Model.</summary>
	struct InternalData
	{
		pvr::ContiguousMap<StringHash, FreeValue> semantics; //!< Store of the semantics

		float clearColor[3];      //!< Background color
		float ambientColor[3];    //!< Ambient color

		std::vector<Mesh> meshes;   //!< Mesh array. Any given mesh can be referenced by multiple Nodes.
		std::vector<Camera> cameras;  //!< Camera array. Any given mesh can be referenced by multiple Nodes.
		std::vector<Light> lights;    //!< Light array. Any given mesh can be referenced by multiple Nodes.
		std::vector<Texture> textures;  //!< Textures in this Model
		std::vector<Material> materials;//!< Materials in this Model
		std::vector<Node> nodes;    //!< Nodes array. The nodes are sorted thus: First Mesh Nodes, then Light Nodes, then Camera nodes.

		uint32_t numMeshNodes;      //!< Number of items in the nodes array which are Meshes
		uint32_t numLightNodes;     //!< Number of items in the nodes array which are Meshes
		uint32_t numCameraNodes;      //!< Number of items in the nodes array which are Meshes

		uint32_t numFrames;       //!< Number of frames of animation
		float currentFrame;     //!< Current frame in the animation
		uint32_t FPS;           //!< The frames per second the animation should be played at

		UInt8Buffer userData;     //!< Custom raw data stored by the user

		float units;          //!< Unit scaling
		uint32_t flags;         //!< Flags
		RefCountedResource<void> userDataPtr; //!< Can be used to store any kind of data that the user wraps in a refcounted resource

		/// <summary>Constructor. Initializing to empty.</summary>
		InternalData() : numMeshNodes(0), numLightNodes(0), numCameraNodes(0), numFrames(0), currentFrame(0), FPS(30), units(1), flags(0)
		{
			memset(clearColor, 0, sizeof(clearColor));
			memset(ambientColor, 0, sizeof(ambientColor));
		}
	};
public:
	/// <summary>Free the vertex data (Vertex attribute values, Vertex Index values) of all meshes to free memory.
	/// Usually called after VBOs/IBOs have been created. Any other data of the Mesh are unaffected.</summary>
	void releaseVertexData()
	{
		for (uint32_t i = 0; i < getNumMeshes(); ++i)
		{
			releaseVertexData(i);
		}
	}

	/// <summary>Free the vertex data (Vertex attribute values, Vertex Index values) of a single mesh to free memory.
	/// Usually called after VBOs/IBOs have been created. Any other data of the Mesh are unaffected.</summary>
	/// <param name="meshId">The meshId of the mesh whose vertex data to free</param>
	void releaseVertexData(uint32_t meshId)
	{
		Mesh& mesh = getMesh(meshId);
		for (uint32_t i = 0; i < mesh.getNumDataElements(); ++i)
		{
			mesh.removeData(i);
		}
		mesh.getFaces().setData(0, 0);
	}

	/// <summary>Return the world-space position of a light. Corresponds to the Model's current frame of animation.
	/// </summary>
	/// <param name="lightId">The node for which to return the world matrix.</param>
	/// <returns>Return The world matrix of (nodeId).</returns>
	glm::vec3 getLightPosition(uint32_t lightId) const;

	/// <summary>Return the model-to-world matrix of a node. Corresponds to the Model's current frame of animation. This
	/// version will store a copy of the matrix in an internal cache so that repeated calls for it will use the cached
	/// copy of it. Will also store the cached versions of all parents of this node, or use cached versions of them if
	/// they exist. Use this if you have long hierarchies and/or repeated calls per frame.</summary>
	/// <param name="nodeId">The node for which to return the world matrix.</param>
	/// <returns>Return The world matrix of (nodeId).</returns>
	glm::mat4x4 getWorldMatrix(uint32_t nodeId) const;

	/// <summary>Return the model-to-world matrix of a node. Corresponds to the Model's current frame of animation. This
	/// version will not use caching and will recalculate the matrix. Faster if the matrix is only used a few times.
	/// </summary>
	/// <param name="nodeId">The node for which to return the world matrix</param>
	/// <returns>return The world matrix of (nodeId)</returns>
	glm::mat4x4 getWorldMatrixNoCache(uint32_t nodeId) const;

	/// <summary>Return the model-to-world matrix of a specified bone. Corresponds to the Model's current frame of
	/// animation. This version will use caching.</summary>
	/// <param name="skinNodeID">The node for which to return the world matrix</param>
	/// <param name="boneId">The bone for which to return the world matrix</param>
	/// <returns>Return The world matrix of (nodeId, boneID)</returns>
	glm::mat4x4 getBoneWorldMatrix(uint32_t skinNodeID, uint32_t boneId) const;

	/// <summary>Transform a custom matrix with a node's parent's transformation. Allows a custom matrix to be applied to a
	/// node, while honoring the hierarchical transformations applied by its parent hierarchy.</summary>
	/// <param name="nodeId">The node whose parents will be applied to the transformation.</param>
	/// <param name="localMatrix">The matrix to transform</param>
	/// <returns>Return The localMatrix transformation, modified by the hierarchical transformations of the node.
	/// </returns>
	/// <remarks>This function can be used to implement custom procedural animation/kinematics schemes, in which case
	/// some nodes may need to have their animations customly defined, but must still honor their parents'
	/// transformations.</remarks>
	glm::mat4x4 toWorldMatrix(uint32_t nodeId, const glm::mat4& localMatrix) const
	{
		int32_t parentID = _data.nodes[nodeId].getParentID();
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
	glm::mat4x4 getLocalMatrix(uint32_t nodeId) const
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
	/// <returns>The clear color (float array R,G,B,A).</returns>
	const float* getBackgroundColor() const { return _data.clearColor; }

	/// <summary>Get the number of distinct camera objects. May be different than the actual number of Camera
	/// Instances (Nodes).</summary>
	/// <returns>Return The number of distinct camera objects.</returns>
	uint32_t getNumCameras() const { return static_cast<uint32_t>(_data.cameras.size()); }

	/// <summary>Get the number of Camera nodes in this model</summary>
	/// <returns>Return The number of Camera instances (Nodes) in this Model.</returns>
	uint32_t getNumCameraNodes() const { return getNumCameras(); /* Will be changed at a future revision */ }

	/// <summary>Get a Camera from this model</summary>
	/// <param name="cameraIndex">The index of the camera. Valid values (0 to getNumCameras()-1)</param>
	/// <returns>Return the camera</returns>
	const Camera& getCamera(uint32_t cameraIndex) const
	{
		assertion(cameraIndex < getNumCameras(), "Invalid camera index");
		return _data.cameras[cameraIndex];
	}
	/// <summary>Get a Camera from this model</summary>
	/// <param name="cameraIndex">The index of the camera. Valid values (0 to getNumCameras()-1)</param>
	/// <returns>Return the camera</returns>
	Camera& getCamera(uint32_t cameraIndex)
	{
		assertion(cameraIndex < getNumCameras(), "Invalid camera index");
		return _data.cameras[cameraIndex];
	}

	/// <summary>Get a specific CameraNode.</summary>
	/// <param name="cameraNodeIndex">The Index of a Camera Node. It is not the same as the NodeID. Valid values: (0
	/// .. getNumCameraNodes()-1)</param>
	/// <returns>Return The Camera Node</returns>
	const Node& getCameraNode(uint32_t cameraNodeIndex) const
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
	uint32_t getNodeIdFromCameraId(uint32_t cameraNodeIndex) const
	{
		// Camera nodes are after the mesh and light nodes in the array
		assertion(cameraNodeIndex < getNumCameraNodes(), "Invalid camera node index");
		return getNumMeshes() + getNumLights() + cameraNodeIndex;
	}

	/// <summary>Get the number of distinct Light objects. May be different than the actual number of Light Instances
	/// (Nodes).</summary>
	/// <returns>Return The number of distinct Light objects in this Model.</returns>
	uint32_t getNumLights() const { return static_cast<uint32_t>(_data.lights.size()); }

	/// <summary>Get the number of Light nodes.</summary>
	/// <returns>Return The number of Light instances (Nodes) in this Model.</returns>
	uint32_t getNumLightNodes() const { return getNumLights(); /* Will be changed at a future revision */ }

	/// <summary>Get the light object with the specific Light Index.</summary>
	/// <param name="lightIndex">The index of the light. Valid values (0..getNumLights()-1)</param>
	/// <returns>Return the light</returns>
	const Light& getLight(uint32_t lightIndex) const
	{
		assertion(lightIndex < getNumLights(), "Invalid light index");
		return _data.lights[lightIndex];
	}
	/// <summary>Get the light object with the specific Light Index.</summary>
	/// <param name="lightIndex">The index of the light. Valid values (0..getNumLights()-1)</param>
	/// <returns>Return the light</returns>
	Light& getLight(uint32_t lightIndex)
	{
		assertion(lightIndex < getNumLights(), "Invalid light index");
		return _data.lights[lightIndex];
	}

	/// <summary>Get a specific Light Node.</summary>
	/// <param name="lightNodeIndex">The Index of the Light Node. It is not the same as the NodeID. Valid values: (0
	/// to getNumLightNodes()-1)</param>
	/// <returns>Return the light node</returns>
	const Node& getLightNode(uint32_t lightNodeIndex) const
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
	uint32_t getNodeIdFromLightNodeId(uint32_t lightNodeIndex) const
	{
		assertion(lightNodeIndex < getNumLightNodes(), "Invalid light node index");
		// Light nodes are after the mesh nodes in the array
		return getNumMeshNodes() + lightNodeIndex;
	}

	/// <summary>Get the number of distinct Mesh objects. Unless each Mesh appears at exactly one Node, may be
	/// different than the actual number of Mesh instances.</summary>
	/// <returns>Return The number of different Mesh objects in this Model.</returns>
	uint32_t getNumMeshes() const { return static_cast<uint32_t>(_data.meshes.size()); }

	/// <summary>Get the number of Mesh nodes.</summary>
	/// <returns>Return The number of Mesh instances (Nodes) in this Model.</returns>
	uint32_t getNumMeshNodes() const { return _data.numMeshNodes; }

	/// <summary>Get the Mesh object with the specific Mesh Index. Constant overload.</summary>
	/// <param name="meshIndex">The index of the Mesh. Valid values (0..getNumMeshes()-1)</param>
	/// <returns>The mesh with id <paramref name="meshIndex."/>Const ref.</returns>
	const Mesh& getMesh(uint32_t meshIndex) const { return _data.meshes[meshIndex]; }

	/// <summary>Get the Mesh object with the specific Mesh Index.</summary>
	/// <param name="index">The index of the Mesh. Valid values (0..getNumMeshes()-1)</param>
	/// <returns>Return the mesh from this model</returns>
	Mesh& getMesh(uint32_t index)
	{
		assertion(index < getNumMeshes(), "Invalid mesh index");
		return _data.meshes[index];
	}

	/// <summary>Get a specific Mesh Node.</summary>
	/// <param name="meshIndex">The Index of the Mesh Node. For meshes, it is the same as the NodeID. Valid values: (0
	/// to getNumMeshNodes()-1)</param>
	/// <returns>Return he Mesh Node from this model</returns>
	const Node& getMeshNode(uint32_t meshIndex) const
	{
		assertion(meshIndex < getNumMeshNodes(), "Invalid mesh index");
		// Mesh nodes are at the start of the array
		return getNode(meshIndex);
	}

	/// <summary>Get a specific Mesh Node.</summary>
	/// <param name="meshIndex">The Index of the Mesh Node. For meshes, it is the same as the NodeID. Valid values: (0
	/// to getNumMeshNodes()-1)</param>
	/// <returns>Return he Mesh Node from this model</returns>
	Node& getMeshNode(uint32_t meshIndex)
	{
		assertion(meshIndex < getNumMeshNodes(), "Invalid mesh index");
		// Mesh nodes are at the start of the array
		return getNode(meshIndex);
	}

	/// <summary>Connect mesh to a mesh node (i.e. set the node's mesh to the mesh</summary>
	/// <param name="meshId">The mesh id</param>
	/// <param name="meshNodeId">The mesh node id to connect to</param>
	void connectMeshWithMeshNode(uint32_t meshId, uint32_t meshNodeId)
	{
		getMeshNode(meshNodeId).setIndex(meshId);
	}

	/// <summary>Connect mesh to number of mesh nodes</summary>
	/// <param name="meshId">The mesh id</param>
	/// <param name="beginMeshNodeId">Begin mesh node id (inclusive)</param>
	/// <param name="endMeshNodeId">End mesh node id (inclusive)</param>
	void connectMeshWithMeshNodes(uint32_t meshId, uint32_t beginMeshNodeId, uint32_t endMeshNodeId)
	{
		for (uint32_t  i = beginMeshNodeId; i <= endMeshNodeId; ++i)
		{
			connectMeshWithMeshNode(meshId, i);
		}
	}

	/// <summary>Assign material id to number of mesh nodes</summary>
	/// <param name="materialIndex">Material id</param>
	/// <param name="beginMeshNodeId">Begin mesh node id (inclusive)</param>
	/// <param name="endMeshNodeId">end mesh node id (inclusive)</param>
	void assignMaterialToMeshNodes(uint32_t materialIndex, uint32_t beginMeshNodeId, uint32_t endMeshNodeId)
	{
		for (uint32_t  i = beginMeshNodeId; i <= endMeshNodeId; ++i)
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
	uint32_t getNodeIdForMeshNodeId(uint32_t meshNodeIndex) const
	{
		debug_assertion(meshNodeIndex < getNumMeshNodes(), "invalid mesh node index");
		// Camera nodes are after the mesh and light nodes in the array
		return meshNodeIndex;
	}

	/// <summary>Get an iterator to the beginning of the meshes.</summary>
	/// <returns>Return an iterator</returns>
	std::vector<Mesh>::iterator beginMeshes() { return _data.meshes.begin(); }

	/// <summary>Get an iterator past the end of the meshes.</summary>
	/// <returns>Iterator to one past the last item of the mesh array</returns>
	std::vector<Mesh>::iterator endMeshes() { return _data.meshes.end(); }

	/// <summary>Get a const_iterator to the beginning of the meshes.</summary>
	/// <returns>Iterator to the start of the mesh array</returns>
	std::vector<Mesh>::const_iterator beginMeshes() const { return _data.meshes.begin(); }

	/// <summary>Get a const_iterator past the end of the meshes.</summary>
	/// <returns>Iterator to one past the last item of the mesh array</returns>
	std::vector<Mesh>::const_iterator endMeshes() const { return _data.meshes.end(); }

	/// <summary>Get the total number of nodes (Meshes, Cameras, Lights, others (helpers etc)).</summary>
	/// <returns>Return number of nodes in this model</returns>
	uint32_t getNumNodes() const { return static_cast<uint32_t>(_data.nodes.size()); }

	/// <summary>Get the node with the specified index.</summary>
	/// <param name="index">The index of the node to get</param>
	/// <returns>Return The Node from this scene</returns>
	const Node& getNode(uint32_t index) const { return _data.nodes[index]; }

	/// <summary>Get the node with the specified index.</summary>
	/// <param name="index">The index of the node to get</param>
	/// <returns>Return The Node from this scene</returns>
	Node& getNode(uint32_t index) { return _data.nodes[index]; }

	/// <summary>Get the number of distinct Textures in the scene.</summary>
	/// <returns>Return number of distinct textures</returns>
	uint32_t getNumTextures() const { return static_cast<uint32_t>(_data.textures.size()); }

	/// <summary>Get the texture with the specified index.</summary>
	/// <param name="index">The index of the texture to get</param>
	/// <returns>Return a texture from this scene</returns>
	const Texture& getTexture(uint32_t index) const { return _data.textures[index]; }

	/// <summary>Get the number of distinct Materials in the scene.</summary>
	/// <returns>Return number of materials in this scene</returns>
	uint32_t getNumMaterials() const { return static_cast<uint32_t>(_data.materials.size()); }

	/// <summary>Get the material with the specified index.</summary>
	/// <param name="index">The index of material to get</param>
	/// <returns>Return a material from this scene</returns>
	const Material& getMaterial(uint32_t index) const { return _data.materials[index]; }

	/// <summary>Add a material to this model, and gets its (just created) material id</summary>
	/// <param name="material">The the material to add to the materials of this model</param>
	/// <returns>The material id generated for the new material.</returns>
	uint32_t addMaterial(const Material& material)
	{
		_data.materials.push_back(material);
		return static_cast<uint32_t>(_data.materials.size() - 1);
	}

	/// <summary>Get the material with the specified index.</summary>
	/// <param name="index">The index of material to get</param>
	/// <returns>Return a material from this scene</returns>
	Material& getMaterial(uint32_t index) { return _data.materials[index]; }

	/// <summary>Get the total number of frames in the scene. The total number of usable animated frames is limited to
	/// exclude (numFrames - 1) but include any partial number up to (numFrames - 1). Example: If there are 100 frames
	/// of animation, the highest frame number allowed is 98, since that will blend between frames 98 and 99. (99
	/// being of course the 100th frame.)</summary>
	/// <returns>Return the number of frames in this model</returns>
	uint32_t getNumFrames() const { return _data.numFrames ? _data.numFrames : 1; }

	/// <summary>Set the current frame. Affects future animation calls (getWorldMatrix etc.).</summary>
	/// <param name="frame">The current frame. Can be fractional, in which case interpolation will normally be
	/// performed</param>
	/// <returns>Return true on success, false if out of bounds.</returns>
	bool setCurrentFrame(float frame);

	/// <summary>Get the current frame of the scene.</summary>
	/// <returns>Return the current frame</returns>
	float getCurrentFrame();

	/// <summary>Set the expected FPS of the animation.</summary>
	/// <param name="fps">FPS of the animation</param>
	void setFPS(uint32_t fps) { _data.FPS = fps; }

	/// <summary>Get the FPS this animation was created for.</summary>
	/// <returns>Get the expected FPS of the animation.</returns>
	uint32_t getFPS()const { return _data.FPS; }

	/// <summary>Set custom user data.</summary>
	/// <param name="size">The size, in bytes, of the data.</param>
	/// <param name="data">Pointer to the raw data. (size) bytes will be copied as-is from this pointer.</param>
	void setUserData(uint32_t size, const char* data);

	/// <summary>Only used for custom model creation. Allocate an number of cameras.</summary>
	/// <param name="count">Number of camera to allocate in this scene</param>
	void allocCameras(uint32_t count);

	/// <summary>Only used for custom model creation. Allocate a number of lights.</summary>
	/// <param name="count">number of lights to allocate in this scene</param>
	void allocLights(uint32_t count);

	/// <summary>Only used for custom model creation. Allocate a number of meshes.</summary>
	/// <param name="count">number of meshes to allocate in this scene</param>
	void allocMeshes(uint32_t count);

	/// <summary>Only used for custom model creation. Allocate a number of nodes.</summary>
	/// <param name="count">number of nodes to allocate in this scene</param>
	void allocNodes(uint32_t count);

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
	void getCameraProperties(int32_t cameraIdx, float& fov, glm::vec3& from, glm::vec3& to, glm::vec3& up) const;

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
	void getCameraProperties(int32_t cameraIdx, float& fov, glm::vec3& from, glm::vec3& to, glm::vec3& up, float& nearClip, float& farClip) const;

	/// <summary>Get the direction of a spot or directional light.</summary>
	/// <param name="lightIdx">index of the light.</param>
	/// <param name="direction">The direction of the light.</param>
	/// <remarks>If lightIdx >= number of lights, an error will be logged and this function will have no effect.
	/// </remarks>
	void getLightDirection(int32_t lightIdx, glm::vec3& direction) const;

	/// <summary>Get the position of a point or spot light.</summary>
	/// <param name="lightIdx">light index.</param>
	/// <param name="position">The position of the light.</param>
	/// <returns>False if <paramref name="lightIdx"/>does not exist</returns>
	/// <remarks>If lightIdx >= number of lights, an error will be logged and this function will have no effect.
	/// </remarks>
	void getLightPosition(int32_t lightIdx, glm::vec3& position) const;

	/// <summary>Get the position of a point or spot light.</summary>
	/// <param name="lightIdx">light index.</param>
	/// <param name="position">The position of the light.</param>
	/// <remarks>If lightIdx >= number of lights, an error will be logged and this function will have no effect.
	/// </remarks>
	void getLightPosition(int32_t lightIdx, glm::vec4& position) const;

	/// <summary>Free the resources of this model.</summary>
	void destroy()
	{
		_data = InternalData();
		initCache();
	}

	/// <summary>Allocate the specified number of mesh nodes.</summary>
	/// <param name="no">The number of mesh nodes to allocate</param>
	void allocMeshNodes(uint32_t no);
private:
	InternalData _data;

	struct Cache
	{
		float frameFraction;
		uint32_t  frame;

#ifdef DEBUG
		int64_t total, frameNCacheHit, frameZeroCacheHit;
		float frameHitPerc, frameZeroHitPerc;
#endif
		std::vector<float> cachedFrame;       // Cache indicating the frames at which the matrix cache was filled
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

typedef Model::Material Material; ///< Export the Material into the pvr::assets namespace
typedef Model::Node Node; ///< Export the Node into the pvr::assets namespace
typedef Model::Mesh::VertexAttributeData VertexAttributeData;///< Export the VertexAttributeData into the pvr::assets namespace

/// <summary>A ref counted handle to a Node. Created by calling getNodeHandle. Uses
///.the Model's reference count</summary>
typedef RefCountedResource<Node> NodeHandle;
/// <summary>A ref counted handle to a Material. Created by calling getMaterialHandle. Uses
///.the Model's reference count</summary>
typedef RefCountedResource<Material> MaterialHandle;

/// <summary>Create a Reference Counted Handle to a Mesh from a Model. The handle provided
///.works as any other RefCountedResource smart pointer, and uses the "shared ref count"
/// feature that allows the created MeshHandle to use the Model's reference count (e.g. if
/// the Mesh is copied, the Model's reference count increases, and if all references to the
/// Model are released, the Model will be kept alive by this reference.</summary>
/// <param name="model">The model to whom the mesh we will create the handle for belongs</param>
/// <param name="meshId">The ID of the meshId inside <paramRef name="model"/></param>
/// <returns>A MeshHandle to the Mesh. It shares the ref counting of
/// <paramRef name="model"/></returns
inline MeshHandle getMeshHandle(ModelHandle model, int meshId)
{
	MeshHandle handle;
	handle.shareRefCountFrom(model, &model->getMesh(meshId));
	return handle;
}

/// <summary>Create a Reference Counted Handle to a Material from a Model. The handle provided
///.works as any other RefCountedResource smart pointer, and uses the "shared ref count"
/// feature that allows the created MaterialHandle to use the Model's reference count (e.g. if
/// the Material is copied, the Model's reference count increases, and if all references to the
/// Model are released, the Model will be kept alive by this reference.</summary>
/// <param name="model">The model to whom the material we will create the handle for belongs</param>
/// <param name="materialId">The ID of the material inside <paramRef name="model"/></param>
/// <returns>A MaterialHandle to the Material. It shares the ref counting of
/// <paramRef name="model"/></returns
inline MaterialHandle getMaterialHandle(ModelHandle model, int materialId)
{
	MaterialHandle handle;
	handle.shareRefCountFrom(model, &model->getMaterial(materialId));
	return handle;
}

/// <summary>Create a Reference Counted Handle to a Light from a Model. The handle provided
///.works as any other RefCountedResource smart pointer, and uses the "shared ref count"
/// feature that allows the created LightHandle to use the Model's reference count (e.g. if
/// the Light is copied, the Model's reference count increases, and if all references to the
/// Model are released, the Model will be kept alive by this reference.</summary>
/// <param name="model">The model to whom the Light we will create the handle for belongs</param>
/// <param name="lightId">The ID of the Light inside <paramRef name="model"/></param>
/// <returns>A LightHandle to the Light. It shares the ref counting of
/// <paramRef name="model"/></returns>
inline LightHandle getLightHandle(ModelHandle model, int lightId)
{
	LightHandle handle;
	handle.shareRefCountFrom(model, &model->getLight(lightId));
	return handle;
}

/// <summary>Create a Reference Counted Handle to a Camera from a Model. The handle provided
///.works as any other RefCountedResource smart pointer, and uses the "shared ref count"
/// feature that allows the created CameraHandle to use the Model's reference count (e.g. if
/// the Camera is copied, the Model's reference count increases, and if all references to the
/// Model are released, the Model will be kept alive by this reference.</summary>
/// <param name="model">The model to whom the Camera we will create the handle for belongs</param>
/// <param name="cameraId">The ID of the Camera inside <paramRef name="model"/></param>
/// <returns>A CameraHandle to the Camera. It shares the ref counting of
/// <paramRef name="model"/></returns>
inline CameraHandle getCameraHandle(ModelHandle model, int cameraId)
{
	CameraHandle handle;
	handle.shareRefCountFrom(model, &model->getCamera(cameraId));
	return handle;
}

/// <summary>Create a Reference Counted Handle to a Node from a Model. The handle provided
///.works as any other RefCountedResource smart pointer, and uses the "shared ref count"
/// feature that allows the created NodeHandle to use the Model's reference count (e.g. if
/// the Node is copied, the Model's reference count increases, and if all references to the
/// Model are released, the Model will be kept alive by this reference.</summary>
/// <param name="model">The model to whom the Node we will create the handle for belongs</param>
/// <param name="nodeId">The ID of the Node inside <paramRef name="model"/></param>
/// <returns>A Node Handle to the Node. It shares the ref counting of
/// <paramRef name="model"/></returns>
inline NodeHandle getNodeHandle(ModelHandle model, int nodeId)
{
	NodeHandle handle;
	handle.shareRefCountFrom(model, &model->getNode(nodeId));
	return handle;
}
}
}
