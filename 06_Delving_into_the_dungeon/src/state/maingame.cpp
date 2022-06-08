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
		Status MainGame::UpdateImpl()
		{
			// Quick save/load
			auto& g = Game::Instance();

			auto player = Game::Instance().PlayerId().Entity();
			if (player == nullptr)
				return Status::Abort;

			// End the game state if player is dead, so that we can move to the death screen
			if (player->GetCreatureData()->hp <= 0)
				return Status::Success;

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

			return Status::Running;
		}

		void MainGame::Render()
		{
			auto& gfx = Graphics::Instance();
			// Create the header if needed
			if (isHeaderDirty)
			{
				isHeaderDirty = false;
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

			// Render a projectile. since this is the only effect, this is VERY inefficient, and the Render pass would be better utilised if more things were rendered
			if (!projectilePath.empty())
			{
				auto time = FrameworkApp::Time() - projectileFireTime;
				const float PROJECTILE_SPEED = 50.0f; // 50 tiles per second
				auto ptIdx = int(PROJECTILE_SPEED * time);
				if (ptIdx < projectilePath.size())
				{
					auto& sparseBufferFx = gfx.RequestBuffer("fx");
					if (!sparseBufferFx.IsInitialized())
						sparseBufferFx.Init(sizeof(glm::uvec4), 200);
					auto bufferData = TileData('*',glm::vec4(1,1,1,1)).PackSparse(gfx.WorldToScreen(projectilePath[ptIdx]));
					sparseBufferFx.Set(1, &bufferData);
					Graphics::Instance().RenderGameOverlay(sparseBufferFx);
				}
				else
					projectilePath.clear();
			}
		}
	}
}