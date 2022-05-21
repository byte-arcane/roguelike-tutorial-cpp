#pragma once

#include <functional>
#include <glm/glm.hpp>

namespace rlf
{
	// Calculate the field of view, given a starting point, a radius, the size of the map, and two callbacks:
	//	cb_is_opaque: this is called to determine whether a 2D point refers to an opaque point on the map (e.g. a wall)
	//	cb_on_visible: this is called when a 2D point is calculated as being visible
	void CalculateFieldOfView(const glm::ivec2& start, int radius, const glm::ivec2& map_size, const std::function<bool(const glm::ivec2&)>& cb_is_opaque, const std::function<void(const glm::ivec2&)>& cb_on_visible);
}