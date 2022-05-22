#include "state.h"

#include <fmt/format.h>

#include "../entity.h"
#include "../game.h"
#include "../commands.h"

using namespace std;
using namespace glm;

namespace rlf
{
	namespace state
	{
		void State::Update(StateStack& stateStack)
		{
			// Run the update. If it's not running anymore:
			auto status = UpdateImpl();
			if (status != Status::Running)
			{
				// Stop listening to events
				stateStack.back()->StopListening();
				// move it away from the stack (which now has an empty state in the back)
				auto thisState = std::move(stateStack.back());
				// remove the last (now empty) state from the stack
				stateStack.pop_back();
				// call the onDone funtion if we have one
				if (onDone)
					onDone(status == Status::Success, this);
				// if there are more states in the stack, run the "ResumeFrom" function on the state at the top of the stack
				if (!stateStack.empty())
					stateStack.back()->ResumeFrom(thisState.get());
			}
		}

		int AddTextToLine(std::vector<glm::uvec4>& buf, const std::string& text, int col, int row, const glm::vec4& color)
		{
			// Add text characters into the buffer, except space
			for (const auto c : text)
			{
				if (c != ' ')
				{
					auto td = TileData{ c,color };
					auto bufferData = td.PackSparse({ col,row });
					buf.push_back(bufferData);
				}
				++col;
			}
			return col;
		}

		void AddSeparatorLine(std::vector<glm::uvec4>& buf, int row, const glm::vec4& color, int numCols, const std::string& centeredText, char fillChar)
		{
			const auto dashTileData = TileData{ fillChar,color };
			// if we don't have text, fill the entire line with the fill character
			if (centeredText.empty())
			{
				if(fillChar != ' ')
					for (int i = 0; i < numCols; ++i)
						buf.push_back(dashTileData.PackSparse({ i,row }));
			}
			// if we do have text, fill chars on the left side and right side, and put the text in the middle
			else
			{
				auto numColsEachSide = (numCols - (centeredText.size() + 2)) / 2;
				if (fillChar != ' ')
					for (int i = 0; i < numColsEachSide; ++i)
					{
						buf.push_back(dashTileData.PackSparse({ i,row }));
						buf.push_back(dashTileData.PackSparse({ numCols - 1 - i,row }));
					}
				for (int i = 0; i<int(centeredText.size()); ++i)
				{
					auto c = centeredText[i];
					if (c != ' ')
					{
						TileData td{ c, color };
						buf.push_back(td.PackSparse({ numColsEachSide + 1 + i,row }));
					}
				}
			}
		}
	}
}