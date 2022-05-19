#include "state.h"

#include <fmt/format.h>

#include "../entity.h"
#include "../game.h"
#include "../eventhandlers.h"

using namespace std;
using namespace glm;

namespace rlf
{
	namespace state
	{
		void State::updateStack(StateStack& stateStack)
		{
			if (!stateStack.empty())
				stateStack.back()->update(stateStack);
		}

		void State::renderStack(StateStack& stateStack)
		{
			if (!stateStack.empty())
				stateStack.back()->render();

		}

		void State::pushToStack(StateStack& stateStack, std::unique_ptr<State>& state)
		{
			stateStack.push_back(std::move(state));
			stateStack.back()->startListening();
		}

		void State::update(StateStack& stateStack)
		{
			auto status = updateImpl(stateStack);
			if (status != Status::Running)
			{
				stateStack.back()->stopListening();
				auto thisState = std::move(stateStack.back());
				stateStack.pop_back();
				if (onDone)
					onDone(status == Status::Success, this);
				if (!stateStack.empty())
					stateStack.back()->onResumeFrom(thisState.get());
			}
		}

		int addTextToLine(std::vector<glm::uvec4>& buf, const std::string& text, int col, int row, const glm::vec4& color)
		{
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

		void addSeparatorLine(std::vector<glm::uvec4>& buf, int row, const glm::vec4& color, int numCols, const std::string& centeredText, char fillChar)
		{
			const auto dashTileData = TileData{ fillChar,color };

			if (centeredText.empty())
			{
				if(fillChar != ' ')
					for (int i = 0; i < numCols; ++i)
						buf.push_back(dashTileData.PackSparse({ i,row }));
			}
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