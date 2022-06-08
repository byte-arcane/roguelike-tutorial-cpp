#include "maingame.h"

#include <GLFW/glfw3.h>
#include "input.h"

#include "../game.h"
#include "../graphics.h"
#include "../commands.h"
#include "../grid.h"

#include "framework.h"

namespace rlf
{
	namespace state
	{
		void MainGameStart()
		{
			// Initialize the game state
			Game::Instance().New();

			// set the level to first
			EnterLevel();

			// Create the player entity
			EntityDynamicConfig dcfg;
			dcfg.position = { 28,5 };
			dcfg.nameOverride = "Sir Rodrick";
			DbIndex cfgdb{ "player" };
			auto player = Game::Instance().CreateEntity(cfgdb, dcfg, true).Entity();
			Game::Instance().SetPlayer(*player);
		}

		void MainGameUpdate()
		{
			// Quick save/load
			auto& g = Game::Instance();

			auto player = Game::Instance().PlayerId().Entity();
			if (player == nullptr)
				return;

			// End the game state if player is dead, so that we can move to the death screen
			if (player->GetCreatureData()->hp <= 0)
				return;

			// Check for direction keys (movement and related contextual actions)
			auto playerPos = player->GetLocation().position;
			glm::ivec2 direction{ 0,0 };
			auto oldPlayerPos = playerPos;
			if (Input::GetKeyDown(GLFW_KEY_LEFT))
				direction.x -= 1;
			if (Input::GetKeyDown(GLFW_KEY_RIGHT))
				direction.x += 1;
			if (Input::GetKeyDown(GLFW_KEY_UP))
				direction.y += 1;
			if (Input::GetKeyDown(GLFW_KEY_DOWN))
				direction.y -= 1;
			if (direction != glm::ivec2(0, 0))
			{
				MoveAdj(*player, direction);
			}
		}

		void MainGameRender()
		{
			auto& gfx = Graphics::Instance();
			gfx.RenderGame();
		}
	}
}