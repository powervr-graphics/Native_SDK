PVRVk
=====

Overview
--------

PVRVK is a library providing an advanced interface that is close enough to the original Vulkan interface to be perfectly useable with the Vulkan spec, while offering a sweet spot combination of simplicity, ease of use and minimal overhead. Its most important features are C++ language offering defaults and constructors for all objects, deterministic lifecycle management through reference counting and in general a clean, modern interface.

You can find the PVRVk source in the SDK package's ``framework/PVRVk`` folder.

Using PVRVk
-----------

PVRVk can be used independently of the rest of the framework, by following the Vulkan spec with the following differences:

- All enums in Vulkan have been replaced with C++ type safe scoped enums (enum class).
- All enum members lose the enum name as part of the values, and gain ``e_`` as a prefix.
- All functions become methods (member functions) of the class of their first argument.
- Sensible, obvious rules, such as using RAII for objects (objects can be released by just null-ing their last handle, or letting it go out of scope).

Code Examples
-------------

Pipeline creation:

.. code:: cpp

   pvrvk::GraphicsPipelineCreateInfo pipeCreate;

   //Create the descriptor set layouts
   pvrvk::DescriptorSetLayoutCreateInfo imageDescParam;
   imageDescParam.setBinding(0, VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, VkShaderStageFlags::e_FRAGMENT_BIT);

   pvrvk::DescriptorSetLayout descLayoutImage = device->createDescriptorSetLayout(imageDescParam);

   pvrvk::DescriptorSetLayoutCreateInfo dynamicUboDescParam;
   dynamicUboDescParam.setBinding(0, VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1, VkShaderStageFlags::e_VERTEX_BIT);

   pvrvk::DescriptorSetLayout descLayoutUboDynamic = device->createDescriptorSetLayout(dynamicUboDescParam);

   pvrvk::DescriptorSetLayoutCreateInfo uboDescParam;
   uboDescParam.setBinding(0, VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1, VkShaderStageFlags::e_VERTEX_BIT);

   pvrvk::DescriptorSetLayout descLayoutUboStatic = device->createDescriptorSetLayout(uboDescParam);

   //Create a pipeline layout from all the descriptor set layouts
   pvrvk::PipelineLayout pipeLayout = device->createPipelineLayout(
                                        pvrvk::PipelineLayoutCreateInfo()
                                        .setDescSetLayout(0, descLayoutImage)
                                        .setDescSetLayout(1, descLayoutUboDynamic)
                                        .setDescSetLayout(2, descLayoutUboStatic));

   //Load the shaders from their bytes. The createShaderModule function accepts a vector of unsigned integers containing the binary spir-v.
   pipeCreate.vertexShader = device->createShaderModule(getAssetStream("Object.vsh.spv")->readToEnd<uint32_t>());
   pipeCreate.fragmentShader = device->createShaderModule(getAssetStream("Solid.fsh.spv")->readToEnd<uint32_t>());

   //Create a color blending state without blending
   pvrvk::PipelineColorBlendAttachmentState cbStateNoBlend(false);


   pipeCreate.vertexInput.addInputAttribute(VertexAttribute(/ *attribute config here* /)).addInputBinding(VertexInputBindingDescription(/ * input bindings here * /);

   pipeCreate.rasterizer.setFrontFaceWinding(VkFrontFace::e_COUNTER_CLOCKWISE);
   pipeCreate.rasterizer.setCullMode(VkCullModeFlags::e_BACK_BIT);
   pipeCreate.depthStencil.enableDepthTest(true);
   pipeCreate.depthStencil.setDepthCompareFunc(VkCompareOp::e_LESS);
   pipeCreate.depthStencil.enableDepthWrite(true);
   pipeCreate.renderPass = onScreenFramebuffer[0]->getRenderPass();
   pipeCreate.pipelineLayout = pipeLayout;

   pipeCreate.viewport.setViewportAndScissor(0,
   pvrvk::Viewport(0, 0, swapchain->getDimension().width,  swapchain->getDimension().height),
                pvrvk::Rect2Di(0, 0, swapchain->getDimension().width, swapchain->getDimension().height));

   // create the pipeline
        = objectVsh;
       pipeCreate.fragmentShader = solidFsh;
       pipeCreate.colorBlend.setAttachmentState(0, cbStateNoBlend);

       pvrvk::GraphicsPipeline solidObjectPipeline = device->createGraphicsPipeline(pipeCreate);
       if (solidObjectPipeline.isNull())
       {
           setExitMessage("Failed to create Opaque rendering pipeline");
           return pvr::Result::UnknownError;
       }

