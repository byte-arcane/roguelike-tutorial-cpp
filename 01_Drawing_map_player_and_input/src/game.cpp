#include "game.h"

#include <gl/glew.h>
#include <GLFW/glfw3.h>

#include <input.h>
#include <utility.h>

#include "graphics.h"
#include "signals.h"

#include "state/maingame.h"

namespace rlf
{
	void Game::Init()
	{
		state::MainGameStart();
	}

	void Game::EnterLevel()
	{
		Array2D<LevelBgElement> layout;
		bg = LoadLevelFromTxtFile(rlf::MediaSearch("maps/starting_map.txt"));	
	}

	// Render the current game state
	void Game::RenderCurrentState()
	{
		Graphics::Instance().BeginRender();
		state::MainGameRender();
		Graphics::Instance().EndRender();
	}

	// Update the current game state
	void Game::UpdateCurrentState()
	{
		
		// Ctrl-L reloads all shaders
		if (Input::GetKeyDown(GLFW_KEY_L) && Input::GetKeyDown(GLFW_KEY_LEFT_CONTROL))
			Graphics::Instance().ReloadShaders();

		state::MainGameUpdate();
	}
}