#pragma once

#include <functional>
#include <glm/glm.hpp>

#include <array2d.h>

namespace rlf
{
	// Calculate a path given a start point, a goal point, the size of the map and a cost function (2d point -> cost), where a lower cost value is "easier to travel to"
	std::vector<glm::ivec2> CalculatePath(const glm::ivec2& start, const glm::ivec2& goal, const glm::ivec2& mapSize, const std::function<float(const glm::ivec2&)>& fnCost);
}