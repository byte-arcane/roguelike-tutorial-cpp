#pragma once

#include <functional>
#include <glm/glm.hpp>

// TODO: need a function for_each_in_shape( ivec2 center, float radius, callback, norm_callback)

namespace rlf
{
	void calculate_fov(const glm::ivec2& start, int radius, const glm::ivec2& map_size, const std::function<bool(const glm::ivec2&)>& cb_is_opaque, const std::function<void(const glm::ivec2&)>& cb_on_visible);
}