#include <algorithm>
#include <memory>

#include <GL/glew.h>
#include <imgui.h>
#include <glm/glm.hpp>
#include <glfw/glfw3.h>

#include <framework.h>
#include <utility.h>
#include <input.h>

#include "graphics.h"
#include "level.h"
#include "game.h"
#include "eventhandlers.h"
#include "astar.h"
#include "grid.h"
#include "inventory.h"
#include "state/state.h"

using namespace glm;
using namespace std;
using namespace rlf;

enum class GameMode
{
	InGame = 0,
	Targeting,
	Inventory_EquipOrUse,
	Inventory_PickUp,
	Inventory_Drop,
	Menu
};

class Game : public cgf::FrameworkApp
{
	std::vector<std::unique_ptr<state::IState>> gameStates;

	GameMode gameMode = GameMode::InGame;
	int inventoryModePageIndex = 0;
	glm::ivec2 targetPosition = { -1,-1 }; // for targetting mode

	// Put here any initialisation code. Happens once, before the main loop and after initialisation of GLFW/GLEW/ImGui
	void onInit() override
	{
		// we can map this to a key for dynamic database reload!
		Db::Instance().LoadFromDisk();

		Graphics::Instance().Init();

		// Set level
		ChangeLevel(0);

		EntityDynamicConfig dcfg;
		dcfg.position = ivec2(4, 4);
		dcfg.nameOverride = "Sir Carpaccio";
		// add one of each item
		for (const auto& kv : Db::Instance().All())
			if (kv.second.allowRandomSpawn && kv.second.type == EntityType::Item)
				dcfg.inventory.emplace_back(kv.first);
		DbIndex cfgdb{ "player" };
		auto& player = SpawnEntity(cfgdb,dcfg);
		GameState::Instance().SetPlayer(player);
	}

	// Put here any termination code. Happens once, before termination of GLFW/GLEW/ImGui
	void onTerminate() override
	{
		Graphics::Instance().Dispose();
	}

	// Put here any rendering code. Called every frame
	void onRender() override
	{
		auto& g = Graphics::Instance();
		g.BeginRender();
		switch (gameMode)
		{
		case GameMode::InGame:
			g.RenderLevel();
		case GameMode::Inventory_EquipOrUse:
		case GameMode::Inventory_PickUp:
		case GameMode::Inventory_Drop:
			g.RenderGui();
			break;
		default:
			break;
		}
		
		g.EndRender();
	}

	// Put here any update related code. Called before render
	void onUpdate() override
	{
		auto& g = GameState::Instance();
		auto player = GameState::Instance().Player().Entity();
		if (gameMode == GameMode::InGame)
		{
			if (player != nullptr)
			{
				auto playerPos = player->GetLocation().position;
				glm::ivec2 direction{ 0,0 };
				auto oldPlayerPos = playerPos;
				if (cgf::Input::GetKeyDown(GLFW_KEY_LEFT))
					direction.x -= 1;
				if (cgf::Input::GetKeyDown(GLFW_KEY_RIGHT))
					direction.x += 1;
				if (cgf::Input::GetKeyDown(GLFW_KEY_UP))
					direction.y += 1;
				if (cgf::Input::GetKeyDown(GLFW_KEY_DOWN))
					direction.y -= 1;
				if (direction != glm::ivec2(0, 0))
				{
					MoveAdj(*player, direction);
					g.GetTurnSystem().SetWaitingForPlayerAction(false);
				}

				if (cgf::Input::GetKeyDown(GLFW_KEY_ENTER))
				{
					PickUpEverythingOrHandle(*player);
					g.GetTurnSystem().SetWaitingForPlayerAction(false);
				}

				if (cgf::Input::GetKeyDown(GLFW_KEY_P))
				{
					auto itemPile = g.CurrentLevel().GetEntity(playerPos, false);
					if (itemPile != nullptr && itemPile->DbCfg() == DbIndex::ItemPile())
					{
						gameMode = GameMode::Inventory_PickUp;
						inventoryModePageIndex = 0;
						Graphics::Instance().BuildInventoryData(inventoryModePageIndex, InventoryMode::PickUpOrDrop, *itemPile);
					}
				}
				if (cgf::Input::GetKeyDown(GLFW_KEY_D))
				{
					gameMode = GameMode::Inventory_Drop;
					inventoryModePageIndex = 0;
					Graphics::Instance().BuildInventoryData(inventoryModePageIndex, InventoryMode::PickUpOrDrop, *player);
				}
				if (cgf::Input::GetKeyDown(GLFW_KEY_E))
				{
					gameMode = GameMode::Inventory_EquipOrUse;
					inventoryModePageIndex = 0;
					Graphics::Instance().BuildInventoryData(inventoryModePageIndex, InventoryMode::EquipOrUse, *player);
				}

#if 0 // Debugging
				if (cgf::Input::GetKey(GLFW_KEY_LEFT_CONTROL))
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
		}
		else if (gameMode == GameMode::Targeting)
		{
#if 0
			bool confirmed = targetingHandleInput(targetPosition);
			if (confirmed)
			{
				gameMode = GameMode::InGame;
				// TODO: 
			}
#endif
		}
		else if (gameMode == GameMode::Inventory_EquipOrUse)
		{
			if (inventoryHandleInput(inventoryModePageIndex, InventoryMode::EquipOrUse, *player))
			{
				if (inventoryModePageIndex >= 0)
					Graphics::Instance().BuildInventoryData(inventoryModePageIndex, InventoryMode::EquipOrUse, *player);
				else
				{
					gameMode = GameMode::InGame;
					Graphics::Instance().ClearInventoryData();
				}
			}
			
		}
		else if (gameMode == GameMode::Inventory_PickUp)
		{
			auto itemPile = g.CurrentLevel().GetEntity(player->GetLocation().position, false);
			if (itemPile == nullptr || inventoryHandleInput(inventoryModePageIndex, InventoryMode::PickUpOrDrop, *itemPile))
			{
				if (inventoryModePageIndex >= 0 && itemPile != nullptr)
					Graphics::Instance().BuildInventoryData(inventoryModePageIndex, InventoryMode::PickUpOrDrop, *itemPile);
				else
				{
					gameMode = GameMode::InGame;
					Graphics::Instance().ClearInventoryData();
				}
			}
		}
		else if (gameMode == GameMode::Inventory_Drop)
		{
			if(inventoryHandleInput(inventoryModePageIndex, InventoryMode::PickUpOrDrop, *player))
			{
				if (inventoryModePageIndex >= 0)
					Graphics::Instance().BuildInventoryData(inventoryModePageIndex, InventoryMode::PickUpOrDrop, *player);
				else
				{
					gameMode = GameMode::InGame;
					Graphics::Instance().ClearInventoryData();
				}
			}
		}
		
	}

	// Put here any GUI related code. Called after render
	void onGui() override
	{
	}
};

int main(int argc, char** argv)
{
	Game game;
	return game.run();
}