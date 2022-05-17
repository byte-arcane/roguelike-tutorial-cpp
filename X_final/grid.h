#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace rlf
{
	// bresenham
	// nb4/nb8
	// ring

	const std::vector<glm::ivec2>& Nb4();
	const std::vector<glm::ivec2>& Nb8();
	void Line(std::vector<glm::ivec2>& points, const glm::ivec2& start, const glm::ivec2& end);
	void Line4(std::vector<glm::ivec2>& points, const glm::ivec2& start, const glm::ivec2& end);
	void Circle(std::vector<glm::ivec2>& points, const glm::ivec2& center, int radius, bool sortByDistance);
	void Square(std::vector<glm::ivec2>& points, const glm::ivec2& center, int radius, bool sortByDistance);
	void Diamond(std::vector<glm::ivec2>& points, const glm::ivec2& center, int radius, bool sortByDistance);
}