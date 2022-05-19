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
#include "state/maingame.h"
#include "state/menu.h"

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

class Game : public rlf::FrameworkApp
{
	std::vector<std::unique_ptr<state::State>> gameStates;

	GameMode gameMode = GameMode::InGame;
	int inventoryModePageIndex = 0;
	glm::ivec2 targetPosition = { -1,-1 }; // for targetting mode

	// Put here any initialisation code. Happens once, before the main loop and after initialisation of GLFW/GLEW/ImGui
	void onInit() override
	{
		// we can map this to a key for dynamic database reload!
		Db::Instance().LoadFromDisk();
		Graphics::Instance().Init();
		std::unique_ptr<state::State> menu = std::make_unique<state::Menu>();
		state::State::pushToStack(gameStates, menu);
	}

	// Put here any termination code. Happens once, before termination of GLFW/GLEW/ImGui
	void onTerminate() override
	{
		Graphics::Instance().Dispose();
	}

	// Put here any rendering code. Called every frame
	void onRender() override
	{
		if (!gameStates.empty())
		{
			Graphics::Instance().BeginRender();
			state::State::renderStack(gameStates);
			Graphics::Instance().EndRender();
		}
	}

	// Put here any update related code. Called before render
	void onUpdate() override
	{
		state::State::updateStack(gameStates);
		if(gameStates.empty())
			exit(0); // Be nicer!
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