/*!
\brief Contains a automated container class for managing Gles buffers and textures for a model.
\file PVRUtils/OGLES/ModelGles.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "ModelGles.h"

namespace pvr {
namespace utils {
void ModelGles::destroy()
{
	model = nullptr;
	for (auto& mesh : meshes)
	{
		if (mesh.vbos.size())
		{
			gl::DeleteBuffers(static_cast<GLsizei>(mesh.vbos.size()), mesh.vbos.data());
		}
		mesh.vbos.clear();
		if (mesh.ibo)
		{
			gl::DeleteBuffers(1, &mesh.ibo);
			mesh.ibo = 0;
		}
	}
	meshes.clear();
	if (textures.size())
	{
		gl::DeleteTextures(static_cast<GLsizei>(textures.size()), textures.data());
		textures.clear();
	}
}

void ModelGles::init(pvr::IAssetProvider& assetProvider, pvr::assets::Model& model, bool isEs2)
{
	this->model = &model;
	textures.resize(model.getNumTextures());
	meshes.resize(model.getNumMeshes());

	for (uint32_t i = 0; i < model.getNumTextures(); ++i)
	{
		bool success = pvr::utils::textureUpload(assetProvider, model.getTexture(i).getName().c_str(), textures[i], isEs2);
		if (!success) { textures[i] = 0; }
	}

	for (uint32_t i = 0; i < model.getNumMeshes(); ++i)
	{
		auto& mesh = model.getMesh(i);
		pvr::utils::createMultipleBuffersFromMesh(mesh, meshes[i].vbos, meshes[i].ibo);
	}
}
}// namespace utils
}// namespace pvr
//!\endcond