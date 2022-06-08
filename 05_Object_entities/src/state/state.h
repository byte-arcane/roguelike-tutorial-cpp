#pragma once

#include <vector>
#include <functional>
#include <memory>
#include <string>

#include <glm/glm.hpp>

namespace rlf
{
	class Game;
	namespace state
	{
		// Helper that adds text to a buffer, to be used for rendering fonts on screen. Specify row, starting column and color
		int AddTextToLine(std::vector<glm::uvec4>& buf, const std::string& text, int col, int row, const glm::vec4& color);
		// Helper that adds a separator line to a buffer, as above. Optionally specify some text that should be centered, and the character to be replicated along the separator line
		void AddSeparatorLine(std::vector<glm::uvec4>& buf, int row, const glm::vec4& color, int numCols, const std::string& centeredText = "", char fillChar = '-');
	}
}