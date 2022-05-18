#include "maingame.h"

#include <GLFW/glfw3.h>
#include "input.h"

#include "../game.h"
#include "../graphics.h"
#include "../eventhandlers.h"

#include "inventory.h"

namespace rlf
{
	namespace state
	{
		bool MainGame::update(StateStack& stateStack)
		{
			// Too easy to accidentally exit the game, so avoid unless we implement quick-save
			//if (rlf::Input::GetKeyDown(GLFW_KEY_ESCAPE))
			//	return true;

			auto& g = GameState::Instance();
			auto player = GameState::Instance().Player().Entity();

			if (player != nullptr)
			{
				auto playerPos = player->GetLocation().position;
				glm::ivec2 direction{ 0,0 };
				auto oldPlayerPos = playerPos;
				if (rlf::Input::GetKeyDown(GLFW_KEY_LEFT))
					direction.x -= 1;
				if (rlf::Input::GetKeyDown(GLFW_KEY_RIGHT))
					direction.x += 1;
				if (rlf::Input::GetKeyDown(GLFW_KEY_UP))
					direction.y += 1;
				if (rlf::Input::GetKeyDown(GLFW_KEY_DOWN))
					direction.y -= 1;
				if (direction != glm::ivec2(0, 0))
				{
					MoveAdj(*player, direction);
					g.GetTurnSystem().SetWaitingForPlayerAction(false);
				}

				if (rlf::Input::GetKeyDown(GLFW_KEY_ENTER))
				{
					PickUpEverythingOrHandle(*player);
					g.GetTurnSystem().SetWaitingForPlayerAction(false);
				}

				if (rlf::Input::GetKeyDown(GLFW_KEY_P))
				{
					auto itemPile = g.CurrentLevel().GetEntity(playerPos, false);
					if (itemPile != nullptr && itemPile->DbCfg() == DbIndex::ItemPile())
						stateStack.emplace_back(new Inventory(Inventory::Mode::PickUp));
				}
				if (rlf::Input::GetKeyDown(GLFW_KEY_D))
					stateStack.emplace_back(new Inventory(Inventory::Mode::Drop));
				if (rlf::Input::GetKeyDown(GLFW_KEY_E))
					stateStack.emplace_back(new Inventory(Inventory::Mode::EquipOrUse));

#if 0 // Debugging
				if (rlf::Input::GetKey(GLFW_KEY_LEFT_CONTROL))
				{
					auto tgt = Graphics::Instance().MouseCursorTile();
					auto path = rlf::GameState::Instance().CurrentLevel().CalcPath(*player, tgt);
					//std::vector<ivec2> path; rlf::Square(path, player->GetLocation().position, 2,false);
					Graphics::Instance().SetHighlightedTiles(path);
				}
				else
					Graphics::Instance().SetHighlightedTiles({});
#endif
			}
			g.GetTurnSystem().Process();

			return false;
		}

		void MainGame::render()
		{
			if (isHeaderDirty)
			{
				isHeaderDirty = false;

				std::vector<glm::uvec4> bufferHeader;
				auto& gfx = Graphics::Instance();
				auto screenSize = gfx.ScreenSize();
				addSeparatorLine(bufferHeader, 0, glm::vec4(1,1,1,1), screenSize.x, "The Tutorial Caverns");

				auto& sparseBufferHeader = gfx.RequestBuffer("header");
				if (!sparseBufferHeader.IsInitialized())
					sparseBufferHeader.Init(sizeof(glm::uvec4), 200);
				sparseBufferHeader.Set(bufferHeader.size(), bufferHeader.data());
			}

			// For the main game view, we need all three elements
			Graphics::Instance().RenderGame();
			Graphics::Instance().RenderGui();
			Graphics::Instance().RenderHeader();
		}
	}
}