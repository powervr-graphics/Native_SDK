/*!*********************************************************************************************************************
\file         PVRAssets/BoundingBox.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief		Functionality to extract and work with the Bounding Boxes of PVRAssets (meshes etc.)
***********************************************************************************************************************/
#pragma once
#include <PVRAssets/Model.h>
#include <PVRCore/AxisAlignedBox.h>
namespace pvr {
namespace assets {
/*!*********************************************************************************************************************
\brief Contains utilities and helpers
***********************************************************************************************************************/
namespace utils {

/*!*********************************************************************************************************************
\brief Return bounding box from vertex data.
\param[in] data ?ertex data
\param[in] stride_bytes Vertex stride in bytes
\param[in] offset_bytes Offset to the vertex data
\param[in] size_bytes Data size 
\return The Axis-aligned bounding box of the data
***********************************************************************************************************************/
inline math::AxisAlignedBox getBoundingBox(const byte* data, size_t stride_bytes, size_t offset_bytes, size_t size_bytes)
{
	math::AxisAlignedBox aabb;
	assertion(data);
	assertion(stride_bytes >= 12 || !stride_bytes);
	assertion(size_bytes >= stride_bytes);
	if (size_bytes && data)
	{
		data = data + offset_bytes;
		glm::vec3 minvec; glm::vec3 maxvec;
		memcpy(&minvec, data, 12);
		memcpy(&maxvec, data, 12);

		for (size_t i = stride_bytes; i < size_bytes; i += stride_bytes)
		{
			glm::vec3 vec; memcpy(&vec, data + i, 12);
			minvec = glm::min(vec, minvec);
			maxvec = glm::max(vec, maxvec);
		}
		aabb.setMinMax(minvec, maxvec);
	}
	else
	{
		aabb.setMinMax(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0));
	}
}

/*!*********************************************************************************************************************
\brief Return bounding box of a mesh.
\param[in] mesh A mesh from which to get the bounding box of
\param[in] positionSemanticName Position attribute semantic name
\return Axis-aligned bounding box
\details It will be assumed that Vertex Position is a vec3.
***********************************************************************************************************************/
inline math::AxisAlignedBox getBoundingBox(const Mesh& mesh, const char* positionSemanticName)
{
	const Mesh::VertexAttributeData* vbo = mesh.getVertexAttributeByName(positionSemanticName);
	if (vbo)
	{
		return getBoundingBox(static_cast<const byte*>(mesh.getData(vbo->getDataIndex())), vbo->getStride(), vbo->getOffset(),
		                      mesh.getDataSize(vbo->getDataIndex()));
	}
}

/*!*********************************************************************************************************************
\brief Return bounding box of a mesh.
\param[in] mesh A mesh from which to get the bounding box of
\return Axis-aligned bounding box
\details It will be assumed that Vertex Position is a vec3 and has the semantic "POSITION".
***********************************************************************************************************************/
inline math::AxisAlignedBox getBoundingBox(const Mesh& mesh)
{
	return getBoundingBox(mesh, "POSITION");
}

/*!*********************************************************************************************************************
\brief Return bounding box of a model.
\param[in] model A model from which to get the bounding box of. All meshes will be considered. 
\return Axis-aligned bounding box
\details It will be assumed that Vertex Position is a vec3 and has the semantic "POSITION".
***********************************************************************************************************************/
inline math::AxisAlignedBox getBoundingBox(const Model& model)
{
	if (model.getNumMeshes())
	{
		math::AxisAlignedBox retval(getBoundingBox(model.getMesh(0)));
		for (int i = 1; i < model.getNumMeshes(); ++i)
		{
			retval.mergeBox(getBoundingBox(model.getMesh(i)));
		}
		return retval;
	}
	else
	{
		return math::AxisAlignedBox();
	}
}

}
}
}
