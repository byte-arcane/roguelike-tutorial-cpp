#include "death.h"

#include <GLFW/glfw3.h>
#include <fmt/format.h>

#include "input.h"
#include "../graphics.h"
#include "../game.h"

using namespace glm;

namespace rlf
{
	namespace state
	{
		Status Death::updateImpl(StateStack& stateStack)
		{
			if (Input::GetKeyDown(GLFW_KEY_ENTER))
				return Status::Success;
			return Status::Running;
		}

		void Death::render()
		{
			auto& gfx = Graphics::Instance();
			auto& sparseBufferDeath = gfx.RequestBuffer("death");
			if (!sparseBufferDeath.IsInitialized())
				sparseBufferDeath.Init(sizeof(uvec4), 2000);
			auto& sparseBufferHeader = gfx.RequestBuffer("header");
			if (!sparseBufferHeader.IsInitialized())
				sparseBufferHeader.Init(sizeof(uvec4), 200);

			if (isGuiDirty)
			{
				isGuiDirty = false;
				
				// Set the header
				std::vector<uvec4> bufferHeader;
				auto screenSize = gfx.ScreenSize();
				addSeparatorLine(bufferHeader, 0, vec4(1), screenSize.x, "You have died");
				sparseBufferHeader.Set(bufferHeader.size(), bufferHeader.data());

				// Set the info text
				auto player = GameState::Instance().Player().Entity();
				std::vector<uvec4> bufferMain;
				int row0 = gfx.RowStartAndNum("main").y - 1; // First row from the top of the game view
				addTextToLine(bufferMain, 
					fmt::format("{0} died on level {1} of the tutorial caverns",player->Name(), player->GetLocation().levelId+1),
					0, row0--, vec4(1));
				addTextToLine(bufferMain,
					fmt::format("Killed {0} monsters", player->GetCreatureData()->xp),
					0, row0--, vec4(1));
				addTextToLine(bufferMain,
					fmt::format("Gathered {0} items", player->GetInventory()->items.size()),
					0, row0--, vec4(1));
				addSeparatorLine(bufferMain, 0, vec4(1), screenSize.x, "Press ENTER to return to the main menu");
				sparseBufferDeath.Set(bufferMain.size(), bufferMain.data());
			}

			Graphics::Instance().RenderGui();
			Graphics::Instance().RenderGameOverlay(sparseBufferDeath);
			Graphics::Instance().RenderHeader();
		}
	}
}