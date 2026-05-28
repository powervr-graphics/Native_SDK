PVRSuperResolution
==================

Overview
--------
PVRSuperResolution library brings to the SDK a set of postprocessing techniques focused on sharpening and upscaling (both spatial and temporal). It implements the techiques below:
  - Mentis Vulkan sharpener.
  - Mentis Vulkan spatial upscaler.

This techniques can be called from an application, and will perform the sharpening / upscaling on the input images provided, storing the results in the output image provided. This library is self-contained and does not have dependencies from other libraries in the PowerVR framework.

A technique might require a single or several postprocessing passes. All postprocessing passes a technique requires are added to the ``SuperResolution::_vectorPass`` vector, casted as PostProcessingPass class instances, interface from which all postprocessing passes in the library inherit from.

.. figure:: ../docs/images/PVRSuperResolution.png

Using PVRSuperResolution
------------------------

All techniques follow the same approach:
- The application using the PVRSuperResolution library declares a new ``pvr::SuperResolution`` variable and an initialization struct. Depending on whether the technique to use is Vulkan API or OpenCL API based, this struct will be ``pvr::VulkanInitializationData`` or ``pvr::OpenCLPostProcessingInitializationData``.
    - Both structs have a long set of fields which need to be filled. These fields represent information on the inputs required by the PVRSuperResolution technique to use, and the output images where the technique should store the results. Calling ``SuperResolution::init`` will, depending on the technique selected (specified by the ``VulkanInitializationData::supernovaMethod`` variable), make the ``pvr::SuperResolution`` library to build the postprocessing passes each technique might need. For instance:
        - ``PostProcessingMethod::SupernovaV1Mode1X`` will add to ``SuperResolution::_vectorPass`` instances of SupernovaYUVAConversionPass and SupernovaV1Mode1XPass.
        - ``PostProcessingMethod::SupernovaV1Mode2X`` will add to ``SuperResolution::_vectorPass`` instances of SupernovaYUVAConversionPass, SupernovaV1Mode2XPassUpscale and SupernovaV1Mode2XPassOutput.
        
- For Vulkan API based techniques (Mentis Vulkan sharpener and Mentis Vulkan spatial upscaler), the application using call to ``SuperResolution::recordCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferIndex)`` will record the commands the technique requires for the command buffer and command buffer index given as parameter.
    - The commandBufferIndex usually matches one of the possible swapchain indices (if there are three swapchain images, commandBufferIndex will take values in {0, 1, 2}). The number of command buffer indices is specified to the PVRSuperResolution technique at initialization time in the ``SuperResolution::init`` method, in the ``VulkanInitializationData::numberCommandBuffer`` field.
    - The command buffer provided by the application using the PVRSuperResolution library in this ``SuperResolution::recordCommands`` call will contain the commands for the technique, ready to be submitted to the corresponding queue (which is also provided at init time through ``VulkanInitializationData::queue`` and ``VulkanInitializationData::queueFamilyIndex``).
    - Before each frame update, and in the case the PVRSuperResolution technique requires any per-frame updates, the application using the PVRSuperResolution library will use a ``pvr::DynamicMap`` variable (declared by the application and provided at init time to PVRSuperResolution in ``VulkanInitializationData::dynamicMap``) to update any values required by the technique being used. Once the values in the ``pvr::DynamicMap`` variable are updated, then a call to ``SuperResolution::frameUpdate(int swapchainIndex)`` to notify the ``PVRSuperResolution`` is required, which will do the corresponding internal operations to update the provided values.
    - Command buffers can now be submitted to the corresponding GPU queue, where the technique from the PVRSuperResolution library will perform operations recorded in the command buffer previously provided in ``SuperResolution::recordCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferIndex)``.

Supernova V1 Mode 2X spatial upscaler
-------------------------------------

Overview
~~~~~~~~
This upscaler algorithm follows a spatial approach, requiring only a color input image at half resolution (960x540), which is converted to YUV color space in a postprocessing pass. Then a set of convolutions are applied to the image in another postprocessing pass, generating the final new image at FullHD resolution (1920x1080). Finally, the image is converted back from YUV color space to RGB in a third and last postprocessing pass. It only requires an RGB input at half resolution (960x540) from the scene, to generate a FullHD output.

How to use
~~~~~~~~~~
The Vulkan SDK sample ``Supernova`` implements this technique from the PVRSuperResolution library. The source code can be found in ``examples\Vulkan\Supernova\VulkanSupernova.cpp``. The client application requires four points to use the techniques present in the library: initialization, command buffer recording, per-frame update information, and command buffer submission. These steps are explained in the points below.

    - The variable ``supernovaV1Mode2X`` allows to instantiate a new PVRSuperResolution technique, in this case implementing the Supernova V1 Mode 2X spatial upscaler.

.. code-block:: C
    pvr::SuperResolution* supernovaV1Mode2X = nullptr;

    - The method ``VulkanSupernova::initializeSupernova()`` takes care of generating a new ``pvr::SuperResolution`` instance to implement this technique. The different fields required by the library are initialized in a ``pvr::VulkanInitializationData`` structure.
        - All the image views used to draw the scene offscreen are provided as inputs in ``VulkanInitializationData::vectorInputImageView`` (``offscreenColorAttachmentImageViewHalfSize``). The SDK sample generates a set of images per swapchain image available, to allow submitting command buffers of more than one frame at the same time. The formats of these images are also specified in ``VulkanInitializationData::inputImageFormat``. The image layout of these images is also specified in ``VulkanInitializationData::inputImageLayout``.
        - The image views of the images where to store the results (one per swapchain) are stored in ``VulkanInitializationData::vectorOutputImageView``. In this case, the output images are present in the variable ``offscreenColorAttachmentImageFullSize``. More information on the output images is also provided, like their format, initial and final layouts, and the extent of these images.
        - Other fields are also required by the library, like the Vulkan logical device bindings and instance bindings as the library will make its own Vulkan API calls through the Vulkan API function pointers provided.
        - The field ``VulkanInitializationData::supernovaMethod`` specifies the technique to implement, ``SupernovaV1Mode2X`` meaning our spatial upscaler
        - The call to ``SuperResolution::init`` will initialize the corresponding postprocessing passes in the PVRSuperResolution library, adding to ``SuperResolution::_vectorPass`` instances of ``SupernovaYUVAConversionPass``, ``SupernovaV1Mode2XPassUpscale`` and ``SupernovaV1Mode2XPassOutput``. The code below shows the initalization of the technique (please note that the code reuses some fields from another technique (``supernovaV1Mode2X``)):

.. code-block:: C
    void VulkanSupernova::initializeSupernova()
    {
        _deviceResources->supernovaV1Mode1X = new pvr::SuperResolution();

        pvr::VulkanInitializationData postProcessingInitializationData = {};
        postProcessingInitializationData.device = _deviceResources->device->getVkHandle();
        postProcessingInitializationData.physicalDevice = _deviceResources->device->getPhysicalDevice()->getVkHandle();
        std::vector<VkImageView> vectorOutputImageView;
        for (size_t i = 0; i < _deviceResources->offscreenColorAttachmentImageViewFullSize.size(); ++i)
        {
            vectorOutputImageView.push_back(_deviceResources->swapchain->getImageView(static_cast<uint32_t>(i))->getVkHandle());

            std::vector<VkImageView> vectorInputImageView;
            vectorInputImageView.push_back(_deviceResources->offscreenColorAttachmentImageViewFullSize[i]->getVkHandle());

            postProcessingInitializationData.vectorInputImageView.push_back(vectorInputImageView);
            postProcessingInitializationData.inputImageFormat.push_back(static_cast<VkFormat>(_deviceResources->offscreenColorAttachmentImageFullSize[0]->getFormat()));
            postProcessingInitializationData.inputImageLayout.push_back(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
        postProcessingInitializationData.inputImageExtent = { getWidth(), getHeight() };
        postProcessingInitializationData.vectorOutputImageView = vectorOutputImageView;
        postProcessingInitializationData.outputImageFormat = static_cast<VkFormat>(_deviceResources->swapchain->getImageFormat());
        postProcessingInitializationData.outputImageInitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        postProcessingInitializationData.outputImageFinalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        postProcessingInitializationData.outputImageExtent = { getWidth(), getHeight() };
        postProcessingInitializationData.queueFlagBits = VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT;
        postProcessingInitializationData.queue = _deviceResources->graphicsQueue->getVkHandle();
        postProcessingInitializationData.queueFamilyIndex = _deviceResources->graphicsQueue->getFamilyIndex();
        postProcessingInitializationData.numberCommandBuffer = _deviceResources->swapchain->getSwapchainLength();
        postProcessingInitializationData.vk = &_deviceResources->device->getVkBindings();
        postProcessingInitializationData.vkInstance = &_deviceResources->instance->getVkBindings();
        postProcessingInitializationData.application = static_cast<void*>(getOSApplication());
        postProcessingInitializationData.supernovaMethod = pvr::PostProcessingMethod::SupernovaV1Mode1X;
        _deviceResources->supernovaV1Mode1X->init(postProcessingInitializationData);

        _deviceResources->supernovaV1Mode2X = new pvr::SuperResolution();
        postProcessingInitializationData.vectorInputImageView.clear();
        for (size_t i = 0; i < _deviceResources->offscreenColorAttachmentImageViewHalfSize.size(); ++i)
        {
            std::vector<VkImageView> vectorInputImageView;
            vectorInputImageView.push_back(_deviceResources->offscreenColorAttachmentImageViewHalfSize[i]->getVkHandle());

            postProcessingInitializationData.vectorInputImageView.push_back(vectorInputImageView);
        }
        postProcessingInitializationData.supernovaMethod = pvr::PostProcessingMethod::SupernovaV1Mode2X;
        postProcessingInitializationData.inputImageExtent = { getWidth() / 2, getHeight() / 2 };
        _deviceResources->supernovaV1Mode2X->init(postProcessingInitializationData);
    }

    - In the ``void VulkanSupernova::recordSupernovaV1Mode2X()`` method records all the command buffers required for a complete frame using the Supernova V1 Mode 2X spatial upscaler from the PVRSuperResolution library. The same command buffer ``supernovaV1Mode2XCommandBuffer`` is used to record a render pass which will draw the scene to an offscreen image, also it is provided to the UI library to draw the user interface, and also it is provided to the PVRSuperResolution library to record the commands required to apply the upscaler to the input image provided at initialization `(``offscreenColorAttachmentImageViewHalfSize``).

.. code-block:: C
    void VulkanSupernova::recordSupernovaV1Mode2X()
    {
        // TODO: Refactor with V1Mode1X
        for (uint32_t i = 0; i < _swapchainLength; ++i)
        {
            _deviceResources->supernovaV1Mode2XCommandBuffer[i]->setObjectName("SupernovaV1Mode1XCommandBufferSwapchain[" + std::to_string(i) + "]");

            _deviceResources->supernovaV1Mode2XCommandBuffer[i]->begin();

            pvr::utils::beginCommandBufferDebugLabel(_deviceResources->supernovaV1Mode2XCommandBuffer[i], pvrvk::DebugUtilsLabel("Supernova v1 mode 2X offscreen pass"));

            // Do an initial offscreen pass writing the scene to a color attachment
            _deviceResources->supernovaV1Mode2XCommandBuffer[i]->beginRenderPass(
                _deviceResources->offscreenFramebufferHalfSize[i], pvrvk::Rect2D(0, 0, getWidth() / 2, getHeight() / 2), true, _clearValues, _isImageOutputMode ? 1 : 2);
            _deviceResources->supernovaV1Mode2XCommandBuffer[i]->bindPipeline(_deviceResources->offscreenPipelineHalfSize);
            _deviceResources->supernovaV1Mode2XCommandBuffer[i]->bindDescriptorSet(
                pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->scenePipelineLayout, 0u, _deviceResources->sceneFragmentDescriptorSets[i]);
            if (_isImageOutputMode)
            {
                _deviceResources->supernovaV1Mode2XCommandBuffer[i]->draw(0, 3);
            }
            else
            {
                _deviceResources->supernovaV1Mode2XCommandBuffer[i]->bindDescriptorSet(
                    pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->scenePipelineLayout, 1u, _deviceResources->sceneVertexDescriptorSets[i]);
                drawMesh(_deviceResources->supernovaV1Mode2XCommandBuffer[i], 0);
            }

            if (_showUIRendererText)
            {
                recordUIRendererCommands(_deviceResources->supernovaV1Mode2XCommandBuffer[i], static_cast<int>(SupernovaTechnique::SUPERNOVA_V1_MODE_2X));
            }
            
            _deviceResources->supernovaV1Mode2XCommandBuffer[i]->endRenderPass();
            pvr::utils::endCommandBufferDebugLabel(_deviceResources->supernovaV1Mode2XCommandBuffer[i]);

            pvr::utils::beginCommandBufferDebugLabel(_deviceResources->supernovaV1Mode2XCommandBuffer[i], pvrvk::DebugUtilsLabel("Supernova v1 mode 2X library call pass"));

            // Call the Supernova library for the Supernova V1 Mode 2X postprocessing pass
            _deviceResources->supernovaV1Mode2X->recordCommands(const_cast<VkCommandBuffer>(_deviceResources->supernovaV1Mode2XCommandBuffer[i]->getVkHandle()), i);

            // After the call, the swapchain image has the sharpened result and is ready for present

            pvr::utils::endCommandBufferDebugLabel(_deviceResources->supernovaV1Mode2XCommandBuffer[i]);

            _deviceResources->supernovaV1Mode2XCommandBuffer[i]->end();
        }
    }

- The method ``pvr::Result VulkanSupernova::renderFrame()`` will call method ``void VulkanSupernova::submitSupernovaV1Mode2XCommands()`` which will submit the command buffer ``supernovaV1Mode2XCommandBuffer`` recorded in ``void VulkanSupernova::recordSupernovaV1Mode2X()`` which contains Vulkan commands for the offscreen scene rendering, UI rendering, and from the PVRSuperResolution library implementing the Supernova V1 Mode 2X spatial upscaler.

.. code-block:: C
    pvr::Result VulkanSupernova::renderFrame()
    {
        _deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->imageAcquiredSemaphores[_frameId]);

        _swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

        // Update uniforms
        // [...]

        _deviceResources->perFrameResourcesFences[_swapchainIndex]->wait();
        _deviceResources->perFrameResourcesFences[_swapchainIndex]->reset();

        // Send commands depending on the technique selected
        switch (_currentTechnique)
        {
        case SupernovaTechnique::SUPERNOVA_V1_MODE_1X: {
            submitSupernovaV1Mode1XCommands();
            break;
        }
        case SupernovaTechnique::SUPERNOVA_V1_MODE_2X: {
            submitSupernovaV1Mode2XCommands();
            break;
        }
        case SupernovaTechnique::MENTIS_SPATIAL_UPSCALER: {
            submitMentisOpenCLUpscalerCommands();
            _deviceResources->vectorMentisBlitCommandBufferFence[_swapchainIndex]->wait();
            break;
        }
        default: {
            Log(LogLevel::Error, "Wrong technique");
            break;
        }
        }

        pvrvk::PresentInfo presentInfo;
        presentInfo.waitSemaphores = &_deviceResources->graphicsSemaphores[_swapchainIndex];
        presentInfo.numWaitSemaphores = 1;
        presentInfo.swapchains = &_deviceResources->swapchain;
        presentInfo.numSwapchains = 1;
        presentInfo.imageIndices = &_swapchainIndex;

        // As above we must present using the same VkQueue as submitted to previously
        _deviceResources->graphicsQueue->present(presentInfo);

        //[...]

        return pvr::Result::Success;
    }

Shader RGBToYUV.fsh
~~~~~~~~~~~~~~~~~~~
This simple postprocessing shader is the first step of a set of three postprocessing passes and will convert each input colour image from RGB to YUV colourspace by using a specific Colour Conversion Matrix CCM (defined as three rows in variables ``RGBToYUVMatrixRow0``, ``RGBToYUVMatrixRow1`` and ``RGBToYUVMatrixRow2``), storing the result in a Vulkan VkImage done by the PVRSuperResolution library with integer, 16-bit pixel format. The Y component of the YUV colour value is stored separately as it is heavily sampled in the next postprocessing pass and it contributes to performance. More details on RGB <-> YUV colour conversion in section "Conversion to/from RGB" in https://en.wikipedia.org/wiki/Y%E2%80%B2UV

.. code-block:: C
    highp ivec3 RGBToYUVMatrixRow0 = ivec3( 4899,   9617,  1868);
    highp ivec3 RGBToYUVMatrixRow1 = ivec3(-2411,  -4733,  7143);

    //[...]

    void main()
    {
       ivec2 texturePosition = ivec2(gl_FragCoord.x, gl_FragCoord.y);
       highp vec4 textureValue = texelFetch(sBaseTex, texturePosition, 0) * 255.0;

       highp ivec4 textureValueInteger = ivec4(textureValue + vec4(0.5));
       
       highp ivec3 YUV = ivec3(integerDot(textureValueInteger.xyz, RGBToYUVMatrixRow0),
                               integerDot(textureValueInteger.xyz, RGBToYUVMatrixRow1),
                               integerDot(textureValueInteger.xyz, RGBToYUVMatrixRow2));

       YUV = round_half_up_vec3(YUV, ivec3(10));

       outputY = YUV.x;
       outputUV = YUV.yz;
    }

Shader SupernovaV1Mode2XUpscale.fsh
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
This is the main shader used for the upscaling. It will read the YUV information from the previous step, where the Y component is in one texture and the UV is in another separate texture. The algorithm performs upscaling based on a long set of convolutions applying sharpening, contrast and blending. The algorithm requires loading a 5x5 pixel neighbourhood, with a total of 25 samples per pixel:

    (-2,-2) (-2,-1) (-2, 0) (-2, 1) (-2, 2)
    (-1,-2) (-1,-1) (-1, 0) (-1, 1) (-1, 2)
    ( 0,-2) ( 0,-1) ( 0, 0) ( 0, 1) ( 0, 2)
    ( 1,-2) ( 1,-1) ( 1, 0) ( 1, 1) ( 1, 2)
    ( 2,-2) ( 2,-1) ( 2, 0) ( 2, 1) ( 2, 2)

Since bandwidth on low-end and medium-end mobile GPUs can be affected by this large amount of samples per pixel, the approach below is followed: 
- 1. First, a set of samples from this neighbourhood from the Y texture is loaded (the second and third rows). All the different convolutions that are required are done. Some can be done completely, other convolutions cannot be completely done with this information, so they are partially done and their results are stored in temporal variables.

.. code-block:: C
  dataRow0 = vec4(
        float(texelFetch(YTexture, texturePosition + ivec2( 0, -1), 0).r),
        float(texelFetch(YTexture, texturePosition + ivec2( 0,  0), 0).r),
        float(texelFetch(YTexture, texturePosition + ivec2( 0,  1), 0).r),
        float(texelFetch(YTexture, texturePosition + ivec2( 0,  2), 0).r)
        );

    dataRow1 = vec4(
        float(texelFetch(YTexture, texturePosition + ivec2( 1, -1), 0).r),
        float(texelFetch(YTexture, texturePosition + ivec2( 1,  0), 0).r),
        float(texelFetch(YTexture, texturePosition + ivec2( 1,  1), 0).r),
        float(texelFetch(YTexture, texturePosition + ivec2( 1,  2), 0).r)
        );

The kernels applied are as shown below:
Contrast kernel. Spans over the 4x4 neighbourhood indices shown below, affecting every element of this 4x4 neighbourhood (all elements in the neighbourhood are processed):
    // (-1, -1) (-1, 0) (-1, 1) (-1, 2)
    // ( 0, -1) ( 0, 0) ( 0, 1) ( 0, 2)
    // ( 1, -1) ( 1, 0) ( 1, 1) ( 1, 2)
    // ( 2, -1) ( 2, 0) ( 2, 1) ( 2, 2)

Convolution 2D Y: Affects only four elements in the neighbourhood:
    ( 0, 0) ( 0, 1) 
    ( 1, 0) ( 1, 1)
    Kernel weights are (note many weights are 0):
    (4,0)
    (0,0)

Sharp y kernel: Spans through a 4x4 neighbourhood, with the indices below showing what part is required.
    (-1, -1) (-1, 0) (-1, 1) (-1, 2)
    ( 0, -1) ( 0, 0) ( 0, 1) ( 0, 2)
    ( 1, -1) ( 1, 0) ( 1, 1) ( 1, 2)
    ( 2, -1) ( 2, 0) ( 2, 1) ( 2, 2)
    Kernel weights are (note many weights are 0):
    ( -3, -5, -3, 0)
    ( -5, 48, -5, 0)
    ( -3, -5, -3, 0)
    (  0,  0,  0, 0)

Convolution 2D Y: Affects only four elements in the neighbourhood:
    ( 0, 0) ( 0, 1)
    ( 1, 0) ( 1, 1)
    Kernel weights are (note many weights are 0):
    (2, 2)
    (0, 0)

Sharp y kernel: Spans through a 4x4 neighbourhood, with the indices below showing what part is required.
    (-1, -1) (-1, 0) (-1, 1) (-1, 2)
    ( 0, -1) ( 0, 0) ( 0, 1) ( 0, 2)
    ( 1, -1) ( 1, 0) ( 1, 1) ( 1, 2)
    ( 2, -1) ( 2, 0) ( 2, 1) ( 2, 2)
    Kernel weights are (note many weights are 0):
    ( -1, -4, -4, -1)
    ( -1, 19, 19, -1)
    ( -1, -4, -4, -1)
    (  0,  0,  0,  0)

Edge x kernel. Spans through the whole 5x5 neighbourhood. The values of this kernel are below:
    ( 0,  -1,  -1,  -1,   0)
    (-1,  -2,  -1,   0,   1)
    (-1,  -1,   0,   1,   1)
    (-1,   0,   1,   2,   1)
    ( 0,   1,   1,   1,   0)

Edge y kernel. Spans as well through the whole 5x5 neighbourhood. The values of this kernel are below:
    ( 0,   1,   1,   1,   0)
    (-1,   0,   1,   2,   1)
    (-1,  -1,   0,   1,   1)
    (-1,  -2,  -1,   0,   1)
    ( 0,  -1,  -1,  -1,   0)

Convolution 2D Y: Affects only four elements in the neighbourhood:
    ( 0, 0) ( 0, 1)
    ( 1, 0) ( 1, 1)
    Kernel weights are (note many weights are 0):
    (1, 0)
    (0, 1)

Convolution 2D Y: Affects only four elements in the neighbourhood:
    ( 0, 0) ( 0, 1)
    ( 1, 0) ( 1, 1)
    Kernel weights are (note many weights are 0):
    (0, 1)
    (1, 0)

Sharp y kernel: Spans through a 4x4 neighbourhood, with the indices below showing what part is required.
    (-1, -1) (-1, 0) (-1, 1) (-1, 2)
    ( 0, -1) ( 0, 0) ( 0, 1) ( 0, 2)
    ( 1, -1) ( 1, 0) ( 1, 1) ( 1, 2)
    ( 2, -1) ( 2, 0) ( 2, 1) ( 2, 2)
    Kernel weights are (note many weights are 0):
    (  0, -1, -1,  0)
    ( -1, 16, -4, -1)
    ( -1, -4, 16, -1)
    (  0, -1, -1,  0)

Sharp y kernel: Spans through a 4x4 neighbourhood, with the indices below showing what part is required.
    (-1, -1) (-1, 0) (-1, 1) (-1, 2)
    ( 0, -1) ( 0, 0) ( 0, 1) ( 0, 2)
    ( 1, -1) ( 1, 0) ( 1, 1) ( 1, 2)
    ( 2, -1) ( 2, 0) ( 2, 1) ( 2, 2)
    Kernel weights are (note many weights are 0):
    (  0, -1, -1,  0)
    ( -1, -4, 16, -1)
    ( -1, 16, -4, -1)
    (  0, -1, -1,  0)


- 2. A second set of samples are then loaded (first and third rows) from the Y texture. More convolutions are done and the ones that can be completed, are completed.

.. code-block:: C
    dataRow0 = vec4(
        float(texelFetch(YTexture, texturePosition + ivec2(-1, -1), 0).r),
        float(texelFetch(YTexture, texturePosition + ivec2(-1,  0), 0).r),
        float(texelFetch(YTexture, texturePosition + ivec2(-1,  1), 0).r),
        float(texelFetch(YTexture, texturePosition + ivec2(-1,  2), 0).r)
        );

    dataRow1 = vec4(
        float(texelFetch(YTexture, texturePosition + ivec2( 2, -1), 0).r),
        float(texelFetch(YTexture, texturePosition + ivec2( 2,  0), 0).r),
        float(texelFetch(YTexture, texturePosition + ivec2( 2,  1), 0).r),
        float(texelFetch(YTexture, texturePosition + ivec2( 2,  2), 0).r)
        );

- Finally, the call to ``completeEdgeXEdgeYComputations(edge_x, edge_y);`` loads the remaining values from the 5x5 neighbourhood in the Y image and the largest convolutions, which had a few values pending to be completed, are completed.

  dataRow0.xyz = vec3(
         float(texelFetch(YTexture, texturePosition + ivec2(-2, -1), 0).r), 
         float(texelFetch(YTexture, texturePosition + ivec2(-2,  0), 0).r),
         float(texelFetch(YTexture, texturePosition + ivec2(-2,  1), 0).r)
        );

    dataRow1.xyz = vec3(
         float(texelFetch(YTexture, texturePosition + ivec2(-1, -2), 0).r), 
         float(texelFetch(YTexture, texturePosition + ivec2( 0, -2), 0).r),
         float(texelFetch(YTexture, texturePosition + ivec2( 1, -2), 0).r)
        );

Edge x kernel. Spans through the whole 5x5 neighbourhood. The values of this kernel are below. As we can see, there are several 0 elements which might allow not sampling that pixel:
    ( 0,  -1,  -1,  -1,   0)
    (-1,  -2,  -1,   0,   1)
    (-1,  -1,   0,   1,   1)
    (-1,   0,   1,   2,   1)
    ( 0,   1,   1,   1,   0)

Sharp y kernel: Spans through a 4x4 neighbourhood, with the indices below showing what part is required.
    (-1, -1) (-1, 0) (-1, 1) (-1, 2)
    ( 0, -1) ( 0, 0) ( 0, 1) ( 0, 2)
    ( 1, -1) ( 1, 0) ( 1, 1) ( 1, 2)
    ( 2, -1) ( 2, 0) ( 2, 1) ( 2, 2)
    Kernel values are as well populated with 0 values
    (  0, -1, -1,  0)
    ( -1, 16, -4, -1)
    ( -1, -4, 16, -1)
    (  0, -1, -1,  0)

Sharp y kernel: Spans through a 4x4 neighbourhood, with the indices below showing what part is required.
    (-1, -1) (-1, 0) (-1, 1) (-1, 2)
    ( 0, -1) ( 0, 0) ( 0, 1) ( 0, 2)
    ( 1, -1) ( 1, 0) ( 1, 1) ( 1, 2)
    ( 2, -1) ( 2, 0) ( 2, 1) ( 2, 2)
    Kernel values are as well populated with 0 values
    (  0, -1, -1,  0)
    ( -1, -4, 16, -1)
    ( -1, 16, -4, -1)
    (  0, -1, -1,  0)

Contrast kernel. Spans over the 4x4 neighbourhood indices shown below, affecting every element of this 4x4 neighbourhood (all elements in the neighbourhood are processed):
    (-1, -1) (-1, 0) (-1, 1) (-1, 2)
    ( 0, -1) ( 0, 0) ( 0, 1) ( 0, 2)
    ( 1, -1) ( 1, 0) ( 1, 1) ( 1, 2)
    ( 2, -1) ( 2, 0) ( 2, 1) ( 2, 2)

- 3. There is a small set of convolutions done which only require information from the UV channels of the image, which are done afterwards.

.. code-block:: C
  dataRow0 = vec4(
        float(texelFetch(UVTexture, texturePosition + ivec2(0, 0), 0).r),
        float(texelFetch(UVTexture, texturePosition + ivec2(0, 1), 0).r),
        float(texelFetch(UVTexture, texturePosition + ivec2(1, 0), 0).r),
        float(texelFetch(UVTexture, texturePosition + ivec2(1, 1), 0).r)
        );

    dataRow1 = vec4(
        float(texelFetch(UVTexture, texturePosition + ivec2(0, 0), 0).g),
        float(texelFetch(UVTexture, texturePosition + ivec2(0, 1), 0).g),
        float(texelFetch(UVTexture, texturePosition + ivec2(1, 0), 0).g),
        float(texelFetch(UVTexture, texturePosition + ivec2(1, 1), 0).g)
        );

Convolution 2D Y: Affects only four elements in the neighbourhood:
    ( 0, 0) ( 0, 1)
    ( 1, 0) ( 1, 1)
    The values of the kernel also have mostly 0 values
    (4,0)
    (0,0)

Convolution 2D Y: Affects only four elements in the neighbourhood:
    ( 0, 0) ( 0, 1)
    ( 1, 0) ( 1, 1)
    The values of the kernel also have mostly 0 values
    (2,0)
    (2,0)

Convolution 2D Y: Affects only four elements in the neighbourhood:
    ( 0, 0) ( 0, 1)
    ( 1, 0) ( 1, 1)
    The values of the kernel also have mostly 0 values
    (2,2)
    (0,0)

Convolution 2D Y: Affects only four elements in the neighbourhood:
    ( 0, 0) ( 0, 1)
    ( 1, 0) ( 1, 1)
    The values of the kernel also have mostly 0 values
    (1,1)
    (1,1)

The final 4 pixels outputted values are stored in three RGBA textures with pixel format 16-bit integer.

Shader SupernovaV1Mode2XOutput.fsh
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Similar to the RGBToYUV shader, this simple postprocessing shader converts from YUV back to RGB using some Colour Conversion Matrix (CCM).

.. code-block:: C
    highp const ivec3 YUVToRGBMatrixRow0 = ivec3( 16384,     0, 18675);
    highp const ivec3 YUVToRGBMatrixRow1 = ivec3( 16384, -6466, -9512);
    highp const ivec3 YUVToRGBMatrixRow2 = ivec3( 16384, 33297,     0);

    //[...]

    void main()
    {
        // [...]

        ivec2 finalTexturePosition = texturePosition / 2;
        highp ivec4 YChannel = texelFetch(YTexture, finalTexturePosition, 0);
        highp ivec4 UChannel = texelFetch(UTexture, finalTexturePosition, 0);
        highp ivec4 VChannel = texelFetch(VTexture, finalTexturePosition, 0);

        int moduleResultToBase10 = moduleResult.x + moduleResult.y * 2;

        highp int YFinal = YChannel[moduleResultToBase10];
        highp int UFinal = UChannel[moduleResultToBase10];
        highp int VFinal = VChannel[moduleResultToBase10];

        // [...]
    }


Supernova V1 Mode 1X sharpener
------------------------------

Overview
~~~~~~~~
This simple sharpener algorithm follows a similar approach to the "Supernova V1 Mode 2X spatial upscaler" algorithm: Requires only a color input, which has applied a Colour Conversion Matrix to transform RGB colour onto YUV on one postprocessing pass. Then a second postprocessing pass applies a sharpening kernel, with the results converted back from YUV onto RGB and outputted.

How to use
~~~~~~~~~~
The Vulkan SDK sample ``Supernova`` implements this technique from the PVRSuperResolution library. The source code can be found in ``examples\Vulkan\Supernova\VulkanSupernova.cpp``. The client application requires four points to use the techniques present in the library: initialization, command buffer recording, per-frame update information, and command buffer submission. These steps are explained in the points below.

    - The variable ``supernovaV1Mode1X`` allows to instantiate a new PVRSuperResolution technique, in this case implementing the Supernova V1 Mode 1X sharpener.

.. code-block:: C
    pvr::SuperResolution* supernovaV1Mode1X = nullptr;

    - The method ``VulkanSupernova::initializeSupernova()`` takes care of generating a new ``pvr::SuperResolution`` instance to implement this technique. The different fields required by the library are initialized in a ``pvr::VulkanInitializationData`` structure.
        - All the image views used to draw the scene offscreen are provided as inputs in ``VulkanInitializationData::vectorInputImageView`` (``offscreenColorAttachmentImageViewFullSize``). The SDK sample generates a set of images per swapchain image available, to allow submitting command buffers of more than one frame at the same time. The formats of these images are also specified in ``VulkanInitializationData::inputImageFormat``. The image layout of these images is also specified in ``VulkanInitializationData::inputImageLayout``.
        - The image views of the images where to store the results (one per swapchain) are stored in ``VulkanInitializationData::vectorOutputImageView``. In this case, the output images are present in the variable ``_deviceResources->swapchain->getImageView(static_cast<uint32_t>(i))->getVkHandle())`` as the output is put directly in the swapchain to be presented. More information on the output images is also provided, like their format, initial and final layouts, and the extent of these images.
        - Other fields are also required by the library, like the Vulkan logical device bindings and instance bindings as the library will make its own Vulkan API calls through the Vulkan API function pointers provided.
        - The field ``VulkanInitializationData::supernovaMethod`` specifies the technique to implement, ``SupernovaV1Mode1X`` meaning our sharpener.
        - The call to ``SuperResolution::init`` will initialize the corresponding postprocessing passes in the PVRSuperResolution library, adding to ``SuperResolution::_vectorPass`` instances of ``SupernovaYUVAConversionPass`` and ``SupernovaV1Mode1XPass``. The code below shows the initialization of the technique:

.. code-block:: C
    void VulkanSupernova::initializeSupernova()
    {
        _deviceResources->supernovaV1Mode1X = new pvr::SuperResolution();

        pvr::VulkanInitializationData postProcessingInitializationData = {};
        postProcessingInitializationData.device = _deviceResources->device->getVkHandle();
        postProcessingInitializationData.physicalDevice = _deviceResources->device->getPhysicalDevice()->getVkHandle();
        std::vector<VkImageView> vectorOutputImageView;
        for (size_t i = 0; i < _deviceResources->offscreenColorAttachmentImageViewFullSize.size(); ++i)
        {
            vectorOutputImageView.push_back(_deviceResources->swapchain->getImageView(static_cast<uint32_t>(i))->getVkHandle());

            std::vector<VkImageView> vectorInputImageView;
            vectorInputImageView.push_back(_deviceResources->offscreenColorAttachmentImageViewFullSize[i]->getVkHandle());

            postProcessingInitializationData.vectorInputImageView.push_back(vectorInputImageView);
            postProcessingInitializationData.inputImageFormat.push_back(static_cast<VkFormat>(_deviceResources->offscreenColorAttachmentImageFullSize[0]->getFormat()));
            postProcessingInitializationData.inputImageLayout.push_back(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
        postProcessingInitializationData.inputImageExtent = { getWidth(), getHeight() };
        postProcessingInitializationData.vectorOutputImageView = vectorOutputImageView;
        postProcessingInitializationData.outputImageFormat = static_cast<VkFormat>(_deviceResources->swapchain->getImageFormat());
        postProcessingInitializationData.outputImageInitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        postProcessingInitializationData.outputImageFinalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        postProcessingInitializationData.outputImageExtent = { getWidth(), getHeight() };
        postProcessingInitializationData.queueFlagBits = VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT;
        postProcessingInitializationData.queue = _deviceResources->graphicsQueue->getVkHandle();
        postProcessingInitializationData.queueFamilyIndex = _deviceResources->graphicsQueue->getFamilyIndex();
        postProcessingInitializationData.numberCommandBuffer = _deviceResources->swapchain->getSwapchainLength();
        postProcessingInitializationData.vk = &_deviceResources->device->getVkBindings();
        postProcessingInitializationData.vkInstance = &_deviceResources->instance->getVkBindings();
        postProcessingInitializationData.application = static_cast<void*>(getOSApplication());
        postProcessingInitializationData.supernovaMethod = pvr::PostProcessingMethod::SupernovaV1Mode1X;
        _deviceResources->supernovaV1Mode1X->init(postProcessingInitializationData);
    
    // [...]
    }

    - The ``void VulkanSupernova::recordSupernovaV1Mode1X()`` method records all the command buffers required for a complete frame using the Supernova V1 Mode 1X sharpener from the PVRSuperResolution library. The same command buffer ``supernovaV1Mode1XCommandBuffer`` is used to record a render pass which will draw the scene to an offscreen image, also it is provided to the UI library to draw the user interface, and also it is provided to the PVRSuperResolution library to record the commands required to apply the upscaler to the input image provided at initialization `(``offscreenColorAttachmentImageViewFullSize``).

.. code-block:: C
    void VulkanSupernova::recordSupernovaV1Mode1X()
    {
        for (uint32_t i = 0; i < _swapchainLength; ++i)
        {
            _deviceResources->supernovaV1Mode1XCommandBuffer[i]->setObjectName("SupernovaV1Mode1XCommandBufferSwapchain[" + std::to_string(i) + "]");

            _deviceResources->supernovaV1Mode1XCommandBuffer[i]->begin();

            pvr::utils::beginCommandBufferDebugLabel(_deviceResources->supernovaV1Mode1XCommandBuffer[i], pvrvk::DebugUtilsLabel("Supernova v1 mode 1X offscreen pass"));

            // Do an initial offscreen pass writing the scene to a color attachment
            _deviceResources->supernovaV1Mode1XCommandBuffer[i]->beginRenderPass(
                _deviceResources->offscreenFramebufferFullSize[i], pvrvk::Rect2D(0, 0, getWidth(), getHeight()), true, _clearValues, _isImageOutputMode ? 1 : 2);
            _deviceResources->supernovaV1Mode1XCommandBuffer[i]->bindPipeline(_deviceResources->offscreenPipelineFullSize);
            _deviceResources->supernovaV1Mode1XCommandBuffer[i]->bindDescriptorSet(
                pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->scenePipelineLayout, 0u, _deviceResources->sceneFragmentDescriptorSets[i]);
            if (_isImageOutputMode)
            {
                _deviceResources->supernovaV1Mode1XCommandBuffer[i]->draw(0, 3);
            }
            else
            {
                _deviceResources->supernovaV1Mode1XCommandBuffer[i]->bindDescriptorSet(
                    pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->scenePipelineLayout, 1u, _deviceResources->sceneVertexDescriptorSets[i]);

                drawMesh(_deviceResources->supernovaV1Mode1XCommandBuffer[i], 0);
            }

            if (_showUIRendererText)
            {
                recordUIRendererCommands(_deviceResources->supernovaV1Mode1XCommandBuffer[i], static_cast<int>(SupernovaTechnique::SUPERNOVA_V1_MODE_1X));
            }
            
            _deviceResources->supernovaV1Mode1XCommandBuffer[i]->endRenderPass();
            pvr::utils::endCommandBufferDebugLabel(_deviceResources->supernovaV1Mode1XCommandBuffer[i]);

            pvr::utils::beginCommandBufferDebugLabel(_deviceResources->supernovaV1Mode1XCommandBuffer[i], pvrvk::DebugUtilsLabel("Supernova v1 mode 1X library call pass"));

            // Call the Supernova library for the Supernova V1 Mode 1X postprocessing pass
            _deviceResources->supernovaV1Mode1X->recordCommands(const_cast<VkCommandBuffer>(_deviceResources->supernovaV1Mode1XCommandBuffer[i]->getVkHandle()), i);

            // After the call, the swapchain image has the sharpened result and is ready for present
            
            pvr::utils::endCommandBufferDebugLabel(_deviceResources->supernovaV1Mode1XCommandBuffer[i]);

            _deviceResources->supernovaV1Mode1XCommandBuffer[i]->end();
        }
    }

- The method ``pvr::Result VulkanSupernova::renderFrame()`` will call method ``void VulkanSupernova::submitSupernovaV1Mode1XCommands()`` which will submit the command buffer ``supernovaV1Mode1XCommandBuffer`` recorded in ``void VulkanSupernova::recordSupernovaV1Mode1X()`` which contains Vulkan commands for the offscreen scene rendering, UI rendering, and from the PVRSuperResolution library implementing the Supernova V1 Mode 1X sharpener.

.. code-block:: C
    pvr::Result VulkanSupernova::renderFrame()
    {
        _deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->imageAcquiredSemaphores[_frameId]);

        _swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

        // Update uniforms
        // [...]

        _deviceResources->perFrameResourcesFences[_swapchainIndex]->wait();
        _deviceResources->perFrameResourcesFences[_swapchainIndex]->reset();

        // Send commands depending on the technique selected
        switch (_currentTechnique)
        {
        case SupernovaTechnique::SUPERNOVA_V1_MODE_1X: {
            submitSupernovaV1Mode1XCommands();
            break;
        }
        case SupernovaTechnique::SUPERNOVA_V1_MODE_2X: {
            submitSupernovaV1Mode2XCommands();
            break;
        }
        case SupernovaTechnique::MENTIS_SPATIAL_UPSCALER: {
            submitMentisOpenCLUpscalerCommands();
            _deviceResources->vectorMentisBlitCommandBufferFence[_swapchainIndex]->wait();
            break;
        }
        default: {
            Log(LogLevel::Error, "Wrong technique");
            break;
        }
        }

        pvrvk::PresentInfo presentInfo;
        presentInfo.waitSemaphores = &_deviceResources->graphicsSemaphores[_swapchainIndex];
        presentInfo.numWaitSemaphores = 1;
        presentInfo.swapchains = &_deviceResources->swapchain;
        presentInfo.numSwapchains = 1;
        presentInfo.imageIndices = &_swapchainIndex;

        // As above we must present using the same VkQueue as submitted to previously
        _deviceResources->graphicsQueue->present(presentInfo);

        //[...]

        return pvr::Result::Success;
    }

Shader RGBToYUV.fsh
~~~~~~~~~~~~~~~~~~~
This shader has already been explained for the "Supernova V1 Mode 2X spatial upscaler" technique.

Shader SupernovaV1Mode1X.fsh
~~~~~~~~~~~~~~~~~~~~~~~~~~~~
This shader will read a 5x5 neighbourhood of YUC color information with integer offsets for each pixel:
    (-2,-2) (-2,-1) (-2, 0) (-2, 1) (-2, 2)
    (-1,-2) (-1,-1) (-1, 0) (-1, 1) (-1, 2)
    ( 0,-2) ( 0,-1) ( 0, 0) ( 0, 1) ( 0, 2)
    ( 1,-2) ( 1,-1) ( 1, 0) ( 1, 1) ( 1, 2)
    ( 2,-2) ( 2,-1) ( 2, 0) ( 2, 1) ( 2, 2)

- First, contrast will be computed in function ``computeContrast``, which requires iterating through all 25 elements:

.. code-block:: C
    int computeContrast()
    {
        highp int maxValue = 0;
        highp int minValue = (1 << 12) - 1;
        for (int i = 1; i < WINDOW_SIZE_Y_CHANNEL; i++)
        {
            for (int j = 1; j < WINDOW_SIZE_Y_CHANNEL; j++)
            {
                highp int px = arrayYData[i][j];
                maxValue = max(maxValue, px);
                minValue = min(minValue, px);
            }
        }
        highp int contrast = maxValue - minValue;
        // processing contrast by clipping small values to 1 and multiplying by boost factor
        if (contrast < lowerClipValue)
        {
            contrast = (1 << 12) - 1;
        }
        // n.b. reg_contrast_sensitivity has 4 fractional bits. Not all 32 bits will be needed in this multiply.
        contrast = clip_int(contrast * contrastSensitivity, 0, (1 << 16) - 1) >> 4;
        return contrast;
    }

- Then, a convolution kernel will be applied in a 4x4 neighbourhood in the function ``conv2d_int``:
    (-1,-1) (-1, 0) (-1, 1) (-1, 2)
    ( 0,-1) ( 0, 0) ( 0, 1) ( 0, 2)
    ( 1,-1) ( 1, 0) ( 1, 1) ( 1, 2)
    ( 2,-1) ( 2, 0) ( 2, 1) ( 2, 2)

The weights of this kernel are defined as below:
    (-3, -5, -3,  0)
    (-5, 48, -5,  0)
    (-3, -5, -3,  0)

Since three of the twelve elements are 0 in this kernel, these elements are not processed in the convolution:

.. code-block:: C
    int conv2d_int(int shift, int xOffset, int yOffset)
    {
       highp int result = 0;
       result += arrayYData[0 + xOffset][0 + yOffset] * -3; //unsharpKernel[0][0];
       result += arrayYData[0 + xOffset][1 + yOffset] * -5; //unsharpKernel[0][1];
       result += arrayYData[0 + xOffset][2 + yOffset] * -3; //unsharpKernel[0][2];
       result += arrayYData[1 + xOffset][0 + yOffset] * -5; //unsharpKernel[1][0];
       result += arrayYData[1 + xOffset][1 + yOffset] * 48; //unsharpKernel[1][1];
       result += arrayYData[1 + xOffset][2 + yOffset] * -5; //unsharpKernel[1][2];
       result += arrayYData[2 + xOffset][0 + yOffset] * -3; //unsharpKernel[2][0];
       result += arrayYData[2 + xOffset][1 + yOffset] * -5; //unsharpKernel[2][1];
       result += arrayYData[2 + xOffset][2 + yOffset] * -3; //unsharpKernel[2][2];

       // Perform rounding
       highp int tmp = result >> shift;
       highp int rem = result - (tmp << shift);
       if (rem >= (1 << (shift - 1)))
       {
          return tmp + 1;
       }
       else
       {
          return tmp;
       }
    }

- Finally, the results from the contrast and convolution computations are blended together in the Y channel, converting back to RGB:

.. code-block:: C
   highp int contrast = computeContrast();

   highp int s_px = conv2d_int(4, 1, 1);
   highp int YResult = (contrast * arrayYData[2][2] + (((1 << 12) - 1) - contrast) * s_px) >> 12; // Blend values
   outputResult = vec4(convertYUVToRGB(ivec3(YResult, UV)), 1.0);
   outputResult.xyz = pow(outputResult.xyz, vec3(2.2));
