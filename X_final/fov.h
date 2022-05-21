#pragma once

#include <functional>
#include <glm/glm.hpp>

namespace rlf
{
	void CalculateFieldOfView(const glm::ivec2& start, int radius, const glm::ivec2& map_size, const std::function<bool(const glm::ivec2&)>& cb_is_opaque, const std::function<void(const glm::ivec2&)>& cb_on_visible);
}