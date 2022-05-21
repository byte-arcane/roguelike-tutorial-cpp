#include "createchar.h"

#include <gl/glew.h>
#include <fmt/format.h>
#include <GLFW/glfw3.h>

#include "input.h"
#include "utility.h"


#include "death.h"
#include "maingame.h"

#include "../eventhandlers.h"
#include "../graphics.h"
#include "../signals.h"
#include "../game.h"

namespace rlf
{
	namespace state
	{
		Status CreateChar::UpdateImpl()
		{
			if (Input::GetKeyDown(GLFW_KEY_ESCAPE))
				return Status::Abort;

			if (Input::GetKeyDown(GLFW_KEY_ENTER) && !charName.empty())
				return Status::Success;

			char c = 0;
			for (int i = 32; i <= 96; ++i)
				if (Input::GetKeyDown(i))
				{
					c = i;
					if (!(Input::GetKeyDown(GLFW_KEY_LEFT_SHIFT) || Input::GetKeyDown(GLFW_KEY_RIGHT_SHIFT)))
						c = std::tolower(c);
					break;
				}
			
			if (c != 0)
			{
				charName.push_back(c);
				isGuiDirty = true;
			}
			else if (Input::GetKeyDown(GLFW_KEY_BACKSPACE) && !charName.empty())
			{
				charName.pop_back();
				isGuiDirty = true;
			}
			
			return Status::Running;
		}

		void CreateChar::Render()
		{
			auto& gfx = Graphics::Instance();
			auto& sparseBuffer = gfx.RequestBuffer("createchar");
			if (!sparseBuffer.IsInitialized())
				sparseBuffer.Init(sizeof(glm::uvec4), 4000);
			if(isGuiDirty)
			{
				isGuiDirty = false;
				std::vector<glm::uvec4> buffer;
				auto screenSize = gfx.ScreenSize();
				int col = 10;
				int row = screenSize.y - 4;
				AddTextToLine(buffer, fmt::format("Enter your name: {0}",charName),col, row, glm::vec4(1));
				
				if(!charName.empty())
					AddSeparatorLine(buffer, 1, glm::vec4(0.5), screenSize.x, "Press ENTER to continue", ' ');
				sparseBuffer.Set(buffer.size(), buffer.data());
			}
			gfx.RenderMenu(sparseBuffer);
		}
	}
}