#include "maingame.h"

#include <GLFW/glfw3.h>
#include "input.h"

#include "../game.h"
#include "../graphics.h"
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
			Game::Instance().EnterLevel();
		}

		void MainGameUpdate()
		{
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