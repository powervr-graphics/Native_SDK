#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#ifdef __APPLE__
#define GLM_COMPILER 0
#endif

#include "../external/glm/glm.hpp"
#include "../external/glm/gtc/type_ptr.hpp"
#include "../external/glm/gtc/matrix_inverse.hpp"
#include "../external/glm/gtc/matrix_access.hpp"
#include "../external/glm/gtc/quaternion.hpp"
#include "../external/glm/gtx/transform.hpp"
#include "../external/glm/gtx/norm.hpp"
#include "../external/glm/simd/matrix.h"
#include "../external/glm/simd/trigonometric.h"
#include "../external/glm/gtx/fast_trigonometry.hpp"
#include "../external/glm/gtx/euler_angles.hpp"
#include "../external/glm/gtx/fast_square_root.hpp"

#include <sstream>

namespace glm {
template<glm::length_t L, typename T, glm::qualifier Q = glm::packed_highp>
inline std::string to_string(const glm::vec<L, T, Q>& vec)
{
	std::stringstream str;
	str << "[" << vec[0];
	for (uint32_t i = 1; i < L; ++i)
	{
		str << "," << vec[i];
	}
	str << "]";
	return str.str();
}
template<glm::length_t lC, glm::length_t lR, typename T, glm::qualifier Q = glm::packed_highp>
inline std::string to_string(const glm::mat<lC, lR, T, Q>& mat)
{
	std::stringstream str;
	str << "[" << to_string(mat[0]);
	for (uint32_t j = 1; j < lC; ++j)
	{
		str << "," << to_string(mat[j]);
	}
	str << "]";
	return str.str();
}
} // namespace glm
