#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace rlf
{
	// returns offsets to the 4 adjacent neighbours: (-1,0) (1,0) (0,1), (0,-1)
	const std::vector<glm::ivec2>& Nb4();
	// returns offsets to the 8 adjacent neighbours: as above, but including diagonals
	const std::vector<glm::ivec2>& Nb8();
	// bresenham line from start to end
	void Line4(std::vector<glm::ivec2>& points, const glm::ivec2& start, const glm::ivec2& end);
	// line from start to end, with 4-connectivity
	void Line(std::vector<glm::ivec2>& points, const glm::ivec2& start, const glm::ivec2& end);
	// filled circle, given a center and a radius, optionally sorting the points by distance to center
	void Circle(std::vector<glm::ivec2>& points, const glm::ivec2& center, int radius, bool sortByDistance);
	// filled square, given a center and a radius, optionally sorting the points by distance to center
	void Square(std::vector<glm::ivec2>& points, const glm::ivec2& center, int radius, bool sortByDistance);
	// filled diamond, given a center and a radius, optionally sorting the points by distance to center
	void Diamond(std::vector<glm::ivec2>& points, const glm::ivec2& center, int radius, bool sortByDistance);
}