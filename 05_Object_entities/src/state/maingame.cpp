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

			// Check if there's something to handle in the vicinity
			if (Input::GetKeyDown(GLFW_KEY_ENTER))
			{
				// Gather potential candidates for handling: on the ground or in adjacent squares
				std::vector<EntityId> handleTargets;
				// check our feet for anything to interact with that is NOT an item pile (need to use "pick up" command)
				auto entityOnGround = g.CurrentLevel().GetEntity(playerPos, false);
				if (entityOnGround != nullptr)
					handleTargets.push_back(entityOnGround->Id());

				for (const auto& nb4 : Nb4())
				{
					auto pnb = playerPos + nb4;
					auto entityNb = g.CurrentLevel().GetEntity(pnb, true);
					if (entityNb != nullptr && entityNb->Type() == EntityType::Object)
						handleTargets.push_back(entityNb->Id());
				}

				// if we have one or more handle targets, do the first
				if (handleTargets.size() >= 1)
				{
					auto& handledObject = *handleTargets[0].Entity();
					Handle(handledObject, *player);
				}
			}
		}

		void MainGameRender()
		{
			auto& gfx = Graphics::Instance();
			// Create the header the first time we're here
			static bool firstTime = true;
			if (firstTime)
			{
				firstTime = false;
				std::vector<glm::uvec4> bufferHeader;
				auto& gfx = Graphics::Instance();
				auto screenSize = gfx.ScreenSize();
				AddSeparatorLine(bufferHeader, 0, glm::vec4(1,1,1,1), screenSize.x, "The Tutorial Caverns");
				// Initialize the header buffer if needed
				auto& sparseBufferHeader = gfx.RequestBuffer("header");
				if (!sparseBufferHeader.IsInitialized())
					sparseBufferHeader.Init(sizeof(glm::uvec4), 200);
				sparseBufferHeader.Set(bufferHeader.size(), bufferHeader.data());
			}

			// For the main game view, we need all three elements: header, character info and game area
			gfx.RenderGame();
			gfx.RenderGui();
			gfx.RenderHeader();
		}
	}
}