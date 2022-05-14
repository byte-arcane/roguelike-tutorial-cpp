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

using namespace glm;
using namespace std;
using namespace rlf;

class FrameworkAppBasics : public cgf::FrameworkApp
{
	Level level;

	int playerBufSlot = -1;

	// Put here any initialisation code. Happens once, before the main loop and after initialisation of GLFW/GLEW/ImGui
	void onInit() override
	{
		Graphics::Instance().Init();

		// Set level
		ChangeLevel(0);

		// Spawn player
		EntityConfig cfg{ EntityType::Creature, {TileData{'@',vec4(1,1,1,1)}} };
		Db::Instance().Add("player", cfg);
		EntityDynamicConfig dcfg;
		dcfg.position = ivec2(17, 43);
		dcfg.nameOverride = "Sir Carpaccio";
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
		Graphics::Instance().BeginRender();
		Graphics::Instance().RenderLevel();
		Graphics::Instance().RenderGui();
		Graphics::Instance().EndRender();
	}

	// Put here any update related code. Called before render
	void onUpdate() override
	{
		auto player = GameState::Instance().Player().Entity();
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
			if (direction != glm::ivec2(0,0))
			{
				MoveAdj(*player, direction);
			}

			if (cgf::Input::GetKeyDown(GLFW_KEY_ENTER))
			{
				HandleOnGround(*player);
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
	FrameworkAppBasics app;
	return app.run();
}