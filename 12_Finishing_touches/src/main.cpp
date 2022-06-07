#include <framework.h>

#include <imgui.h>

#include "graphics.h"
#include "game.h"
#include "db.h"

using namespace rlf;

class GameApp : public rlf::FrameworkApp
{
	// Put here any initialisation code. Happens once, before the main loop and after initialisation of GLFW/GLEW/ImGui
	void onInit() override
	{
		// we can map this to a key for dynamic database reload!
		Db::Instance().LoadFromDisk();
		Graphics::Instance().Init();
		Game::Instance().Init();
	}

	// Put here any termination code. Happens once, before termination of GLFW/GLEW/ImGui
	void onTerminate() override
	{
		Graphics::Instance().Dispose();
	}

	// Put here any rendering code. Called every frame
	void onRender() override
	{
		Game::Instance().RenderCurrentState();
	}

	// Put here any Update related code. Called before Render
	void onUpdate() override
	{
		Game::Instance().UpdateCurrentState();
	}

	// Put here any GUI related code. Called after Render
	void onGui() override
	{
		// A simple example: reload the database dynamically
		if (ImGui::Button("Reload DB"))
			Db::Instance().LoadFromDisk();
	}
};

int main(int argc, char** argv)
{
	GameApp game;
	return game.run();
}