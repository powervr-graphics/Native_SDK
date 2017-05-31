/*!
\brief DO NOT USE. This file exists to make the PVRApi DOXYGEN documentation generation better.
\file PVRApi/Doxygen_Typedefs.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

namespace pvr {

namespace api {

/*!  \class pvr::api::UboView */
/// <summary>Reference-counted handle to a Uniform Buffer object view of a Buffer. Default constructor returns an empty handle
/// that wraps a NULL object. Use the IGraphicsContext's createUbo to construct a UboView. As with all
/// reference-counted handles, access with the arrow operator. For implementation details, see
/// pvr::api::impl::UboView_</summary>
class UboView{};

/// <summary>Reference-counted handle to a Shader Storage Buffer object view of a Buffer. Default constructor returns an empty
/// handle that wraps a NULL object. Use the IGraphicsContext's createUbo to construct a SsboView. As with all
/// reference-counted handles, access with the arrow operator. For implementation details, see
/// pvr::api::impl::SsboView_</summary>
class SsboView{};

/// <summary>Reference-counted handle to a Buffer. Default constructor returns an empty handle that wraps a NULL object. Use the
/// IGraphicsContext's createUbo to construct a Buffer. As with all reference-counted handles, access with the
/// arrow operator. For implementation details, see pvr::api::impl::Buffer_</summary>
class Buffer{};

/// <summary>Reference-counted handle to an Atomic Buffer object view of a Buffer. Default constructor returns an empty handle
/// that wraps a NULL object. Use the IGraphicsContext's createUbo to construct a AtomicBufferView. As with all
/// reference-counted handles, access with the arrow operator. For implementation details, see
/// pvr::api::impl::AtomicBufferView_</summary>
class AtomicBufferView{};

}
}