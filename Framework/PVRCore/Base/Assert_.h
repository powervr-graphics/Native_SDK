/*!
\brief Defines PVR_ASSERTION macros.
\file PVRCore/Base/Assert_.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include <cassert>

#ifdef DEBUG
#ifdef _WIN32
#include <crtdbg.h>
#define PVR_ASSERTION(expr) _ASSERT((expr));
#else
#define PVR_ASSERTION(expr) assert(expr);
#endif
#else
#define PVR_ASSERTION(expr)
#endif

#ifndef __ANDROID__
//Throws a compilation error if the expression is not true. Only works on constant expressions, throws an error if not.
//Note: some compilers dont complain on creating array of size 0 so we use -1 here.
#define PVR_STATIC_ASSERT(condition, message) typedef char static_assertion_##message[(condition) ? 1 : -1];
#else
#ifndef ADD_Q
#define ADD_Q(x) #x
#define ADD_QUOTES(x) ADD_Q(x)
#endif
#define PVR_STATIC_ASSERT(condition, message) static_assert(condition, ADD_QUOTES(message))
#endif