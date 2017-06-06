/*!
\brief Contains the Scene Hierarchy class.
\file PVRApi/ApiObjects/SceneHierarchy.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRApi/ApiIncludes.h"
namespace pvr {
namespace api {
/// <summary>Contains all information required to create a Scene Hierarchy.</summary>
struct SceneHierarchyCreateParam
{
	SceneHierarchyCreateParam() {}
};

namespace impl {
/// <summary>API SceneHierarchy Object wrapper. Access through the framework-managed SceneHierarchy object.</summary>
class SceneHierarchy_
{
protected:
	SceneHierarchy_(const GraphicsContext& device, const SceneHierarchyCreateParam& desc) :
		_context(device), _createParams(desc) {}

	GraphicsContext _context;

	SceneHierarchyCreateParam _createParams;
public:
	const GraphicsContext& getContext()const { return _context; }
	GraphicsContext& getContext() { return _context; }

	SceneHierarchyCreateParam& getCreateParams() { return _createParams; }

	virtual ~SceneHierarchy_() { }

	/// <summary>Return this native object handle (const)</summary>
	const native::HSceneHierarchy_& getNativeObject()const;
	/// <summary>Return this native object handle</summary>
	native::HSceneHierarchy_& getNativeObject();
};
}//namespace impl
}// namspace api
}