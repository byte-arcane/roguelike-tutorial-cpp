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
		Status Death::UpdateImpl()
		{
			// Enter ends this state
			if (Input::GetKeyDown(GLFW_KEY_ENTER))
				return Status::Success;
			return Status::Running;
		}

		void Death::Render()
		{
			// Get the unique buffers to write the data: for the death info screen and the header
			auto& gfx = Graphics::Instance();
			auto& sparseBufferDeath = gfx.RequestBuffer("death");
			// Initialize if necessary. 
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
				AddSeparatorLine(bufferHeader, 0, vec4(1), screenSize.x, "You have died");
				sparseBufferHeader.Set(bufferHeader.size(), bufferHeader.data());

				// Set the info text
				auto player = Game::Instance().PlayerId().Entity();
				std::vector<uvec4> bufferMain;
				int row0 = gfx.RowStartAndNum("main").y - 1; // First row from the top of the game view
				AddTextToLine(bufferMain, 
					fmt::format("{0} died on level {1} of the tutorial caverns",player->Name(), player->GetLocation().levelId+1),
					0, row0--, vec4(1));
				AddTextToLine(bufferMain,
					fmt::format("Killed {0} monsters", player->GetCreatureData()->xp),
					0, row0--, vec4(1));
				AddTextToLine(bufferMain,
					fmt::format("Gathered {0} items", player->GetInventory()->items.size()),
					0, row0--, vec4(1));
				AddSeparatorLine(bufferMain, 0, vec4(1), screenSize.x, "Press ENTER to return to the main menu");
				sparseBufferDeath.Set(bufferMain.size(), bufferMain.data());
			}
			
			// Render the character info, the death info, and the header
			Graphics::Instance().RenderGui();
			Graphics::Instance().RenderGameOverlay(sparseBufferDeath);
			Graphics::Instance().RenderHeader();
		}
	}
}