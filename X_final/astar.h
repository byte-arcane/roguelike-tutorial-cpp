#pragma once

#include <functional>
#include <glm/glm.hpp>

#include <array2d.h>

namespace rlf
{
	std::vector<glm::ivec2> CalculatePath(const glm::ivec2& start, const glm::ivec2& goal, const glm::ivec2& mapSize, const std::function<float(const glm::ivec2&)>& fnCost);
}