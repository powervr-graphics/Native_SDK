/*!
\brief The DescriptorSet class, representing a "directory" of shader-accessible objects
like Buffers and Images
\file PVRVk/DescriptorSetVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRVk/DeviceVk.h"
#include "PVRVk/PipelineConfigVk.h"
#include "PVRVk/BufferVk.h"
namespace pvrvk {

/// <summary>Contains all information required to create a Descriptor Set Layout. This is the number of Textures,
/// Samplers, Uniform Buffer Objects, and Shader Storage Buffer Objects bound for any shader stage.</summary>
struct DescriptorSetLayoutCreateInfo
{
private:
	struct DescriptorSetLayoutBinding
	{
		uint16_t binding;
		VkDescriptorType descriptorType;
		uint16_t descriptorCount;
		VkShaderStageFlags stageFlags;
		Sampler   immutableSampler;
		DescriptorSetLayoutBinding() : descriptorCount(1), stageFlags(VkShaderStageFlags::e_ALL) {}
		DescriptorSetLayoutBinding(uint16_t bindIndex, VkDescriptorType descType,
		                           uint16_t descriptorCount = 1, VkShaderStageFlags stageFlags = VkShaderStageFlags::e_ALL,
		                           const Sampler& immutableSampler = Sampler())
			: binding(bindIndex), descriptorType(descType), descriptorCount(descriptorCount), stageFlags(stageFlags),
			  immutableSampler(immutableSampler) {}

		bool operator==(const DescriptorSetLayoutBinding& rhs)const
		{
			return binding == rhs.binding &&
			       descriptorType == rhs.descriptorType &&
			       descriptorCount == rhs.descriptorCount &&
			       stageFlags == rhs.stageFlags;
		}

		bool operator !=(const DescriptorSetLayoutBinding& rhs)const
		{
			return !(*this == rhs);
		}
	};
	std::vector<DescriptorSetLayoutBinding> descLayoutInfo;

public:

	/// <summary>Set the buffer binding of Descriptor Objects in the specified shader stages.</summary>
	/// <param name="binding">The index to which the binding will be added</param>
	/// <param name="descriptorType">The type of descriptor</param>
	/// <param name="descriptorCount">The number of descriptors to add starting at that index</param>
	/// <param name="stageFlags">The shader stages for which the number of bindings is set to (count)</param>
	/// <param name="immutableSampler">If an immutable sampler is set, pass it here. See vulkan spec</param>
	/// <returns>This object (allows chaining of calls)</returns>
	DescriptorSetLayoutCreateInfo& setBinding(uint16_t binding, VkDescriptorType descriptorType,
	    uint16_t descriptorCount = 1, VkShaderStageFlags stageFlags = VkShaderStageFlags::e_ALL,
	    Sampler immutableSampler = Sampler())
	{
		const DescriptorSetLayoutBinding layoutBinding(binding, descriptorType, descriptorCount, stageFlags, immutableSampler);
		std::vector<DescriptorSetLayoutBinding>::iterator it = std::find_if(descLayoutInfo.begin(),
		    descLayoutInfo.end(), [&](const DescriptorSetLayoutBinding & info)
		{
			return info.binding == layoutBinding.binding;
		});
		if (it != descLayoutInfo.end())
		{
			(*it) = layoutBinding;
		}
		else
		{
			descLayoutInfo.push_back(layoutBinding);
		}
		return *this;
	}

	/// <summary>Clear all entries</summary>
	/// <returns>Return this for chaining</returns>
	DescriptorSetLayoutCreateInfo& clear()
	{
		descLayoutInfo.clear();
		return *this;
	}

	/// <summary>Return the number of images in this object</summary>
	/// <returns>the number of images in this object</returns>
	uint16_t getNumBindings()const { return (uint16_t)descLayoutInfo.size(); }

	/// <summary>Equality operator. Does deep comparison of the contents.</summary>
	/// <param name="rhs">The right-hand side argument of the operator.</param>
	/// <returns>True if the layouts have identical bindings</returns>
	bool operator==(const DescriptorSetLayoutCreateInfo& rhs)const
	{
		if (getNumBindings() != rhs.getNumBindings()) { return false; }
		for (uint32_t i = 0; i < getNumBindings(); ++i)
		{
			if (descLayoutInfo[i] != rhs.descLayoutInfo[i]) { return false; }
		}
		return true;
	}

	/// <summary>Get descriptor binding</summary>
	/// <param name="bindingId">Binding Index</param>
	/// <returns>The binding layout object (DescriptorBindingLayout)</returns>
	const DescriptorSetLayoutBinding* getBinding(uint16_t bindingId)const
	{
		auto it = std::find_if(descLayoutInfo.begin(),
		                       descLayoutInfo.end(), [&](const DescriptorSetLayoutBinding & info)
		{
			return info.binding == bindingId;
		});
		if (it != descLayoutInfo.end())
		{
			return &(*it);
		}
		return nullptr;
	}

	/// <summary>Get all layout bindings</summary>
	/// <returns>const DescriptorSetLayoutBinding*</returns>
	const DescriptorSetLayoutBinding* getAllBindings()const
	{
		return descLayoutInfo.data();
	}
private:
	friend class ::pvrvk::impl::DescriptorSetLayout_;
	friend struct ::pvrvk::WriteDescriptorSet;
};

namespace impl {
/// <summary>Constructor. . Vulkan implementation of a DescriptorSet.</summary>
class DescriptorSetLayout_
{
public:
	DECLARE_NO_COPY_SEMANTICS(DescriptorSetLayout_)

	/// <summary>Get vulkan handle</summary>
	/// <returns>const VkDescriptorSetLayout&</returns>
	const VkDescriptorSetLayout& getNativeObject()const { return _vkDescsetLayout; }

	/// <summary>Get the DescriptorSetCreateInfo object that was used to create this layout.</summary>
	/// <returns>The DescriptorSetCreateInfo object that was used to create this layout.</returns>
	const DescriptorSetLayoutCreateInfo& getCreateInfo()const { return _createInfo; }

	/// <summary>Clear the descriptor set layout create param list.</summary>
	void clearCreateInfo() { _createInfo.clear(); }

	/// <summary>Get the device that this layout belongs to.</summary>
	/// <returns>The device that this layout belongs to.</returns>
	DeviceWeakPtr& getDevice() { return _device; }

	/// <summary>Get the device that this layout belongs to. (const)</summary>
	/// <returns>The device that this layout belongs to.</returns>
	const DeviceWeakPtr& getDevice()const { return _device; }

private:
	template<typename> friend struct ::pvrvk::RefCountEntryIntrusive;
	friend class ::pvrvk::impl::Device_;

	~DescriptorSetLayout_() { destroy(); }

	DescriptorSetLayout_(const DeviceWeakPtr& device) :
		_vkDescsetLayout(VK_NULL_HANDLE), _device(device)
	{}

	bool init(const DescriptorSetLayoutCreateInfo& createInfo);

	void destroy();

	DescriptorSetLayoutCreateInfo  _createInfo;
	VkDescriptorSetLayout          _vkDescsetLayout;
	DeviceWeakPtr           _device;
};

/// <summary>Internal class</summary>
template<typename T, uint32_t ArraySize>
class DescriptorStore
{
	//!\cond NO_DOXYGEN
public:
	DescriptorStore()
	{
		_ptr = _tArray;
		_numItems = 0;
	}

	DescriptorStore(const DescriptorStore& descStore)
	{
		_numItems = descStore._numItems;
		_tVec = descStore._tVec;
		for (int i = 0; i < ArraySize; ++i)
		{
			_tArray[i] = descStore._tArray[i];
		}
		if (_tVec.size())
		{
			_ptr = _tVec.data();
		}
		else
		{
			_ptr = _tArray;
		}
	}

	DescriptorStore& operator=(const DescriptorStore& descStore)
	{
		if (this == &descStore) { return *this; }
		_numItems = descStore._numItems;
		_tVec = descStore._tVec;
		for (int i = 0; i < ArraySize; ++i)
		{
			_tArray[i] = descStore._tArray[i];
		}
		if (_tVec.size())
		{
			_ptr = _tVec.size();
		}
		else
		{
			_ptr = _tArray;
		}
	}

	void clear()
	{
		for (uint32_t i = 0; i < _numItems; ++i)
		{
			*(_ptr + i) = T();
		}
		_ptr = _tArray;
		_numItems = 0;
		_tVec.clear();
	}

	void set(uint32_t index, const T& obj)
	{
		if (index >= ArraySize)
		{
			moveToOverFlow();
			_ptr = &_tVec[0];
		}
		else if (index > _tVec.size() && _tVec.size() != 0)
		{
			_tVec.resize(index + 1);
		}
		_numItems += uint32_t(!_ptr[index].isValid());
		_ptr[index] = obj;
		_ptr[index] = obj;
	}

	uint32_t size()const
	{
		return _numItems;
	}

	const T* begin()const { return _ptr; }
	const T* end()const { return _ptr + _numItems; }

	const T& operator[](uint32_t index)const
	{
		return get(index);
	}

	const T& get(uint32_t index)const
	{
		return _ptr[index];
	}


private:
	void moveToOverFlow()
	{
		_tVec.reserve(ArraySize * 2);
		_tVec.assign(_tArray, _tArray + ArraySize);
	}

	T _tArray[ArraySize];
	std::vector<T> _tVec;
	T* _ptr;
	uint32_t _numItems;
	//!\endcond
};
}

/// <summary>Descriptor Pool create parameter.</summary>
struct DescriptorPoolCreateInfo
{
private:
	std::pair<VkDescriptorType, uint16_t> _descriptorTypes[static_cast<uint32_t>(VkDescriptorType::e_RANGE_SIZE)];
	uint16_t _numDescriptorTypes;
	uint16_t _maxSets;
public:

	/// <summary>Constructor</summary>
	DescriptorPoolCreateInfo() : _numDescriptorTypes(0), _maxSets(200) {}

	/// <summary>Add the maximum number of the specified descriptor types that the pool will contain.</summary>
	/// <param name="descType">Descriptor type</param>
	/// <param name="count">Maximum number of descriptors of (type)</param>
	/// <returns>this (allow chaining)</returns>
	DescriptorPoolCreateInfo& addDescriptorInfo(VkDescriptorType descType, uint16_t count)
	{
		_descriptorTypes[_numDescriptorTypes] = std::make_pair(descType, count);
		_numDescriptorTypes++;
		return *this;
	}

	/// <summary>Set the maximum number of descriptor sets.</summary>
	/// <param name="maxSets">The maximum number of descriptor sets</param>
	/// <returns>this (allow chaining)</returns>
	DescriptorPoolCreateInfo& setMaxDescriptorSets(uint16_t maxSets)
	{
		this->_maxSets = maxSets; return *this;
	}

	/// <summary>Get the number of allocations of a descriptor type is supported on this pool (const).</summary>
	/// <param name="descType">DescriptorType</param>
	/// <returns>Number of allocations.</returns>
	uint16_t getNumDescriptorTypes(VkDescriptorType descType) const
	{
		for (uint16_t i = 0; i < _numDescriptorTypes; i++)
		{
			if (_descriptorTypes[i].first == descType)
			{
				return _descriptorTypes[i].second;
			}
		}
		return 0;
	}

	/// <summary>Get maximum sets supported on this pool.</summary>
	/// <returns>uint32_t</returns>
	uint16_t getMaxDescriptorSets() const { return _maxSets; }

	DescriptorPoolCreateInfo& configureBasic(uint16_t combinedImageSamplers = 32, uint16_t inputAttachments = 0, uint16_t staticUbo = 32, uint16_t dynamicUbo = 32u, uint16_t staticSsbo = 0, uint16_t dynamicSsbo = 0)
	{
		if (combinedImageSamplers != 0) { addDescriptorInfo(VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, combinedImageSamplers); }
		if (inputAttachments != 0) { addDescriptorInfo(VkDescriptorType::e_INPUT_ATTACHMENT, staticUbo); }
		if (staticUbo != 0) { addDescriptorInfo(VkDescriptorType::e_UNIFORM_BUFFER, staticUbo); }
		if (dynamicUbo != 0) { addDescriptorInfo(VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, dynamicUbo); }
		if (staticSsbo != 0) { addDescriptorInfo(VkDescriptorType::e_STORAGE_BUFFER, staticSsbo); }
		if (dynamicSsbo != 0) { addDescriptorInfo(VkDescriptorType::e_STORAGE_BUFFER_DYNAMIC, dynamicSsbo); }
		return *this;
	}

};

/// <summary>This class contains all the information necessary to populate a Descriptor Set with the actual API
/// objects. Use with the method update of the DescriptorSet. Populate this object with actual Descriptor objects
/// (UBOs, textures etc).</summary>
struct DescriptorImageInfo
{
	Sampler sampler;//!< Sampler handle, and is used in descriptor updates for types VkDescriptorType::e_SAMPLER and VkDescriptorType::e_COMBINED_IMAGE_SAMPLER if the binding being updated does not use immutable samplers
	ImageView imageView;//!< Image view handle, and is used in descriptor updates for types VkDescriptorType::e_SAMPLED_IMAGE, VkDescriptorType::e_STORAGE_IMAGE, VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, and VkDescriptorType::e_INPUT_ATTACHMENT
	VkImageLayout imageLayout;//!< Layout that the image subresources accessible from imageView will be in at the time this descriptor is accessed. imageLayout is used in descriptor updates for types VkDescriptorType::e_SAMPLED_IMAGE, VkDescriptorType::e_STORAGE_IMAGE, VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, and VkDescriptorType::e_INPUT_ATTACHMENT
	/// <summary>Constructor. Initalizes to VkImageLayout::e_UNDEFINED</summary>
	DescriptorImageInfo() : imageLayout(VkImageLayout::e_UNDEFINED) {}

	/// <summary>Constructor from a sampler object.</summary>
	/// <param name="sampler">Sampler handle, and is used in descriptor updates for types
	/// VkDescriptorType::e_SAMPLER and VkDescriptorType::e_COMBINED_IMAGE_SAMPLER if the binding being
	/// updated does not use immutable samplers</param>
	DescriptorImageInfo(const Sampler& sampler) : sampler(sampler) {}

	/// <summary>Constructor from all elements</summary>
	/// <param name="imageView">Image view handle, and is used in descriptor updates for types
	/// VkDescriptorType::e_SAMPLED_IMAGE, VkDescriptorType::e_STORAGE_IMAGE,
	/// VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, and VkDescriptorType::e_INPUT_ATTACHMENT</param>
	/// <param name="sampler">Sampler handle, and is used in descriptor updates for types
	/// VkDescriptorType::e_SAMPLER and VkDescriptorType::e_COMBINED_IMAGE_SAMPLER if the binding
	/// being updated does not use immutable samplers</param>
	/// <param name="imageLayout">Layout that the image subresources accessible from imageView
	/// will be in at the time this descriptor is accessed. imageLayout is used in descriptor
	/// updates for types VkDescriptorType::e_SAMPLED_IMAGE, VkDescriptorType::e_STORAGE_IMAGE,
	/// VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, and VkDescriptorType::e_INPUT_ATTACHMENT</param>
	DescriptorImageInfo(const ImageView& imageView, const Sampler& sampler,
	                    VkImageLayout imageLayout = VkImageLayout::e_GENERAL)
		: sampler(sampler), imageView(imageView), imageLayout(imageLayout) {}

	/// <summary>Constructor from all elements except sampler</summary>
	/// <param name="imageView">Image view handle, and is used in descriptor updates for types
	/// VkDescriptorType::e_SAMPLED_IMAGE, VkDescriptorType::e_STORAGE_IMAGE,
	/// VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, and VkDescriptorType::e_INPUT_ATTACHMENT</param>
	/// <param name="imageLayout">Layout that the image subresources accessible from imageView
	/// will be in at the time this descriptor is accessed. imageLayout is used in descriptor
	/// updates for types VkDescriptorType::e_SAMPLED_IMAGE, VkDescriptorType::e_STORAGE_IMAGE,
	/// VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, and VkDescriptorType::e_INPUT_ATTACHMENT</param>
	DescriptorImageInfo(const ImageView& imageView,
	                    VkImageLayout imageLayout = VkImageLayout::e_GENERAL)
		: imageView(imageView), imageLayout(imageLayout) {}
};
/// <summary>A struct describing a descriptor buffer binding</summary>
struct DescriptorBufferInfo
{
	Buffer        buffer; //!< The buffer object
	VkDeviceSize  offset; //!< The offset into the buffer
	VkDeviceSize  range; //!< The range of the buffer
	/// <summary>Constructor. Zero initialize.</summary>
	DescriptorBufferInfo() : offset(0), range(0) {}
	/// <summary>Constructor. Individual elements.</summary>
	/// <param name="buffer">The referenced buffer</param>
	/// <param name="offset">An offset into the buffer, if the buffer is piecemally bound</param>
	/// <param name="range">A range of the buffer, if the buffer is piecemally bound</param>
	DescriptorBufferInfo(const Buffer& buffer, VkDeviceSize  offset, VkDeviceSize  range)
		:  buffer(buffer), offset(offset), range(range) {}
};

/// <summary>Contains information for an initialization/update of a descriptor set. This class
/// contains both the update AND the descriptor set to be updated, so that multiple descriptor
/// sets can be updated in a single call to Device::updateDescriptorSets</summary>
struct WriteDescriptorSet
{
	/// <summary>Constructor. No operations defined.</summary>
	WriteDescriptorSet() { }

	/// <summary>Constructor. Initializes with a specified descriptor into a set</summary>
	/// <param name="descType">The descriptor type of this write</param>
	/// <param name="descSet">The descriptor set which to update</param>
	/// <param name="dstBinding">The binding to update</param>
	/// <param name="descType">The descriptor type of this write</param>
	/// <param name="dstArrayElement">If the destination is an array, the array index to update</param>
	WriteDescriptorSet(VkDescriptorType descType, DescriptorSet descSet,
	                   uint32_t dstBinding = 0, uint32_t dstArrayElement = 0) : _descType(descType),
		_descSet(descSet), _dstBinding(dstBinding), _dstArrayElement(dstArrayElement)
	{
		set(descType, descSet, dstBinding, dstArrayElement);
	}

	/// <summary>Set the descriptor type</summary>
	/// <param name="descType">The descriptor type</param>
	/// <returns>This object (allow chaining)</returns>
	WriteDescriptorSet& setDescriptorType(VkDescriptorType descType)
	{
		_descType = descType;
		if ((_descType >= VkDescriptorType::e_SAMPLER && _descType <= VkDescriptorType::e_STORAGE_IMAGE) ||
		    _descType == VkDescriptorType::e_INPUT_ATTACHMENT)
		{
			_infoType = InfoType::ImageInfo;
		}
		else if (_descType >= VkDescriptorType::e_UNIFORM_BUFFER && _descType <= VkDescriptorType::e_STORAGE_BUFFER_DYNAMIC)
		{
			_infoType = InfoType::BufferInfo;
		}
		else if (_descType == VkDescriptorType::e_UNIFORM_TEXEL_BUFFER || _descType == VkDescriptorType::e_STORAGE_TEXEL_BUFFER)
		{
			_infoType = InfoType::TexelBufferView;
		}
		else
		{
			debug_assertion(false, "Cannot resolve Info type from descriptor type");
		}
		return *this;
	}

	/// <summary>Set the descriptor set</summary>
	/// <param name="descriptorSet">The descriptor set that will be updated</param>
	/// <returns>This object (allow chaining)</returns>
	WriteDescriptorSet& setDescriptorSet(DescriptorSet&  descriptorSet)
	{
		_descSet = descriptorSet; return *this;
	}

	/// <summary>Set the destination binding</summary>
	/// <param name="binding">The binding into the descriptor set</param>
	/// <returns>This object (allow chaining)</returns>
	WriteDescriptorSet& setDestBinding(uint32_t binding)
	{
		_dstBinding = binding; return *this;
	}

	/// <summary>Set destination array element</summary>
	/// <param name="arrayElement">Array element.</param>
	/// <returns>This object (allow chaining)</returns>
	WriteDescriptorSet& setDestArrayElement(uint32_t arrayElement)
	{
		_dstArrayElement = arrayElement; return *this;
	}

	/// <summary>Sets all the data of this write</summary>
	/// <param name="newDescType">The new descriptor type</param>
	/// <param name="descSet">The new descriptor set</param>
	/// <param name="dstBinding">The new destination Binding in spir-v</param>
	/// <param name="dstArrayElement">If the target descriptor is an array,
	/// the index to update.</param>
	/// <returns>This object (allow chaining)</returns>
	WriteDescriptorSet& set(VkDescriptorType newDescType, const DescriptorSet& descSet,
	                        uint32_t dstBinding = 0, uint32_t dstArrayElement = 0)
	{
		setDescriptorType(newDescType);
		_descSet = descSet;
		_dstBinding = dstBinding;
		_dstArrayElement = dstArrayElement;
		_infos.clear();
		return *this;
	}

	/// <summary>If the target descriptor is an image, set the image info image info</summary>
	/// <param name="arrayIndex">The target array index</param>
	/// <param name="imageInfo">The image info to set</param>
	/// <returns>This object (allow chaining)</returns>
	WriteDescriptorSet& setImageInfo(uint32_t arrayIndex, const DescriptorImageInfo& imageInfo)
	{
		// VALIDATE DESCRIPTOR TYPE
		assertion(((_descType >= VkDescriptorType::e_SAMPLER) &&
		           (_descType <= VkDescriptorType::e_STORAGE_IMAGE)) ||
		          (_descType == VkDescriptorType::e_INPUT_ATTACHMENT));
		if (_descType == VkDescriptorType::e_COMBINED_IMAGE_SAMPLER)
		{
			debug_assertion(imageInfo.sampler.isValid() && imageInfo.imageView.isValid(),
			                "Sampler and ImageView must be valid");
		}
		Infos info;
		info.imageInfo = imageInfo;
		_infos.set(arrayIndex, info);
		return *this;
	}

	/// <summary>If the target descriptor is a buffer, sets the buffer info</summary>
	/// <param name="arrayIndex">If an array, the target array index</param>
	/// <param name="bufferInfo">The buffer info to set</param>
	/// <returns>This object(allow chaining)</returns>
	WriteDescriptorSet& setBufferInfo(uint32_t arrayIndex, const DescriptorBufferInfo& bufferInfo)
	{
		assertion(_descType >= VkDescriptorType::e_UNIFORM_BUFFER && _descType <= VkDescriptorType::e_STORAGE_BUFFER_DYNAMIC);
		debug_assertion(bufferInfo.buffer.isValid(), "Buffer must be valid");
		Infos info;
		info.bufferInfo = bufferInfo;
		_infos.set(arrayIndex, info);
		return *this;
	}

	/// <summary>If the target descriptor is a Texel buffer, set the Texel Buffer info</summary>
	/// <param name="arrayIndex">If an array, the target array index</param>
	/// <param name="bufferView">The Texel Buffer view to set</param>
	/// <returns>This object(allow chaining)</returns>
	WriteDescriptorSet& setTexelBufferInfo(uint32_t arrayIndex, const BufferView& bufferView)
	{
		assertion(_descType >= VkDescriptorType::e_UNIFORM_TEXEL_BUFFER && _descType <= VkDescriptorType::e_STORAGE_TEXEL_BUFFER);
		debug_assertion(bufferView.isValid(), "Texel BufferView must be valid");
		Infos info;
		info.texelBuffer = bufferView;
		_infos.set(arrayIndex, info);
		return *this;
	}

	/// <summary>Clear all info of this object</summary>
	/// <returns>This object(allow chaining)</returns>
	WriteDescriptorSet& clearAllInfos()
	{
		_infos.clear(); return *this;
	}

	/// <summary>Get the number of descriptors being updated</summary>
	/// <returns>The the number of descriptors</returns>
	uint32_t getNumDescriptors()const
	{
		return _infos.size();
	}

	/// <summary>Get descriptor type</summary>
	/// <returns>The descriptor type</returns>
	VkDescriptorType getDescriptorType()const
	{
		return _descType;
	}

	/// <summary>Get descriptor set to update</summary>
	/// <returns>The descriptor set</returns>
	DescriptorSet getDescriptorSet()
	{
		return _descSet;
	}

	/// <summary>Get the descriptor set to update</summary>
	/// <returns>The descriptor set</returns>
	const DescriptorSet& getDescriptorSet()const
	{
		return _descSet;
	}

	/// <summary>If an array, get the destination array element</summary>
	/// <returns>The destination array element</returns>
	uint32_t getDestArrayElement()const
	{
		return _dstArrayElement;
	}

	/// <summary>Get the destination binding indiex</summary>
	/// <returns>The destination binding index</returns>
	uint32_t getDestBinding()const
	{
		return _dstBinding;
	}
private:
	template<typename> friend struct ::pvrvk::RefCountEntryIntrusive;
	friend class ::pvrvk::impl::Device_;

	VkDescriptorType _descType;
	DescriptorSet _descSet;
	uint32_t _dstBinding;
	uint32_t _dstArrayElement;
	struct Infos
	{
		DescriptorImageInfo imageInfo;
		DescriptorBufferInfo bufferInfo;
		BufferView texelBuffer;
		uint32_t accumulationImageGlobalBinding;

		Infos() : accumulationImageGlobalBinding(uint32_t(-1)) {}
		bool isValid()const
		{
			return imageInfo.imageView.isValid() ||
			       imageInfo.sampler.isValid() ||
			       bufferInfo.buffer.isValid() ||
			       texelBuffer.isValid();
		}
	};

	impl::DescriptorStore<Infos, 16> _infos;

	enum InfoType
	{
		ImageInfo, BufferInfo, TexelBufferView, RayExecutables
	};
	InfoType _infoType;

	// CALL THIS ONE FROM THE DEVICE - CPU SIDE KEEPING ALIVE OF THE DESCRIPTORS IN THIS SET
	void updateKeepAliveIntoDestinationDescriptorSet() const;
};

/// <summary>Copy descriptor set</summary>
struct CopyDescriptorSet
{
	DescriptorSet      srcSet;//!< Source descriptor set to copy from
	uint32_t           srcBinding;//!< source binding to copy
	uint32_t           srcArrayElement;//!< source array element to copy from
	DescriptorSet      dstSet;//!< Destination descriptor set
	uint32_t           dstBinding;//!< Destination binding to copy in to.
	uint32_t           dstArrayElement;//!< Destination array element to copy into
	uint32_t           descriptorCount;//!< Number of descriptor to copy
};

namespace impl {
/// <summary>Vulkan implementation of a DescriptorSet.</summary>
class DescriptorSet_
{
public:
	typedef uint16_t IndexType; //!<The datatype used for Indexes into descriptor sets.
	DECLARE_NO_COPY_SEMANTICS(DescriptorSet_)

	/// <summary>Return the layout of this DescriptorSet.</summary>
	/// <returns>This DescriptorSet's DescriptorSetLayout</returns>
	const DescriptorSetLayout& getDescriptorSetLayout()const {return _descSetLayout;}

	/// <summary>Return the descriptor pool from which this descriptor set was allocated</summary>
	/// <returns>The descriptor pool</returns>
	const DescriptorPool& getDescriptorPool()const { return _descPool; }

	/// <summary>Return the descriptor pool from which this descriptor set was allocated</summary>
	/// <returns>The descriptor pool</returns>
	DescriptorPool& getDescriptorPool() { return _descPool; }

	/// <summary>Return the Vulkan raw object (VkDescriptorSet)</summary>
	/// <returns>The raw vulkan object</returns>
	const VkDescriptorSet& getNativeObject()const { return _vkDescriptorSet; }
private:
	friend struct ::pvrvk::WriteDescriptorSet;
	friend class ::pvrvk::impl::DescriptorPool_;
	template<typename> friend struct ::pvrvk::RefCountEntryIntrusive;
	friend class ::pvrvk::impl::Device_;

	~DescriptorSet_() { destroy(); }

	bool init();
	void destroy();

	DescriptorSet_(const DescriptorSetLayout& descSetLayout, const DescriptorPool& pool) :
		_descSetLayout(descSetLayout), _descPool(pool), _vkDescriptorSet(VK_NULL_HANDLE) {}

	mutable std::vector<std::vector<RefCountedResource<void>/**/>/**/> _keepAlive;
	DescriptorSetLayout _descSetLayout;
	DescriptorPool      _descPool;
	VkDescriptorSet     _vkDescriptorSet;
};

/// <summary>A descriptor pool - an object used to allocate (and recycle) Descriptor Sets.</summary>
class DescriptorPool_ : public EmbeddedRefCount<DescriptorPool_>
{
public:

	/// <summary>Allocate descriptor set</summary>
	/// <param name="layout">Descriptor set layout</param>
	/// <returns>Return DescriptorSet else null if fails.</returns>
	DescriptorSet allocateDescriptorSet(const DescriptorSetLayout& layout);

	/// <summary>Get vulkan handle(const)</summary>
	/// <returns>const VkDescriptorPool&</returns>
	const VkDescriptorPool& getNativeObject()const { return _vkDescPool; }

	/// <summary>Get Device </summary>
	/// <returns>DeviceWeakPtr</returns>
	DeviceWeakPtr getDevice()
	{
		return _device;
	}

	/// <summary>Get Device  </summary>(const)
	/// <returns>DeviceWeakPtr</returns>
	const DeviceWeakPtr& getDevice()const
	{
		return _device;
	}
private:
	DECLARE_NO_COPY_SEMANTICS(DescriptorPool_)
	// Implementing EmbeddedRefCount
	template <typename> friend class ::pvrvk::EmbeddedRefCount;
	friend class ::pvrvk::impl::Device_;

	bool init(const DescriptorPoolCreateInfo& createInfo);
	void destroy();

	~DescriptorPool_()
	{
		destroy();
	}
	static DescriptorPool createNew(const DeviceWeakPtr& device)
	{
		return EmbeddedRefCount<DescriptorPool_>::createNew(device);
	}
	DescriptorPool_(const DeviceWeakPtr& device) : _device(device), _vkDescPool(VK_NULL_HANDLE) {}
	DeviceWeakPtr _device;
	VkDescriptorPool _vkDescPool;
	/* IMPLEMENTING EmbeddedResource */
	void destroyObject() { destroy(); }
};

inline bool DescriptorSet_::init()
{
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VkStructureType::e_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.pSetLayouts = &getDescriptorSetLayout()->getNativeObject();
	allocInfo.descriptorSetCount = 1;
	allocInfo.descriptorPool = getDescriptorPool()->getNativeObject();
	const auto& it = _descSetLayout->getCreateInfo().getAllBindings();
	uint16_t maxbinding = 0;
	uint32_t i = 0, size = _descSetLayout->getCreateInfo().getNumBindings();
	for (; i < size; ++i)
	{
		maxbinding = std::max(it[i].binding, maxbinding);
	}
	_keepAlive.resize(maxbinding + 1);
	for (i = 0; i < size; ++i)
	{
		auto& entry = it[i];
		auto& aliveentry = _keepAlive[entry.binding];
		aliveentry.resize(entry.descriptorCount);
	}
	return (vk::AllocateDescriptorSets(_descSetLayout->getDevice()->getNativeObject(),
	                                   &allocInfo, &_vkDescriptorSet) == VkResult::e_SUCCESS);
}
}

inline void WriteDescriptorSet::updateKeepAliveIntoDestinationDescriptorSet() const
{
	auto& keepalive = getDescriptorSet()->_keepAlive[this->_dstBinding];
	if (_infoType == InfoType::BufferInfo)
	{
		for (uint32_t i = 0; i < _infos.size(); ++i)
		{
			keepalive[i] = _infos[i].bufferInfo.buffer;
		}
	}
	else if (_infoType == InfoType::ImageInfo)
	{
		for (uint32_t i = 0; i < _infos.size(); ++i)
		{
			auto newpair = RefCountedResource<std::pair<Sampler, ImageView>/**/>();
			newpair.construct();
			newpair->first = _infos[i].imageInfo.sampler;
			newpair->second = _infos[i].imageInfo.imageView;
			keepalive[i] = newpair;
		}
	}
	else if (_infoType == InfoType::TexelBufferView)
	{
		for (uint32_t i = 0; i < _infos.size(); ++i)
		{
			keepalive[i] = _infos[i].texelBuffer;
		}
	}
}
}// namespace pvrvk
