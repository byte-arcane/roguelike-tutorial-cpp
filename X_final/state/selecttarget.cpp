#include "selecttarget.h"

#include <GLFW/glfw3.h>
#include "input.h"

namespace rlf
{
	namespace state
	{
		bool SelectTarget::update(StateStack& stateStack)
		{
			if (rlf::Input::GetKeyDown(GLFW_KEY_ESCAPE))
			{
				targetPosition = { -1,-1 };
				return true;
			}
			return false;
		}

		void SelectTarget::render()
		{

		}
	}
}