#include "maingame.h"

#include <GLFW/glfw3.h>
#include "input.h"

#include "../game.h"
#include "../graphics.h"
#include "../eventhandlers.h"
#include "../grid.h"

#include "framework.h"
#include "inventory.h"
#include "selecttarget.h"

namespace rlf
{
	namespace state
	{
		Status MainGame::updateImpl(StateStack& stateStack)
		{
			// Too easy to accidentally exit the game, so avoid unless we implement quick-save
			//if (Input::GetKeyDown(GLFW_KEY_ESCAPE))
			//	return true;

			auto& g = GameState::Instance();
			auto player = GameState::Instance().Player().Entity();

			if (player != nullptr)
			{
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
					GameState::Instance().EndTurn();
				}

				if (Input::GetKeyDown(GLFW_KEY_ENTER))
				{
					// Gather potential candidates for handling: on the ground or in adjacent squares
					
					std::vector<EntityId> handleTargets;
					// check our feet for anything to interact with that is NOT an item pile (need to use "pick up" command)
					auto entityOnGround = g.CurrentLevel().GetEntity(playerPos, false);
					if (entityOnGround != nullptr && entityOnGround->GetInventory() == nullptr)
						handleTargets.push_back(entityOnGround->Id());

					for (const auto& nb4 : Nb4())
					{
						auto pnb = playerPos + nb4;
						auto entityNb = g.CurrentLevel().GetEntity(pnb, true);
						if (entityNb != nullptr && entityNb->Type() == EntityType::Object)
							handleTargets.push_back(entityNb->Id());
					}

					// if we have one handle target, do it
					if (handleTargets.size() == 1)
					{
						auto& handledObject = *handleTargets[0].Entity();
						handledObject.GetObjectData()->Handle(handledObject, *player);
						GameState::Instance().EndTurn();
					}
					//if we have > 1 handle targets, start targetting state
					else if (handleTargets.size() > 1)
					{
						std::vector<glm::ivec2> handlePositions;
						for (const auto& ht : handleTargets)
							handlePositions.push_back(ht.Entity()->GetLocation().position);
						stateStack.emplace_back(new state::SelectTarget(handlePositions, [handleTargets](bool success, const State* state) {
							// if success, handle that target and end turn
							if (success)
							{
								auto player = GameState::Instance().Player().Entity();
								auto targetIndex = static_cast<const state::SelectTarget*>(state)->targetIndex;
								auto& handledObject = *handleTargets[targetIndex].Entity();
								handledObject.GetObjectData()->Handle(handledObject, *player);
								GameState::Instance().EndTurn();
							}
						}));
					}
				}

				if (Input::GetKeyDown(GLFW_KEY_T))
				{
					// if we have a ranged weapon equipped...
					auto equippedWeaponIdx = player->GetInventory()->EquippedItemAtSlot(ItemCategory::Weapon);
					if (equippedWeaponIdx >= 0)
					{
						auto weapon = player->GetInventory()->items[equippedWeaponIdx].Entity();
						int attackRange = weapon->DbCfg().Cfg()->itemCfg.attackRange;
						if (attackRange > 1)
						{
							// ...and there are >0 valid targets. How to find them?
							std::vector<glm::ivec2> validTargetPositions;
							// Gather all points in the attack range
							Circle(validTargetPositions, playerPos, attackRange, false);
							// Erase-remove idiom, removing elements that fail to satisfy the condition for containing a valid target
							validTargetPositions.erase(std::remove_if(validTargetPositions.begin(), validTargetPositions.end(), [&](const glm::ivec2& p) {
								// If not currently visible, not valid
								const auto& level = g.CurrentLevel();
								if (!level.FogOfWar().InBounds(p) || level.FogOfWar()(p.x, p.y) != FogOfWarStatus::Visible)
									return true;
								// If not a creature, or is player, not valid
								auto entity = level.GetEntity(p, true);
								if (entity == nullptr || entity->Type() != EntityType::Creature || entity == player)
									return true;
								return false;
							}), validTargetPositions.end());
							
							// start the targetting state
							if (!validTargetPositions.empty())
							{
								auto& projPath = projectilePath;
								auto& projFireTime = projectileFireTime;
								stateStack.emplace_back(new state::SelectTarget(validTargetPositions, [validTargetPositions,&projPath,&projFireTime](bool success, const State* state) {
									// if success, handle that target and end turn
									if (success)
									{
										auto& g = GameState::Instance();
										auto targetIndex = static_cast<const state::SelectTarget*>(state)->targetIndex;
										auto targetPosition = validTargetPositions[targetIndex];
										auto target = g.CurrentLevel().GetEntity(targetPosition, true);
										assert(target != nullptr);
										auto player = g.Player().Entity();
										AttackEntity(*player, *target);
										Line(projPath, player->GetLocation().position, targetPosition);
										projFireTime = FrameworkApp::Time();
										GameState::Instance().EndTurn();
									}
									}));
							}
						}
					}
				}

				if (Input::GetKeyDown(GLFW_KEY_P))
				{
					auto itemPile = g.CurrentLevel().GetEntity(playerPos, false);
					if (itemPile != nullptr && itemPile->DbCfg() == DbIndex::ItemPile())
						stateStack.emplace_back(new Inventory(Inventory::Mode::PickUp, {}));
				}
				if (Input::GetKeyDown(GLFW_KEY_D))
					stateStack.emplace_back(new Inventory(Inventory::Mode::Drop, {}));
				if (Input::GetKeyDown(GLFW_KEY_E))
					stateStack.emplace_back(new Inventory(Inventory::Mode::EquipOrUse, {}));

#if 0 // Debugging
				if (Input::GetKey(GLFW_KEY_LEFT_CONTROL))
				{
					auto tgt = Graphics::Instance().MouseCursorTile();
					auto path = GameState::Instance().CurrentLevel().CalcPath(*player, tgt);
					//std::vector<ivec2> path; Square(path, player->GetLocation().position, 2,false);
					Graphics::Instance().SetHighlightedTiles(path);
				}
				else
					Graphics::Instance().SetHighlightedTiles({});
#endif
			}

			return Status::Running;
		}

		void MainGame::render()
		{
			auto& gfx = Graphics::Instance();
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
			gfx.RenderGame();
			gfx.RenderGui();
			gfx.RenderHeader();

			// render a projectile. since this is the only effect, this is VERY inefficient, and the render pass would be better utilised if more things were rendered
			if (!projectilePath.empty())
			{
				auto time = FrameworkApp::Time() - projectileFireTime;
				const float PROJECTILE_SPEED = 50.0f; // 10 tiles per second
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