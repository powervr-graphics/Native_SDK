/*!
\brief INTERNAL TO RenderManager. Implementation of the EffectApi class.
\file PVREngineUtils/EffectApi.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVREngineUtils/EffectApi_2.h"
#include "PVRApi/ApiObjects/Fbo.h"
#include "PVRCore/IO/BufferStream.h"

namespace pvr {
namespace utils {
namespace effect {
namespace impl {

namespace {
std::map<Api, std::set<StringHash>/**/> apiToStringList;
inline void addMapping(Api api, const StringHash& str)
{
	apiToStringList[api].insert(str);
}

inline bool initializeStringLists()
{
	apiToStringList.clear();
	addMapping(Api::Vulkan, "Vulkan");
	addMapping(Api::Vulkan, "vulkan");
	addMapping(Api::Vulkan, "VK");
	addMapping(Api::Vulkan, "vk");
	addMapping(Api::Vulkan, "VULKAN");
	addMapping(Api::OpenGLES2, "OpenGLES2");
	addMapping(Api::OpenGLES2, "OGLES2");
	addMapping(Api::OpenGLES2, "GLES2");
	addMapping(Api::OpenGLES2, "GL2");
	addMapping(Api::OpenGLES3, "OpenGLES3");
	addMapping(Api::OpenGLES3, "OGLES3");
	addMapping(Api::OpenGLES3, "GLES3");
	addMapping(Api::OpenGLES3, "GL3");
	addMapping(Api::OpenGLES31, "OpenGLES31");
	addMapping(Api::OpenGLES31, "OGLES31");
	addMapping(Api::OpenGLES31, "GLES31");
	addMapping(Api::OpenGLES31, "GL31");
	return true;
}

inline StringHash findMatchingApiString(const Effect_::AssetEffect& asset, Api api)
{
	StringHash retval("");
	const auto& versions = asset.getVersions();

	auto apiStringsIt = apiToStringList.find(api);
	if (apiStringsIt == apiToStringList.end())
	{
		Log(Log.Warning, "EffectApi: Could not find any matching string in the Effect asset ('apiversion' elements), so will use the default (empty string) implementation.\n"
		    "If the effect has not been designed to work with OpenGL ES implementations in the default settings, errors will occur.\n"
		    "Default strings that OpenGL ES implementations accept are :\n"
		    "[nothing], 'OpenGLES2', 'OGLES2', 'GLES2', 'GL2','OpenGLES3', 'OGLES3', 'GLES3', 'GL3','OpenGLES31', 'OGLES31', 'GLES31', 'GL31'"
		    "Default strings that Vulkan implementations accept are :\n"
		    "[nothing], 'VK', 'vk', 'VULKAN', 'Vulkan', 'vulkan'"
		    "If providing an apiversion other than this, use the function pvr::utils::effect::addApiversionStringMapping from application code"
		    "to add it to your implementation."
		   );
		return retval;
	}

	Api invalidVersion = api > Api::OpenGLESMaxVersion ? Api::OpenGLESMaxVersion : Api::Unspecified;
	while (api > invalidVersion)
	{
		apiStringsIt = apiToStringList.find(api);
		for (auto versionsit = versions.begin(); versionsit != versions.end(); ++versionsit)
		{
			for (auto apiStrings = apiStringsIt->second.begin(); apiStrings != apiStringsIt->second.end(); ++apiStrings)
			{
				if (*versionsit == *apiStrings)
				{
					retval = *apiStrings;
					return retval;
				}
			}
		}
		api = Api((int)api - 1);
	}
	return retval;
}

inline bool addTexture(Effect_& effect, AssetLoadingDelegate& loader, const assets::effect::TextureReference& textureRef,
                       std::map<StringHash, api::TextureView>& textures, const StringHash& pipeline)
{
	PipelineDef* pipeDef =  effect.getPipelineDefinition(pipeline);
	if (!pipeDef) { return false; }
	pipeDef->descSetExists[textureRef.set] = true;
	api::TextureView view;
	GraphicsContext ctx = effect.getContext();

	if (!textureRef.textureName.empty())
	{
		const assets::effect::TextureDefinition& textureDef = effect.getEffectAsset().textures.find(textureRef.textureName)->second;

		if (textureDef.path.length())
		{
			loader.effectOnLoadTexture(textureDef.path, view);
		}
		else
		{
			api::TextureStore s = ctx->createTexture();
			ImageStorageFormat fmt;
			fmt = textureDef.fmt;

			s->allocate2D(fmt, textureDef.width, textureDef.height);
			if (s.isValid())
			{
				view = ctx->createTextureView(s);
			}
		}

		if (view.isNull())
		{
			Log("ApiEffect: Failed to create texture with name %s", textureDef.name.c_str());
			return false;
		}
		else
		{
			textures[textureDef.name] = view;
		}
	}
	else if (!textureRef.semantic.empty())
	{
		pipeDef->descSetIsFixed[textureRef.set] = false;
		effect.registerTextureSemantic(pipeline, textureRef.semantic, textureRef.set, textureRef.binding);
	}
	else
	{
		Log("ApiEffect: For pipeline [%s] texture [%s] neither a 'name' nor a 'semantic' attribute was not defined. If this texture is to be loaded or created by the effect,"
		    "define a <texture> element in the pfx file and set the 'name' attribute in the pipeline's <texture> element. If this texture is intended will be provided with"
		    "a model, define the 'semantic' attribute in the ");
		return false;
	}
	return true;
}

inline bool addBuffer(Effect_& effect, effect::PipelineDef& pipedef, const assets::effect::BufferRef& bufferRef, std::map<StringHash, BufferDef>& buffers)
{
	pipedef.descSetExists[bufferRef.set] = true;
	//auto buffer_it = effect.getEffectAsset().buffers.find(bufferRef.name);
	//auto buffer_it = buffers.find(bufferDef.name)
	if (bufferRef.bufferName.length())
	{
		const assets::effect::BufferDefinition& assetBufferDef = effect.getEffectAsset().buffers.find(bufferRef.bufferName)->second;

		BufferDef& bufferdef = buffers[assetBufferDef.name];

		pipedef.descSetIsMultibuffered[bufferRef.set] = assetBufferDef.multibuffering;
		if (bufferdef.scope == types::VariableScope::Unknown) // First time this buffer is referenced.
		{
			assertion(bufferdef.bufferView.getConnectedBuffer(0).isNull());

			bufferdef.allSupportedBindings = assetBufferDef.allSupportedBindings;
			bufferdef.isDynamic = assetBufferDef.isDynamic;
			bufferdef.scope = assetBufferDef.scope;

			if (assetBufferDef.multibuffering)
			{
				bufferdef.numBuffers = (uint16)effect.getContext()->getSwapChainLength();
			}
			bufferdef.bufferView.setMultibufferCount(bufferdef.numBuffers);

			for (size_t i = 0; i < assetBufferDef.entries.size(); ++i)
			{
				bufferdef.bufferView.addEntryPacked(assetBufferDef.entries[i].semantic, assetBufferDef.entries[i].dataType, assetBufferDef.entries[i].arrayElements);
			}

		}

		//Add it to the pipeline's lists
		switch (bufferdef.scope)
		{
		case types::VariableScope::Effect:
		{
			auto& binfo = pipedef.effectScopeBuffers[assetBufferDef.name];
			static_cast<BufferRef&>(binfo) = static_cast<const BufferRef&>(bufferRef);
		}
		break;
		case types::VariableScope::Model:
		case types::VariableScope::BoneBatch:
		{
			pipedef.descSetIsFixed[bufferRef.set] = false;

			auto& binfo = pipedef.modelScopeBuffers[assetBufferDef.name];
			static_cast<BufferRef&>(binfo) = static_cast<const BufferRef&>(bufferRef);
		}
		break;
		case types::VariableScope::Node:
		{
			pipedef.descSetIsFixed[bufferRef.set] = false;
			auto& binfo = pipedef.nodeScopeBuffers[assetBufferDef.name];
			static_cast<BufferRef&>(binfo) = static_cast<const BufferRef&>(bufferRef);
		}
		break;
		}


	}
	else
	{
		Log("ApiEffect: A buffer with name [%s] was not properly defined but was referenced in a pipeline", bufferRef.bufferName.c_str());
		return false;
	}
	return true;
}

inline void createTextures(Effect_& effect, AssetLoadingDelegate& loader, std::map<StringHash, api::TextureView>& textures)
{
	const Effect_::AssetEffect assetEffect = effect.getEffectAsset();
	auto pipes_it = assetEffect.versionedPipelines.find(effect.getApiString());
	if (pipes_it != assetEffect.versionedPipelines.end())
	{
		auto& pipes = pipes_it->second;
		for (auto pipe_it = pipes.begin(); pipe_it != pipes.end(); ++pipe_it)
		{
			for (auto texture_it = pipe_it->second.textures.begin(); texture_it != pipe_it->second.textures.end(); ++texture_it)
			{
				addTexture(effect, loader, *texture_it, textures, pipe_it->first);
			}
		}
	}
}

inline void createBuffers(Effect_& effect, std::map<StringHash, effect::PipelineDef>& createParams, std::map<StringHash, BufferDef>& buffers)
{
	const Effect_::AssetEffect assetEffect = effect.getEffectAsset();
	auto pipes = assetEffect.versionedPipelines.find(effect.getApiString());
	if (pipes != assetEffect.versionedPipelines.end())
	{
		for (auto pipe_it = pipes->second.begin(); pipe_it != pipes->second.end(); ++pipe_it)
		{
			auto pipeDef = effect.getPipelineDefinition(pipe_it->first);
			if (!pipeDef) { continue; }
			for (auto buffer_it = pipe_it->second.buffers.begin(); buffer_it != pipe_it->second.buffers.end(); ++buffer_it)
			{
				addBuffer(effect, *pipeDef, *buffer_it, buffers);
			}
		}
	}
}


inline ImageStorageFormat getAttachmentFormat(const PixelFormat& pixelFmt, bool srgb)
{
	switch (pixelFmt.getPixelTypeId())
	{
	case GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID:
		return (srgb ? ImageStorageFormat(pixelFmt, 1, types::ColorSpace::sRGB) :
		        ImageStorageFormat(PixelFormat::RGBA_8888));

	case GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID:
		return ImageStorageFormat(pixelFmt, 1, types::ColorSpace::lRGB, VariableType::Float);

	case GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID:
		return ImageStorageFormat(pixelFmt, 1);

	case GeneratePixelType3<'r', 'g', 'b', 5, 6, 5>::ID:
		return ImageStorageFormat(pixelFmt, 1);

	case GeneratePixelType1<'r', 32>::ID:
		return ImageStorageFormat(pixelFmt, 1, types::ColorSpace::lRGB, VariableType::SignedInteger);

	case GeneratePixelType1<'d', 16>::ID:
		return ImageStorageFormat(pixelFmt, 1, types::ColorSpace::lRGB, VariableType::Float);

	case GeneratePixelType1<'d', 24>::ID:
		return ImageStorageFormat(pixelFmt, 1, types::ColorSpace::lRGB, VariableType::Float);

	case GeneratePixelType2<'d', 's', 24, 32>::ID:
		return ImageStorageFormat(pixelFmt, 1, types::ColorSpace::lRGB, VariableType::Float);

	case GeneratePixelType1<'d', 32>::ID:
		return ImageStorageFormat(pixelFmt, 1, types::ColorSpace::lRGB, VariableType::Float);

	default:
		return ImageStorageFormat(PixelFormat::RGBA_8888, 1);
	}
}

const assets::effect::PipelineDefinition* getPipeline(const assets::effect::Effect& effect, const StringHash& version, const StringHash& name)
{
	const Effect_::AssetEffect& assetEffect = effect;
	auto pipes = assetEffect.versionedPipelines.find(version);
	if (pipes == assetEffect.versionedPipelines.end())
	{
		pipes = assetEffect.versionedPipelines.find(StringHash());
	}
	if (pipes != assetEffect.versionedPipelines.end())
	{
		auto pipe = pipes->second.find(name);
		if (pipe != pipes->second.end())
		{
			return  &pipe->second;
		}
	}
	return NULL;
}

bool getRenderPassAndFboForPass(Effect_& effect, const assets::effect::Effect& effectAsset,
                                const assets::effect::Pass& pass, api::FboSet& fbos, api::RenderPass& rp,
                                std::vector<std::pair<StringHash, uint32>/**/>& colorAttachmentIndex)
{
	GraphicsContext ctx = effect.getContext();
	// create the onscreen fbo if we have only one subpass and and one target which is default
	if (pass.subpasses.size() == 1 && pass.subpasses[0].targets[0] == "default" &&
	    pass.subpasses[0].targets[1].empty() && pass.subpasses[0].targets[2].empty() &&
	    pass.subpasses[0].targets[3].empty())
	{
		fbos = ctx->createOnScreenFboSet();
		rp = fbos[0]->getRenderPass();
	}
	else
	{
		std::set<assets::effect::TextureDefinition> colorAttachmentsSet;
		std::set<assets::effect::TextureDefinition> inputAttachments;
		bool createOnScreenFbo = false;

		// gather all sub passes targets and input attachments
		// keep a unique list of tarets  and input attachments from the subpass
		for (uint32 i = 0; i < pass.subpasses.size(); ++i)
		{
			const assets::effect::Subpass& subpass = pass.subpasses[i];

			// TARGET ATTACHMENTS
			if (!subpass.targets[0].empty())
			{
				// if an target of any of the subpass is default then we are creating onscreen fbo
				if (subpass.targets[0] == "default")
				{
					createOnScreenFbo = true;
				}
				else
				{
					const auto& found = effectAsset.textures.find(subpass.targets[0]);
					if (found != effectAsset.textures.end())
					{
						colorAttachmentsSet.insert(found->second);
					}
				}
			}
			if (!subpass.targets[1].empty())
			{
				const auto& found = effectAsset.textures.find(subpass.targets[1]);
				if (found != effectAsset.textures.end())
				{
					colorAttachmentsSet.insert(found->second);
				}
			}
			if (!subpass.targets[2].empty())
			{
				const auto& found = effectAsset.textures.find(subpass.targets[2]);
				if (found != effectAsset.textures.end())
				{
					colorAttachmentsSet.insert(found->second);
				}
			}
			if (!subpass.targets[3].empty())
			{
				const auto& found = effectAsset.textures.find(subpass.targets[3]);
				if (found != effectAsset.textures.end())
				{
					colorAttachmentsSet.insert(found->second);
				}
			}

			// INPUT ATTACHMENTS
			if (!subpass.inputs[0].empty())
			{
				const auto& found = effectAsset.textures.find(subpass.inputs[0]);
				if (found != effectAsset.textures.end())
				{
					inputAttachments.insert(found->second);
				}

			}
			if (!subpass.inputs[1].empty())
			{
				const auto& found = effectAsset.textures.find(subpass.inputs[1]);
				if (found != effectAsset.textures.end())
				{
					inputAttachments.insert(found->second);
				}
			}
			if (!subpass.inputs[2].empty())
			{
				const auto& found = effectAsset.textures.find(subpass.inputs[2]);
				if (found != effectAsset.textures.end())
				{
					inputAttachments.insert(found->second);
				}
			}
			if (!subpass.inputs[3].empty())
			{
				const auto& found = effectAsset.textures.find(subpass.inputs[3]);
				if (found != effectAsset.textures.end())
				{
					inputAttachments.insert(found->second);
				}
			}
		}// next subpass

		// assign the unique list of input attachment to the vector for indexing
		// if it is onscreen fbo then make the attachment 0 is the swapchain image
		std::vector<assets::effect::TextureDefinition> colorAttachments(createOnScreenFbo ? colorAttachmentsSet.size() + 1 : colorAttachmentsSet.size());
		if (createOnScreenFbo)
		{
			colorAttachments[0] = assets::effect::TextureDefinition("default", "", ctx->getDisplayAttributes().width,
			                      ctx->getDisplayAttributes().height, ctx->getPresentationImageFormat());
		}
		std::copy(colorAttachmentsSet.begin(), colorAttachmentsSet.end(), createOnScreenFbo ? colorAttachments.begin() + 1 : colorAttachments.begin());
		colorAttachmentsSet.clear();

		Multi<api::OnScreenFboCreateParam> onScreenFboInfo;
		Multi<api::FboCreateParam> fboCreateInfo;
		api::RenderPassCreateParam rpCreateInfo;
		const uint32 fboWidth = ctx->getDisplayAttributes().width;
		const uint32 fboHeight =  ctx->getDisplayAttributes().height;

		for (uint32 i = 0; i < colorAttachments.size(); ++i)
		{
			const assets::effect::TextureDefinition& texDef = colorAttachments[i];
			// make sure the attachments image's width and height are consistent
			if (fboWidth != texDef.width)
			{
				Log(Log.Warning, "Framebuffer attachment %s width is inconsistent with other attachments. Forcing to %d", texDef.name.c_str(), fboWidth);
			}
			if (fboHeight != texDef.height)
			{
				Log(Log.Warning, "Framebuffer attachment %s height is inconsistent with other attachments. Forcing to %d", texDef.name.c_str(), fboHeight);
			}

			for (uint32 swapChain = 0; swapChain < ctx->getSwapChainLength(); ++swapChain)
			{
				api::TextureStore texture = ctx->createTexture();

				// Create on Screen fbo attachment
				// attachment index 0 is reserved for the swapchain image
				if (createOnScreenFbo)
				{
					// if it is the first attachment then it has to tbe swapchain image.
					// else f it is not the first attachment then  create a transient attachment if the
					// subpasses use them as input and targets. Transient attachments are lazly allocated on the
					// device and the memory is optimized out on some driver.
					if (i == 0)
					{
						rpCreateInfo.setColorInfo(0, api::RenderPassColorInfo(ctx->getPresentationImageFormat(), types::LoadOp::Clear));
					}
					else
					{
						if (inputAttachments.find(texDef) != inputAttachments.end())
						{
							texture->allocateTransient(texDef.fmt, fboWidth, fboHeight);
						}
						else
						{
							texture->allocate2D(texDef.fmt, fboWidth, fboHeight, types::ImageUsageFlags::ColorAttachment,
							                    types::ImageLayout::ColorAttachmentOptimal);
						}
						onScreenFboInfo[swapChain].addOffScreenColor(ctx->createTextureView(texture));
						// if it is onscreen fbo then the index 0 is reserved for the swapchain
						// TODO LOAD OP SHOULD NOT BE HARDCODED.
						rpCreateInfo.setColorInfo(i, api::RenderPassColorInfo(texture->getFormat(), types::LoadOp::Clear));

						colorAttachmentIndex.push_back(std::make_pair(texDef.name, i));
					}

				}
				else
				{
					// create a transient attachment if the
					// subpasses use them as input and targets. Transient attachments are lazly allocated on the
					// device and the memory is optimized out on some driver.
					if (inputAttachments.find(texDef) != inputAttachments.end())
					{
						texture->allocateTransient(texDef.fmt, fboWidth, fboHeight);
					}
					else
					{
						texture->allocate2D(texDef.fmt, fboWidth, fboHeight, types::ImageUsageFlags::ColorAttachment,
						                    types::ImageLayout::ColorAttachmentOptimal);
					}
					fboCreateInfo[swapChain].setColor(i, ctx->createTextureView(texture));
					rpCreateInfo.setColorInfo(i, api::RenderPassColorInfo(texture->getFormat(), types::LoadOp::Clear));
					colorAttachmentIndex.push_back(std::make_pair(texDef.name, i));
				}
			}
		}// next color attachment

		//------------------------------------
		// Depth stencil attachment
		// create offscreen depthstencil attachment if the target depthstencil is not default and not empty
		// else use the swapchain's depth stencil attachment
		api::TextureView dsAttachments[(uint32)FrameworkCaps::MaxSwapChains];
		if (pass.targetDepthStencil != "default" && !pass.targetDepthStencil.empty()) // depth stencil
		{
			const auto& found =  effectAsset.textures.find(pass.targetDepthStencil);
			if (found != effectAsset.textures.end())
			{
				if (fboWidth != found->second.width)
				{
					Log(Log.Warning, "Framebuffer attachment %s width is inconsistent with other attachments. "
					    "Forcing to %d", found->first.c_str(), fboWidth);
				}
				if (fboHeight != found->second.height)
				{
					Log(Log.Warning, "Framebuffer attachment %s height is inconsistent with other attachments. "
					    "Forcing to %d", found->first.c_str(), fboHeight);
				}
				for (uint32 ii = 0; ii < ctx->getSwapChainLength(); ++ii)
				{
					api::TextureStore tex = ctx->createTexture();
					tex->allocate2D(ImageStorageFormat(found->second.fmt), fboWidth, fboHeight,
					                types::ImageUsageFlags::DepthStencilAttachment, types::ImageLayout::DepthStencilAttachmentOptimal);
					dsAttachments[ii] = ctx->createTextureView(tex);
				}
				rpCreateInfo.setDepthStencilInfo(0, api::RenderPassDepthStencilInfo(ImageStorageFormat(found->second.fmt), types::LoadOp::Clear));
			}
			else
			{
				Log("EffectApi: Depth-Stencil attachment referenced in pass %s is not found");
			}
		}
		else
		{
			rpCreateInfo.setDepthStencilInfo(0, api::RenderPassDepthStencilInfo(ctx->getDepthStencilImageFormat(), types::LoadOp::Clear));
		}

		//-----------------------
		// create the subpass
		for (uint32 i = 0; i < pass.subpasses.size(); ++i)
		{
			api::SubPass subpass;
			uint8 attachmentId = 0, preserveAttachmentIndex = 0;

			// for each color attachments:
			// add the attachment as a color / input attachment if the sub pass uses them.
			for (auto it = colorAttachments.cbegin(); it != colorAttachments.cend(); ++it, ++attachmentId)
			{
				if (pass.subpasses[i].targets[0] == it->name)
				{
					subpass.setColorAttachment(0, attachmentId);
				}
				else if (pass.subpasses[i].targets[1] == it->name)
				{
					subpass.setColorAttachment(1, attachmentId);
				}
				else if (pass.subpasses[i].targets[2] == it->name)
				{
					subpass.setColorAttachment(2, attachmentId);
				}
				else if (pass.subpasses[i].targets[3] == it->name)
				{
					subpass.setColorAttachment(3, attachmentId);
				}
				else if (pass.subpasses[i].inputs[0] == it->name)
				{
					subpass.setInputAttachment(0, attachmentId);
					subpass.setPreserveAttachment(preserveAttachmentIndex, attachmentId);
					++preserveAttachmentIndex;
				}
				else if (pass.subpasses[i].inputs[1] == it->name)
				{
					subpass.setInputAttachment(1, attachmentId);
					subpass.setPreserveAttachment(preserveAttachmentIndex, attachmentId);
					++preserveAttachmentIndex;
				}
				else if (pass.subpasses[i].inputs[2] == it->name)
				{
					subpass.setInputAttachment(2, attachmentId);
					subpass.setPreserveAttachment(preserveAttachmentIndex, attachmentId);
					++preserveAttachmentIndex;
				}
				else if (pass.subpasses[i].inputs[3] == it->name)
				{
					subpass.setInputAttachment(3, attachmentId);
					subpass.setPreserveAttachment(preserveAttachmentIndex, attachmentId);
					++preserveAttachmentIndex;
				}
				else
				{
					subpass.setPreserveAttachment(preserveAttachmentIndex, attachmentId);
					++preserveAttachmentIndex;
				}
			}

			subpass.setDepthStencilAttachment(0);
			subpass.enableDepthStencilAttachment(pass.subpasses[i].useDepthStencil);

			// TODO  support resolve attachments

			rpCreateInfo.setSubPass(i, subpass);
		}// next subpass

		//-----------------------------------------
		// Subpass dependency
		// loop through all subppasses and create a dependecy chain between them.
		for (uint32 i = 0; i < rpCreateInfo.getNumSubPass(); ++i)
		{
			const api::SubPass& subpassDst = rpCreateInfo.getSubPass(i);
			for (uint32 j = 0; j < rpCreateInfo.getNumSubPass(); ++j)
			{
				// avoid if src and dest subpasses are same and the src sub pass index must be less than the
				// destination sub pass index
				if (j >= i) { continue; }
				const api::SubPass& subpassSrc = rpCreateInfo.getSubPass(j);

				auto  srcAccessFlag = types::AccessFlags(0);
				auto  dstAccessFlag = types::AccessFlags(0);

				// COLOR
				if (subpassSrc.getNumColorAttachment() != 0 && subpassDst.getNumInputAttachment() != 0)
				{
					srcAccessFlag |= types::AccessFlags::ColorAttachmentWrite;
					dstAccessFlag |= types::AccessFlags::ColorAttachmentRead;
				}
				else if (subpassSrc.getNumColorAttachment() != 0 && subpassDst.getNumColorAttachment() != 0)
				{
					srcAccessFlag |= types::AccessFlags::ColorAttachmentWrite;
					dstAccessFlag |= types::AccessFlags::ColorAttachmentWrite;
				}
				else if (subpassSrc.getNumInputAttachment() != 0 && subpassDst.getNumInputAttachment() != 0)
				{
					srcAccessFlag |= types::AccessFlags::ColorAttachmentRead;
					dstAccessFlag |= types::AccessFlags::ColorAttachmentRead;
				}

				// DEPTH STENCIL
				if (subpassSrc.usesDepthStencilAttachment() && subpassDst.usesDepthStencilAttachment())
				{
					srcAccessFlag |= types::AccessFlags::DepthStencilAttachmentWrite;
					dstAccessFlag |= types::AccessFlags::DepthStencilAttachmentWrite;
				}
				else if (subpassSrc.usesDepthStencilAttachment())
				{
					srcAccessFlag |= types::AccessFlags::DepthStencilAttachmentWrite;
					dstAccessFlag |= types::AccessFlags::DepthStencilAttachmentRead;
				}
				else if (subpassDst.usesDepthStencilAttachment())
				{
					srcAccessFlag |= types::AccessFlags::DepthStencilAttachmentRead;
					dstAccessFlag |= types::AccessFlags::DepthStencilAttachmentWrite;
				}

				rpCreateInfo.addSubPassDependency(api::SubPassDependency(j, i, types::PipelineStageFlags::AllGraphics,
				                                  types::PipelineStageFlags::AllGraphics, srcAccessFlag, dstAccessFlag, true));
			}
		}

		rp = ctx->createRenderPass(rpCreateInfo);
		if (!rp.isValid())
		{
			Log("EffectApi: Failed to create a renderpass for the pass: %s", pass.name.c_str());
			return false;
		}
		if (createOnScreenFbo)
		{
			fbos = ctx->createOnScreenFboSetWithRenderPass(rp, onScreenFboInfo);
		}
		else
		{
			for (uint32 i = 0; i < ctx->getSwapChainLength(); ++i)
			{
				fboCreateInfo[i].width = fboWidth, fboCreateInfo[i].height = fboHeight;
				fboCreateInfo[i].renderPass = rp;
				if (dsAttachments[i].isValid())
				{
					fboCreateInfo[i].setDepthStencil(0, dsAttachments[i]);
				}
			}
			fbos = ctx->createFboSet(fboCreateInfo);
		}
		return fbos[0].isValid();
	}
	return false;
}

void createPasses(Effect_& effect, std::vector<Pass>& passes, std::map<StringHash, api::PipelineLayout>& layouts,
                  std::map<StringHash, effect::PipelineDef>& createParams, std::map<StringHash,
                  std::map<StringHash, effect::TextureInfo>/**/>& samplersIndexedByPipeAndTexture)
{
	const assets::effect::Effect& assetEffect = effect.getEffectAsset();
	uint32 pass_idx = 0;

	passes.resize(assetEffect.passes.end() - assetEffect.passes.begin());

	// for each pass
	for (auto pass_it = assetEffect.passes.begin(); pass_it != assetEffect.passes.end(); ++pass_it, ++pass_idx)
	{
		Pass& pass = passes[pass_idx];
		// The color index keep an index in to the fbo's color attachments. will be used later
		std::vector<std::pair<StringHash, uint32>/**/> colorIndex;
		getRenderPassAndFboForPass(effect, assetEffect, *pass_it, pass.fbos, pass.renderPass, colorIndex);

		uint32 subpass_idx = 0;
		pass.subpasses.resize((uint32)(pass_it->subpasses.end() - pass_it->subpasses.begin()));
		// for each subpasses within the pass

		for (auto subpass_it = pass_it->subpasses.begin(); subpass_it != pass_it->subpasses.end(); ++subpass_it, ++subpass_idx)
		{
			auto subpassGroupId = uint32(0);
			Subpass& subpass = pass.subpasses[subpass_idx];
			subpass.groups.resize((uint32)(subpass_it->groups.end() - subpass_it->groups.begin()));
			for (auto supassGroup_it = subpass_it->groups.begin(); supassGroup_it != subpass_it->groups.end();
			     ++supassGroup_it, ++subpassGroupId)
			{
				uint32 pipe_idx = 0;
				SubpassGroup& group = subpass.groups[subpassGroupId];
				group.name = supassGroup_it->name;
				group.pipelines.resize((uint32)(supassGroup_it->pipelines.end() - supassGroup_it->pipelines.begin()));
				for (auto pipe_it = supassGroup_it->pipelines.begin(); pipe_it != supassGroup_it->pipelines.end();
				     ++pipe_it, ++pipe_idx)
				{
					ConditionalPipeline& pipeline = group.pipelines[pipe_idx];
					pipeline.conditions = pipe_it->conditions;
					pipeline.identifiers = pipe_it->identifiers;

					const assets::effect::PipelineDefinition* pipedef = getPipeline(assetEffect, effect.getApiString(), pipe_it->pipelineName);

					if (pipedef == NULL)
					{
						Log("EffectApi initialization: Could not find the pipeline [%s] referenced in pass #%d subpass #%d",
						    pipe_it->pipelineName.c_str(), pass_idx, subpass_idx);
						continue;
					}
					effect::PipelineDef& effectPipeDef = createParams[pipedef->name];
					api::GraphicsPipelineCreateParam& cp = effectPipeDef.createParam;

					pipeline.pipeline = pipedef->name;

					// Vertex Attributes
					effectPipeDef.attributes = pipedef->attributes;

					// INPUT Attachments
					for (uint32 i = 0; i < assets::effect::Subpass::MaxInputs; ++i)
					{
						const StringHash& name = subpass_it->inputs[i];
						if (!name.empty())
						{
							// get the location where the attachment is in the fbo
							std::vector<std::pair<StringHash, uint32>/**/>::iterator it = std::find_if(colorIndex.begin(),
							    colorIndex.end(),
							[&name](std::pair<StringHash, uint32>& attachment) {return name == attachment.first;});

							for (uint32 j = 0; j < pass.fbos.size(); ++j)
							{

								std::vector<assets::effect::InputAttachmentRef>::const_iterator assetAtatchmentRef =
								  std::find_if(pipedef->inputAttachments.begin(), pipedef->inputAttachments.end(),
								[ = ](const assets::effect::InputAttachmentRef & ref) { return ref.targetIndex == i; });
								if (assetAtatchmentRef != pipedef->inputAttachments.end())
								{
									effectPipeDef.inputAttachments[j][name] =
									  InputAttachmentInfo(pass.fbos[j]->getColorAttachment(it->second), name, assetAtatchmentRef->set,
									                      assetAtatchmentRef->binding, "");
									effectPipeDef.descSetExists[assetAtatchmentRef->set] = true;
									effectPipeDef.descSetIsMultibuffered[assetAtatchmentRef->set] = true;
								}
							}
						}
					}


					/////// ASSIGN THE SAMPLERS ///////
					auto samplers_it = samplersIndexedByPipeAndTexture.find(pipe_it->pipelineName);

					if (samplers_it != samplersIndexedByPipeAndTexture.end())
					{
						for (auto textures_it = pipedef->textures.begin(); textures_it != pipedef->textures.end(); ++textures_it)
						{
							if (!textures_it->semantic.empty())
							{
								auto tex_sampler_it = samplers_it->second.find(textures_it->semantic);
								if (tex_sampler_it != samplers_it->second.end())
								{
									cp.es2TextureBindings.setTextureUnit(tex_sampler_it->second.binding, tex_sampler_it->second.variableName.str().c_str());
									effectPipeDef.textureSamplersByTexSemantic[tex_sampler_it->first] = tex_sampler_it->second;
								}
								else
								{
									Log("EffectApi: Could not find a sampler for texture [%s], pipeline [%s] referenced in pass #%d",
									    tex_sampler_it->first.c_str(), pipe_it->pipelineName.c_str(), pass_idx);
								}

							}
							else if (!textures_it->textureName.empty())
							{
								auto tex_sampler_it = samplers_it->second.find(textures_it->textureName);
								if (tex_sampler_it != samplers_it->second.end())
								{
									effectPipeDef.textureSamplersByTexName[tex_sampler_it->first] = tex_sampler_it->second;
								}
								else
								{
									Log("EffectApi: Could not find a sampler for texture [%s], pipeline [%s] referenced in pass #%d",
									    tex_sampler_it->first.c_str(), pipe_it->pipelineName.c_str(), pass_idx);
								}
							}
							else
							{
								Log("EffectApi: Found texture for which neither name nor semantic was defined: pipeline [%s] referenced in pass #%d",
								    pipe_it->pipelineName.c_str(), pass_idx);
							}
						}
					}
					else if (!pipedef->textures.empty()) //We have textures, but no samplers
					{
						Log("EffectApi: initialization: Pipeline [%s] referenced in pass #%d had textures, but no samplers were defined for them.",
						    pipe_it->pipelineName.c_str(), pass_idx);
						continue;
					}

					/////// ASSIGN THE PIPELINE LAYOUT ///////
					auto layouts_it = layouts.find(pipe_it->pipelineName);

					if (layouts_it == layouts.end())
					{
						Log("EffectApi initialization: Could not find a layout for pipeline [%s] referenced in pass #%d", pipe_it->pipelineName.c_str(), pass_idx);
						continue;
					}
					else if (layouts_it->second.isNull())
					{
						Log("EffectApi initialization: Layout for pipeline [%s] referenced in pass #%d was null", pipe_it->pipelineName.c_str(), pass_idx);
						continue;
					}
					cp.pipelineLayout = layouts_it->second;

					/////// CONFIGURE BLENDING ETC ///////
					// add blending states for the attachment if the target is a valid string and not 'none'
					for (uint32 i = 0; i < assets::effect::Subpass::MaxTargets; ++i)
					{
						if (subpass_it->targets[i].empty() || subpass_it->targets[i] == "none") { continue; }
						cp.colorBlend.setAttachmentState(i, pipedef->blending);
					}
					cp.renderPass = pass.renderPass;
					cp.subPass = subpass_idx;

					////// CONFIGURE DEPTHSTENCILSTATES //////
					if (!subpass_it->useDepthStencil) { cp.depthStencil.enableState(false); }
					cp.depthStencil.setDepthWrite(pipedef->enableDepthWrite);
					cp.depthStencil.setDepthTestEnable(pipedef->enableDepthTest);
					cp.depthStencil.setDepthCompareFunc(pipedef->depthCmpFunc);
					cp.depthStencil.setStencilTest(pipedef->enableStencilTest);
					cp.depthStencil.setStencilFront(pipedef->stencilFront).setStencilBack(pipedef->stencilBack);


					///// CONFIGURE RASTER STATES /////
					cp.rasterizer.setCullFace(pipedef->cullFace);
					cp.rasterizer.setFrontFaceWinding(pipedef->windingOrder);

					///// CONFIGURE VERTEXINPUT BINDING /////
					for (uint32 i = 0; i < pipedef->vertexBinding.size(); ++i)
					{
						cp.vertexInput.setInputBinding((uint16)pipedef->vertexBinding[i].index, 0, pipedef->vertexBinding[i].stepRate);
					}

					/////// CONFIGURE SHADERS ETC ///////
					for (auto shader_it = pipedef->shaders.begin(); shader_it != pipedef->shaders.end(); ++shader_it)
					{
						api::Shader shader = effect.getContext()->createShader(BufferStream("VertexShader",
						                     (*shader_it)->source.data(), (*shader_it)->source.length()), (*shader_it)->type);
						if (shader.isNull())
						{
							Log("EffectApi initialization: Failed to create shader with name [%s]", (*shader_it)->name.c_str());
							continue;
						}
						switch ((*shader_it)->type)
						{
						case types::ShaderType::VertexShader: cp.vertexShader = shader; break;
						case types::ShaderType::FragmentShader: cp.fragmentShader = shader; break;
						case types::ShaderType::GeometryShader: cp.geometryShader = shader; break;
						case types::ShaderType::TessControlShader: cp.tesselationStates.setControlShader(shader); break;
						case types::ShaderType::TessEvaluationShader: cp.tesselationStates.setEvaluationShader(shader); break;
						default: Log("EffectApi initialization: Shader with name [%s] had unknown type", (*shader_it)->name.c_str()); break;
						}
					}

					for (auto& assetuniform : pipedef->uniforms)
					{
						auto& apiuniform = effectPipeDef.uniforms[assetuniform.semantic];
						static_cast<assets::effect::UniformSemantic&>(apiuniform) = assetuniform;
					}
				}
			}
		}
	}
}

struct TempDescBinding
{
	api::DescriptorSetLayoutCreateParam desclayoutcreateparam;
	bool active;
	TempDescBinding() : active(false) {}
};

struct TempDescBindings
{
	TempDescBinding layouts[4];
	uint16 pipe_tmp_asset_idx;
};

struct TempPipeIdAndSetNo
{
	uint16 pipe_id;
	uint16 set_no;
	TempPipeIdAndSetNo() {}
	TempPipeIdAndSetNo(uint16 pipe_id, uint16 set_no) : pipe_id(pipe_id), set_no(set_no) {}
};

struct TempListOfSetsEntry
{
	api::DescriptorSetLayout desclayout;
	std::vector<TempPipeIdAndSetNo> pipeids_setnos;
};

void createLayouts(Effect_& effect, std::map<StringHash, api::PipelineLayout>& pipeLayoutsIndexed)
{
	//This function will iterate over the effect in order to detect all pipeline layouts, detect any duplicates,
	//only create layouts for the ones required, and then map each pipeline to one of them.
	GraphicsContext& ctx = effect.getContext();

	const assets::effect::Effect& assetEffect = effect.getEffectAsset();

	auto it = assetEffect.versionedPipelines.find(effect.getApiString());

	auto& asset_pipes = it->second;

	std::vector<TempDescBindings> all_sets_and_duplicates;
	all_sets_and_duplicates.reserve(asset_pipes.size() * 4); // Just a starting number... No point in multiple allocations.

	uint16 pipe_idx = 0;
	for (auto it_pipe = asset_pipes.begin(); it_pipe != asset_pipes.end(); ++it_pipe)
	{
		all_sets_and_duplicates.resize(all_sets_and_duplicates.size() + 1);
		TempDescBindings& pipeBindings = all_sets_and_duplicates.back();
		pipeBindings.pipe_tmp_asset_idx = pipe_idx;
		for (auto it_buff = it_pipe->second.buffers.begin(); it_buff != it_pipe->second.buffers.end(); ++it_buff)
		{
			pipeBindings.layouts[it_buff->set].desclayoutcreateparam.setBinding(it_buff->binding, it_buff->type);
			pipeBindings.layouts[it_buff->set].active = true;
		}
		for (auto it_tex = it_pipe->second.textures.begin(); it_tex != it_pipe->second.textures.end(); ++it_tex)
		{
			pipeBindings.layouts[it_tex->set].desclayoutcreateparam.setBinding(it_tex->binding, types::DescriptorType::CombinedImageSampler);
			pipeBindings.layouts[it_tex->set].active = true;
		}

		for (auto it_inputs = it_pipe->second.inputAttachments.begin(); it_inputs != it_pipe->second.inputAttachments.end(); ++it_inputs)
		{
			pipeBindings.layouts[it_inputs->set]
			.desclayoutcreateparam.setBinding(it_inputs->binding, types::DescriptorType::InputAttachment);
			pipeBindings.layouts[it_inputs->set].active = true;
		}
		++pipe_idx;
	}

	//REMOVE ALL DUPLICATES!
	//This data structure will keep a list of each active descriptor set, together with what pipes it belongs to.
	//Afterwards, we can use it to create a "proper" list. For example, have, for each pipe, an index to a simple list...

	//Destroying all duplicates, we can actually also create the real DescriptorSetLayouts.
	std::vector<TempListOfSetsEntry> sets_with_pipe_ids;
	sets_with_pipe_ids.reserve(all_sets_and_duplicates.size() / 2); //assume half duplicate.

	for (size_t outer = 0; outer < all_sets_and_duplicates.size(); ++outer)
	{
		//We will be adding one to the list, then making sure no other duplicates of it exist in the list.
		//So, at each of these lines, each one MUST be unique, or it would have been removed already.
		for (size_t outer_set = 0; outer_set < 4; ++outer_set)
		{
			auto& current_set = all_sets_and_duplicates[outer].layouts[outer_set];
			if (current_set.active)
			{
				// Add it to our list...
				sets_with_pipe_ids.resize(sets_with_pipe_ids.size() + 1);
				sets_with_pipe_ids.back().desclayout = ctx->createDescriptorSetLayout(current_set.desclayoutcreateparam);
				sets_with_pipe_ids.back().pipeids_setnos.push_back(TempPipeIdAndSetNo((uint16)outer, (uint16)outer_set));

				// ...and remove any duplicates:
				// ...Traverse the rest of the sets...
				for (size_t inner = outer + 1; inner < all_sets_and_duplicates.size(); ++inner)
				{
					for (size_t inner_set = 0; inner_set < 4; ++inner_set)
					{
						auto& current_inner_set = all_sets_and_duplicates[inner].layouts[inner_set];
						if (current_inner_set.active)
						{
							if (current_set.desclayoutcreateparam == current_inner_set.desclayoutcreateparam)
							{
								current_inner_set.active = false;
								current_inner_set.desclayoutcreateparam.clear();
								sets_with_pipe_ids.back().pipeids_setnos.push_back(TempPipeIdAndSetNo((uint16)inner, (uint16)inner_set));
							}
						}
					}
				}
			}
		}
	}

	//Now, use these to add a pipeline -> pipeline layout effect
	//Can we additionally spot "compatible" pipeline layouts?
	std::vector<api::PipelineLayoutCreateParam> pipeLayoutCps;
	pipeLayoutCps.resize(asset_pipes.size());
	for (auto it = sets_with_pipe_ids.begin(); it != sets_with_pipe_ids.end(); ++it)
	{
		for (auto it2 = it->pipeids_setnos.begin(); it2 != it->pipeids_setnos.end(); ++it2)
		{
			pipeLayoutCps[it2->pipe_id].setDescSetLayout(it2->set_no, it->desclayout);
		}
	}

	//The actual pipeline layouts! Shared refcounting makes sure of no duplication.
	std::vector<api::PipelineLayout> pipeLayouts;
	pipeLayouts.resize(pipeLayoutCps.size());

	//Inner loop: We traverse the list twice
	for (size_t outer = 0; outer < pipeLayoutCps.size(); ++outer)
	{
		if (pipeLayouts[outer].isNull())
		{
			// Create a new layout if one does not exist, else, do nothing.
			pipeLayouts[outer] = ctx->createPipelineLayout(pipeLayoutCps[outer]);

			// Traverse the rest of the list, and assign the same pipe layout to the rest of the pipes.
			for (size_t inner = outer + 1; inner != pipeLayoutCps.size(); ++inner)
			{
				if (pipeLayoutCps[outer] == pipeLayoutCps[inner])
				{
					pipeLayouts[inner] = pipeLayouts[outer];
				}
			}
		}
	}

	size_t idx = 0;
	for (auto it = asset_pipes.begin(); it != asset_pipes.end(); ++it)
	{
		pipeLayoutsIndexed[it->first] = pipeLayouts[idx++];
	}
}

struct TempSamplers
{
	std::vector<types::PackedSamplerFilter> samplerPerTextureInOrder;
	uint16 pipe_tmp_asset_idx;
};

struct TempPipeIdAndTextureNo
{
	uint16 pipe_id;
	uint16 tex_no;
	TempPipeIdAndTextureNo() {}
	TempPipeIdAndTextureNo(uint16 pipe_id, uint16 tex_id) : pipe_id(pipe_id), tex_no(tex_id) {}
};

struct TempListOfSamplersEntry
{
	api::Sampler sampler;
	std::vector<TempPipeIdAndTextureNo> pipeids_texturenos;
};

void createSamplers(Effect_& effect, std::map<StringHash, std::map<StringHash, effect::TextureInfo>/**/>& textureInfoByPipeAndTex)
{
	GraphicsContext ctx = effect.getContext();

	const assets::effect::Effect& assetEffect = effect.getEffectAsset();

	auto it = assetEffect.versionedPipelines.find(effect.getApiString());

	auto& asset_pipes = it->second;

	std::vector<TempSamplers> all_samplers_with_duplicates;
	all_samplers_with_duplicates.reserve(asset_pipes.size() * 2); // Just a starting number... No point in multiple allocations.

	uint16 pipe_idx = 0;
	for (auto it_pipe = asset_pipes.begin(); it_pipe != asset_pipes.end(); ++it_pipe)
	{
		all_samplers_with_duplicates.resize(all_samplers_with_duplicates.size() + 1);
		all_samplers_with_duplicates.back().pipe_tmp_asset_idx = pipe_idx;
		for (auto it_tex = it_pipe->second.textures.begin(); it_tex != it_pipe->second.textures.end(); ++it_tex)
		{
			all_samplers_with_duplicates.back().samplerPerTextureInOrder.push_back(it_tex->samplerFilter);
		}
		++pipe_idx;
	}

	//REMOVE ALL DUPLICATES!
	//This data structure will keep a list of each active sampler set, together with what pipes it belongs to.
	//Afterwards, we can use it to create the "proper" list.

	//Destroying all duplicates, we can actually also create the real DescriptorSetLayouts.
	std::vector<TempListOfSamplersEntry> samplers_with_pipe_ids;
	samplers_with_pipe_ids.reserve(20); //shouldn't be that many...

	for (size_t outer = 0; outer < all_samplers_with_duplicates.size(); ++outer)
	{
		//We will be adding one to the list, then making sure no other duplicates of it exist in the list.
		//So, at each of these lines, each one MUST be unique, or it would have been removed already.
		for (size_t outer_tex = 0; outer_tex < all_samplers_with_duplicates[outer].samplerPerTextureInOrder.size(); ++outer_tex)
		{
			auto& current_sampler = all_samplers_with_duplicates[outer].samplerPerTextureInOrder[outer_tex];
			if (current_sampler != types::PackedSamplerFilter(-1))
			{
				//NEW ONE. Create a sampler for it.
				api::SamplerCreateParam param;

				types::unpackSamplerFilter(current_sampler, param.minificationFilter, param.magnificationFilter, param.mipMappingFilter);

				// Add it to our list...
				samplers_with_pipe_ids.resize(samplers_with_pipe_ids.size() + 1);
				samplers_with_pipe_ids.back().sampler = ctx->createSampler(param);
				samplers_with_pipe_ids.back().pipeids_texturenos.push_back(TempPipeIdAndTextureNo((uint16)outer, (uint16)outer_tex));

				// ...and remove any duplicates:
				// ...Traverse the rest of the list...
				for (size_t inner = outer + 1; inner < all_samplers_with_duplicates.size(); ++inner)
				{
					for (size_t inner_tex = 0; inner_tex < all_samplers_with_duplicates[inner].samplerPerTextureInOrder.size(); ++inner_tex)
					{
						auto& current_inner_sampler = all_samplers_with_duplicates[inner].samplerPerTextureInOrder[inner_tex];
						//If it is active...
						if (current_inner_sampler != types::PackedSamplerFilter(-1))
						{
							//If it is the same as the current one...
							if (current_sampler == current_inner_sampler)
							{
								//Add a reference to the original list - which item this one will be referring to.
								current_inner_sampler = types::PackedSamplerFilter(-1); //Deactivate it
								//And add a reference to it in the sampler's list.
								samplers_with_pipe_ids.back().pipeids_texturenos.push_back(TempPipeIdAndTextureNo((uint16)inner, (uint16)inner_tex));
							}
						}
					}
				}
			}
		}
	}

	//Now, we have a list of samplers, along with the items that each belongs to!
	//Now, we create a flat list of samplers - the list can be flat only because we are DIRECTLY ITERATING
	std::vector<std::vector<api::Sampler>/**/> samplers;
	samplers.resize(asset_pipes.size());
	for (auto it = samplers_with_pipe_ids.begin(); it != samplers_with_pipe_ids.end(); ++it)
	{
		for (auto it2 = it->pipeids_texturenos.begin(); it2 != it->pipeids_texturenos.end(); ++it2)
		{
			if (samplers[it2->pipe_id].size() <= it2->tex_no)
			{
				samplers[it2->pipe_id].resize(it2->tex_no + 1);
			}
			samplers[it2->pipe_id][it2->tex_no] = it->sampler;
		}
	}

	size_t idx1 = 0;
	for (auto it = asset_pipes.begin(); it != asset_pipes.end(); ++it)
	{
		size_t idx2 = 0;
		for (auto it2 = it->second.textures.begin(); it2 != it->second.textures.end(); ++it2)
		{
			if (!it2->textureName.empty())
			{
				auto& tmp = textureInfoByPipeAndTex[it->first][it2->textureName];
				static_cast<assets::effect::TextureRef&>(tmp) = static_cast<const assets::effect::TextureRef&>(*it2);
				tmp.sampler = samplers[idx1][idx2];
			}
			if (!it2->semantic.empty())
			{
				auto& tmp = textureInfoByPipeAndTex[it->first][it2->semantic];
				tmp.sampler = samplers[idx1][idx2];
				static_cast<assets::effect::TextureRef&>(tmp) = static_cast<const assets::effect::TextureRef&>(*it2);
			}
			++idx2;
		}
		++idx1;
	}
}


bool createFixedDescriptorSets(Effect_& effect, std::map<StringHash, PipelineDef>& pipelines, std::map<StringHash, api::PipelineLayout>& pipelineLayouts)
{
	std::map<api::DescriptorSetLayout, Multi<api::DescriptorSet>/**/> sets;
	for (auto& pipeDef : pipelines)
	{
		auto layout_it = pipelineLayouts.find(pipeDef.first);
		assertion(layout_it != pipelineLayouts.end(), strings::createFormatted("EffectApi::init Pipeline layout was not created correctly for pipeline [%s]", pipeDef.first.c_str()));
		auto& pipelayout = layout_it->second;
		assertion(pipelayout.isValid(), strings::createFormatted("EffectApi::init Pipeline layout was not created correctly for pipeline[%s]", pipeDef.first.c_str()));
		uint32 count = pipelayout->getNumDescritporSetLayout();
		for (uint32 i = 0; i < count; ++i)
		{
			auto& setlayout = pipelayout->getDescriptorSetLayout(i);
			assertion(setlayout.isValid(), strings::createFormatted("EffectApi::init Descriptor set layout [%d] for pipeline[%s] was \"Fixed\", but it had not been created", i, pipeDef.first.c_str()));
			if (pipeDef.second.descSetIsFixed[i])
			{
				auto& set = sets[setlayout];
				uint32 numsets = pipeDef.second.descSetIsMultibuffered[i] ? effect.getContext()->getSwapChainLength() : 1;
				for (uint32 swapindex = 0; swapindex < numsets; ++swapindex)
				{
					if (set[swapindex].isNull())
					{
						set[swapindex] = effect.getDescriptorPool()->allocateDescriptorSet(setlayout);
						if (!set[swapindex].isValid())
						{
							Log("EffectApi: Failed to create pipeline %s descriptor set for swapchain %d", pipeDef.first.str().c_str(), swapindex);
							return false;
						}
					}
				}
				pipeDef.second.fixedDescSet[i] = set;
			}
		}
	}
	return true;
}
}//

void Effect_::registerUniformSemantic(StringHash pipeline, StringHash semantic, StringHash variableName)
{
	PipelineDef* pipe = getPipelineDefinition(pipeline);
	if (pipe) { pipe->uniforms[semantic] = UniformSemantic(semantic, variableName);}
}

void Effect_::registerTextureSemantic(StringHash pipeline, StringHash semantic, uint16 set, uint16 binding)
{
	PipelineDef* pipe = getPipelineDefinition(pipeline);
	if (pipe) { pipe->textures[semantic] = ObjectSemantic(semantic, set, binding);}
}

Effect_::Effect_(const GraphicsContext& context, AssetLoadingDelegate& effectDelegate) : delegate(&effectDelegate), context(context)
{
}


bool Effect_::init(const assets::effect::Effect& effect)
{
	static bool firsttime = initializeStringLists(); (void)firsttime; // bypass the warning
	assetEffect = effect;

	apiString = findMatchingApiString(assetEffect, context->getApiType());

	std::map<StringHash, api::PipelineLayout> pipeLayoutsIndexed;
	std::map<StringHash, std::map<StringHash, TextureInfo>/**/> samplersIndexedByPipeAndTexture;

	name = effect.name;
	createLayouts(*this, pipeLayoutsIndexed);
	createSamplers(*this, samplersIndexedByPipeAndTexture);
	createPasses(*this, passes, pipeLayoutsIndexed, pipelineDefinitions, samplersIndexedByPipeAndTexture);
	createTextures(*this, *delegate, textures);
	createBuffers(*this, pipelineDefinitions, bufferDefinitions);


	descriptorPool = context->createDescriptorPool(api::DescriptorPoolCreateParam()
	                 .addDescriptorInfo(types::DescriptorType::CombinedImageSampler, 32)
	                 .addDescriptorInfo(types::DescriptorType::UniformBuffer, 16)
	                 .addDescriptorInfo(types::DescriptorType::UniformBufferDynamic, 16)
	                 .addDescriptorInfo(types::DescriptorType::StorageBuffer, 16)
	                 .addDescriptorInfo(types::DescriptorType::StorageBufferDynamic, 16)
	                 .addDescriptorInfo(types::DescriptorType::InputAttachment, 16));

	return createFixedDescriptorSets(*this, pipelineDefinitions, pipeLayoutsIndexed);
}
}

}
}// namespace api
}// namespace pvr
//!\endcond
