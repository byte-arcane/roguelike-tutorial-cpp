#include "createchar.h"

#include <gl/glew.h>
#include <fmt/format.h>
#include <GLFW/glfw3.h>

#include "input.h"
#include "utility.h"

#include "death.h"
#include "maingame.h"

#include "../commands.h"
#include "../graphics.h"
#include "../signals.h"
#include "../game.h"

namespace rlf
{
	namespace state
	{
		Status CreateChar::UpdateImpl()
		{
			// Escape gets us back to the previous state, the main menu
			if (Input::GetKeyDown(GLFW_KEY_ESCAPE))
				return Status::Abort;

			// If we specified a name and press enter, the state is complete. We go back to the menu where we're going to start the new game
			if (Input::GetKeyDown(GLFW_KEY_ENTER) && !charName.empty())
				return Status::Success;

			// Listen for text input. Possibly better ways of doing this, but it mostly works. Special characters might not display properly
			char c = 0;
			for (int i = 32; i <= 96; ++i)
				if (Input::GetKeyDown(i))
				{
					c = i;
					if (!(Input::GetKey(GLFW_KEY_LEFT_SHIFT) || Input::GetKey(GLFW_KEY_RIGHT_SHIFT)))
						c = std::tolower(c);
					break;
				}
			
			// if we did read a character, append it to the name
			if (c != 0)
			{
				charName.push_back(c);
				isGuiDirty = true;
			}
			// if we read backspace, remove last char (if any)
			else if (Input::GetKeyDown(GLFW_KEY_BACKSPACE) && !charName.empty())
			{
				charName.pop_back();
				isGuiDirty = true;
			}
			
			return Status::Running;
		}

		void CreateChar::Render()
		{
			// Get a unique buffer to write the GUI data
			auto& gfx = Graphics::Instance();
			auto& sparseBuffer = gfx.RequestBuffer("createchar");
			// Initialize if necessary. 400 chars for this puny gui are more than enough.
			if (!sparseBuffer.IsInitialized())
				sparseBuffer.Init(sizeof(glm::uvec4), 400);
			if(isGuiDirty)
			{
				isGuiDirty = false;
				std::vector<glm::uvec4> buffer;
				auto screenSize = gfx.ScreenSize();
				int col = 10; // offset a bit in x
				int row = screenSize.y - 4; // 4 rows away from the top
				// add the line, first part in light grey, 2nd part in white
				col = AddTextToLine(buffer, "Enter your name: ",col, row, glm::vec4(.7, .7, .7, 1.0));
				AddTextToLine(buffer, charName, col, row, glm::vec4(1));
				// If name is non-empty, show message to proceed to next screen
				if(!charName.empty())
					AddSeparatorLine(buffer, 1, glm::vec4(0.5), screenSize.x, "Press ENTER to continue", ' ');
				// Ok done building the buffer, now send the data to the GPU
				sparseBuffer.Set(buffer.size(), buffer.data());
			}
			// Render the buffer
			gfx.RenderMenu(sparseBuffer);
		}
	}
}