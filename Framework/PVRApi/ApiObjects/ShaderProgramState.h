/*!*********************************************************************************************************************
\file         PVRApi\ApiObjects\ShaderProgramState.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Internally used. Contains implementations for the ShaderProgramState used by the Graphics Pipeline.
***********************************************************************************************************************/
#pragma once
#include "PVRApi/ApiObjects/PipelineState.h"
#include "PVRApi/ApiObjects/ShaderProgramState.h"
namespace pvr {
namespace api {
namespace impl {
/*!*********************************************************************************************************************
\brief Pipeline graphics shader program state.
***********************************************************************************************************************/
class GraphicsShaderProgramState : public GraphicsPipelineImplState
{
public:
	GraphicsShaderProgramState() { m_isValid = false; }
	GraphicsShaderProgramState(const GraphicsShaderProgramState& shaderProgram)
		: m_shaderProgram(shaderProgram.m_shaderProgram)
	{
		m_isValid = true;
	}

	/*!*********************************************************************************************************************
	\brief Bind this program state.
	***********************************************************************************************************************/
	void bind();

	/*!*********************************************************************************************************************
	\brief Set this program state.
	***********************************************************************************************************************/
	void set(pvr::IGraphicsContext& device) { bind(); }

	/*!*********************************************************************************************************************
	\brief Reset this program state.
	***********************************************************************************************************************/
	void reset(pvr::IGraphicsContext& device);

	/*!*********************************************************************************************************************
	\brief Unset this program state.
	***********************************************************************************************************************/
	void unset(pvr::IGraphicsContext& device)
	{
		if (m_parent)
		{
			m_parent->set(device);
		}
		else {reset(device);}
	}

	/*!*********************************************************************************************************************
	\brief Return default program state.
	***********************************************************************************************************************/
	PipelineState::ptr_type createDefault()const { return new GraphicsShaderProgramState(); }

	/*!*********************************************************************************************************************
	\brief Return clone program state.
	***********************************************************************************************************************/
	PipelineState::ptr_type createClone()const { return new GraphicsShaderProgramState(*this); }

	/*!*********************************************************************************************************************
	\brief Get thi state type.
	***********************************************************************************************************************/
	GraphicsStateType::Enum getStateType() const { return GraphicsStateType::ShaderProgram; }

	bool operator==(const GraphicsShaderProgramState& rhs)const	{ return (m_shaderProgram == rhs.m_shaderProgram); }
	bool operator!=(const GraphicsShaderProgramState& rhs)const { return !(*this == rhs); }
	void generate();
	
	/*!*********************************************************************************************************************
	\brief Destroy this.
	***********************************************************************************************************************/
	void destroy();

	/*!************************************************************************************************************
	\brief	Return the api program object.
	\return	const HShaderProgram&
	***************************************************************************************************************/
	const native::HShaderProgram& getNativeHandle() const { return m_shaderProgram; }

	/*!************************************************************************************************************
	\brief	Return the api program object.
	\return	const HShaderProgram&
	***************************************************************************************************************/
	native::HShaderProgram& getNativeHandle() { return m_shaderProgram; }

	void getUniformsLocation(const char8** uniforms, uint32 numUniforms, std::vector<int32>& outLocation);
	int32 getUniformLocation(const char8* uniform);

	/*!************************************************************************
	\brief Save the program binary to a file.
	\param fileStream Output file stream. Must be writable
	\return True if it is successful
	**************************************************************************/
	bool saveProgramBinary(Stream& fileStream);

private:
	native::HShaderProgram m_shaderProgram;
	void setDefault(pvr::IGraphicsContext& device) { }
};

/*!*********************************************************************************************************************
\brief ComputePipeline shader program state.
***********************************************************************************************************************/
class ComputeShaderProgramState : public ComputePipelineImplState
{
public:

	/*!*********************************************************************************************************************
	\brief  
	***********************************************************************************************************************/
	ComputeShaderProgramState() { m_isValid = false; }
	
	/*!*********************************************************************************************************************
	\brief  
	***********************************************************************************************************************/
	ComputeShaderProgramState(const ComputeShaderProgramState& shaderProgram)
		: m_shaderProgram(shaderProgram.m_shaderProgram)
	{
		m_isValid = true;
	}

	/*!*********************************************************************************************************************
	\brief Bind this program state.
	***********************************************************************************************************************/
	void bind();

	/*!*********************************************************************************************************************
	\brief Set this program state.
	***********************************************************************************************************************/
	void set(pvr::IGraphicsContext& device) { bind(); }

	/*!*********************************************************************************************************************
	\brief Reset this program state.
	***********************************************************************************************************************/
	void reset(pvr::IGraphicsContext& device);

	/*!*********************************************************************************************************************
	\brief Unset this program state.
	***********************************************************************************************************************/
	void unset(pvr::IGraphicsContext& device)
	{
		if (m_parent)
		{
			m_parent->set(device);
		}
		else { reset(device); }
	}

	/*!*********************************************************************************************************************
	\brief Return native handle.
	***********************************************************************************************************************/
	native::HShaderProgram& getNativeHandle() { return m_shaderProgram; }

	/*!*********************************************************************************************************************
	\brief Return a default prgram state.
	***********************************************************************************************************************/
	PipelineState::ptr_type createDefault()const { return new ComputeShaderProgramState(); }

	/*!*********************************************************************************************************************
	\brief Return clone of this program state.
	***********************************************************************************************************************/
	PipelineState::ptr_type createClone()const { return new ComputeShaderProgramState(*this); }

	/*!*********************************************************************************************************************
	\brief Return this state type.
	***********************************************************************************************************************/
	GraphicsStateType::Enum getStateType() const { return GraphicsStateType::ShaderProgram; }

	bool operator==(const ComputeShaderProgramState& rhs)const	{ return (m_shaderProgram == rhs.m_shaderProgram); }
	bool operator!=(const ComputeShaderProgramState& rhs)const { return !(*this == rhs); }
	void generate();
	void destroy();

	/*!************************************************************************************************************
	\brief	Return the api program object.
	\return	const HShaderProgram&
	***************************************************************************************************************/
	const native::HShaderProgram get() const { return m_shaderProgram; }

	/*!************************************************************************************************************
	\brief	Return the api program object.
	\return	const HShaderProgram&
	***************************************************************************************************************/
	native::HShaderProgram get() { return m_shaderProgram; }

	/*!************************************************************************************************************
	\brief If free standing uniforms are supported by the underlying API (ApiCapability::Uniforms), get an array 
	       of the locations of several shader uniform variables for use with setUniform/setUniformPtr. 
		   If a uniform is inactive, its location is set to -1.
	\param uniforms An array of uniform variable names
	\param numUniforms The number of uniforms in the array
	\param outLocation A vector where the locations will be saved on a 1-1 mapping with uniforms.
	***************************************************************************************************************/
	void getUniformsLocation(const char8** uniforms, uint32 numUniforms, std::vector<int32>& outLocation);

	/*!************************************************************************************************************
	\brief If free standing uniforms are supported by the underlying API (ApiCapability::Uniforms), get the location
	       of a shader uniform variable for use with setUniform/setUniformPtr. 
		   If a uniform is inactive, -1 is returned.
	\param uniform The name of a uniform variable
	\return outLocation
	***************************************************************************************************************/
	int32 getUniformLocation(const char8* uniform);
private:
	native::HShaderProgram m_shaderProgram;
	void setDefault(pvr::IGraphicsContext& device) { }
};

}// namespace impl
}// namespace api
}// namespace pvr
