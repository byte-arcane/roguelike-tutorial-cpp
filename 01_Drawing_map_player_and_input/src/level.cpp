#include "level.h"

#include "graphics.h"
#include "utility.h"
#include "game.h"
#include "grid.h"
#include "signals.h"

using namespace glm;

namespace rlf
{
	Array2D<LevelBgElement> LoadLevelFromTxtFile(const std::string& filename)
	{	
		static const LevelBgElement bgFloor = { "floor", false, false, false, '.', glm::vec4(.7, .7, .7, 1) };
		static const LevelBgElement bgWall = { "wall", true, true, false, '#', glm::vec4(.7, .7, .7, 1) };
		static const LevelBgElement bgWater = { "water", false, true, true, '=', glm::vec4(0, 0, 1, 1) };

		auto text = ReadTextFile(filename);
		
		// remove all occurences of \r, for windows-style newlines, so we always split newlines with '\n'
		text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());
		// make sure we end in a newline
		if(text.back() != '\n')
			text.push_back('\n');
		// width is first occurence of newline
		int width = text.find('\n');
		int height = text.size() / (width + 1); // each line contains all chars PLUS the newline

		Array2D<LevelBgElement> bg( glm::ivec2(width, height));

		// Go through the data
		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x)
			{
				// Remember that the file starts with highest Y value first (top-to-bottom)
				auto c = text[x + (height-1-y) * (width + 1)];

				// Set the bg element -- the floor is used if we can't find the glyph (e.g. if the glyph represents treasure, under the treasure we have a floor)
				// If we have no symbol (empty space) then it's a wall
				auto bgElement = c == ' ' ? bgWall : bgFloor;
				if (c == bgWall.glyph)
					bgElement = bgWall;
				else if (c == bgWater.glyph)
					bgElement = bgWater;
				bg(x, y) = bgElement;
			}
		}

		return bg;
	}
}