#include "menu.h"
#include <GLFW/glfw3.h>
#include "input.h"

namespace rlf
{
	namespace state
	{
		bool Menu::update(StateStack& stateStack)
		{
			if (rlf::Input::GetKeyDown(GLFW_KEY_ESCAPE))
			{
				option = -1;
				return true;
			}
			return false;
		}

		void Menu::render()
		{

		}
	}
}