RECT_CONSTRUCTOR = """(int32_t x, int32_t y, uint32_t width, uint32_t height)
	{
		setOffset(pvrvk::Offset2D(x, y));
		setExtent(pvrvk::Extent2D(width, height));
	}"""

# a list of default values to be used in the corresponding PVRVk types' constructors
pvrvk_structure_constructor_defaults = {
	"VkClearRect" : {"defaults" : [["Rect2D()", "0", "1"], ["", "0", "1"]]},
	"VkImageSubresourceRange" : {"defaults" : [["ImageAspectFlags::e_MAX_ENUM", "0", "1", "0", "1"], ["", "0", "1", "0", "1"]]},
	"VkImageSubresourceLayers" : {"defaults" : [["ImageAspectFlags::e_COLOR_BIT", "0", "0", "1"]]},
	"VkStencilOpState" : {"defaults" : [["StencilOp::e_KEEP", "StencilOp::e_KEEP", "StencilOp::e_KEEP", "CompareOp::e_ALWAYS", "0xff", "0xff", "0"]]},
	"VkVertexInputBindingDescription" : {"defaults" : [["0", "0", "VertexInputRate::e_VERTEX"], ["", "", "VertexInputRate::e_VERTEX"]]},
	"VkPushConstantRange" : {"defaults" : [["ShaderStageFlags::e_ALL", "0", "0"]]},
	"VkPipelineColorBlendAttachmentState" : {"defaults" : [["false", "BlendFactor::e_ONE", "BlendFactor::e_ZERO", "BlendOp::e_ADD", "BlendFactor::e_ONE", "BlendFactor::e_ZERO", "BlendOp::e_ADD", "ColorComponentFlags::e_ALL_BITS"], ["", "BlendFactor::e_ONE", "BlendFactor::e_ZERO", "BlendOp::e_ADD", "BlendFactor::e_ONE", "BlendFactor::e_ZERO", "BlendOp::e_ADD", "ColorComponentFlags::e_ALL_BITS"]]},
	"VkViewport" : {"defaults" : [["0", "0", "1", "1", "0.f", "1.f"], ["", "", "", "", "0.f", "1.f"]]},
	"VkAttachmentReference" : {"defaults" : [["static_cast<uint32_t>(-1)", "ImageLayout::e_UNDEFINED"]]},
	"VkRect2D" : {"defaults" : [["Offset2D()", "Extent2D()"]], "custom" : [RECT_CONSTRUCTOR]}
}