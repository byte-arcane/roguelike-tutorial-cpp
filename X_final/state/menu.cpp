#include "menu.h"
#include <GLFW/glfw3.h>
#include "input.h"
#include "../graphics.h"

namespace rlf
{
	namespace state
	{
		Status Menu::updateImpl(StateStack& stateStack)
		{
			if (Input::GetKeyDown(GLFW_KEY_ESCAPE))
				return Status::Abort;
			else if (Input::GetKeyDown(GLFW_KEY_1))
			{
				// new game
				option = 0;
				return Status::Success;
			}
			else if (Input::GetKeyDown(GLFW_KEY_2))
			{
				// continue game
				option = 1;
				return Status::Success;
			}
			else if (Input::GetKeyDown(GLFW_KEY_3))
			{
				// quit game
				option = 2;
				return Status::Success;
			}
			return Status::Running;
		}

		void Menu::render()
		{
			auto& gfx = Graphics::Instance();
			auto& sparseBuffer = gfx.RequestBuffer("menu");
			if (!sparseBuffer.IsInitialized())
			{
				sparseBuffer.Init(sizeof(glm::uvec4), 3000);
				std::vector<glm::uvec4> buffer;
			}
			gfx.RenderMenu(sparseBuffer);
		}
	}
}