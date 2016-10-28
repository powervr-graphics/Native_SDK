/*!*********************************************************************************************************************
\file         PVRApi\OGLES\Bindables.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains classes representing objects that can be bound to pipeline binding points.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#pragma once
#include "PVRCore/IGraphicsContext.h"
#include "PVRCore/ForwardDecApiObjects.h"

// Forward Declarations
namespace pvr {
class IGraphicsContext;
namespace api {
//!\cond NO_DOXYGEN
class BindDescriptorSets;
namespace impl {
template<typename> class PackagedBindable;
template<typename, typename> class PackagedBindableWithParam;
}
//!\endcond

/*!****************************************************************************************************************
\brief Interface for bindable objects. A bindable has a bind(IGraphicsContext) command.
*******************************************************************************************************************/
class IBindable
{
	template <typename> friend class ::pvr::api::impl::PackagedBindable;
	void execute(IGraphicsContext& context) { bind(context); }
public:
	virtual void bind(IGraphicsContext& context) const = 0;
	typedef void* isBindable;//!<To facilitate SFINAE compile time tricks
	virtual ~IBindable() {}
};

/*!*********************************************************************************************************************
\brief Interface for index bindable. An index bindable can be bound to an indexed  binding point : bind(IGraphicsContext, index).
***********************************************************************************************************************/
class IIndexBindable
{
	friend class ::pvr::api::BindDescriptorSets;
	friend class ::pvr::api::impl::DescriptorSet_;
	template <typename, typename> friend class ::pvr::api::impl::PackagedBindableWithParam;
public:
	virtual void bind(IGraphicsContext& context, uint32 bindIdx)const = 0;
	typedef void* isIndexBindable;//!<To facilitate SFINAE compile time tricks
	virtual ~IIndexBindable() {}
};

}
}
//!\endcond
