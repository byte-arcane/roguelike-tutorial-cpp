#include "menu.h"

#include <gl/glew.h>
#include <GLFW/glfw3.h>

#include "input.h"
#include "utility.h"


#include "death.h"
#include "maingame.h"
#include "createchar.h"

#include "../commands.h"
#include "../graphics.h"
#include "../signals.h"
#include "../game.h"

namespace rlf
{
	namespace state
	{
		Status Menu::UpdateImpl()
		{
			// Escape exits the game
			if (Input::GetKeyDown(GLFW_KEY_ESCAPE))
				return Status::Abort;
			// 1 starts a new game
			else if (Input::GetKeyDown(GLFW_KEY_1))
			{
				std::unique_ptr<State> newState(new CreateChar([&](bool success, const State* state) {
					if (success)
					{
						const auto& charName = static_cast<const CreateChar*>(state)->CharName();
						StartNewGame(charName);
					}
					}));
				Game::Instance().PushState(newState);
			}
			// 2 loads a saved game from disk
			else if (Input::GetKeyDown(GLFW_KEY_2))
			{
				// continue game
				ContinueGame();
				
			}
			else if (Input::GetKeyDown(GLFW_KEY_3))
			{
				// quit game
				return Status::Success;
			}

			if (changeToDeathState)
			{
				changeToDeathState = false;
				std::unique_ptr<State> newState(new state::Death());
				Game::Instance().PushState(newState);
			}

			return Status::Running;
		}

		void Menu::StartListening()
		{
			sig::onPlayerDied.connect<Menu,&Menu::OnPlayerDied>(this);
		}
		void Menu::StopListening()
		{
			sig::onPlayerDied.disconnect<&Menu::OnPlayerDied>(this);
		}

		void Menu::OnPlayerDied()
		{
			changeToDeathState = true;
		}

		void Menu::StartNewGame(const std::string& charName)
		{
			// Change the state to main game
			std::unique_ptr<State> newState(new state::MainGame());
			Game::Instance().PushState(newState);

			// Initialize the game state
			Game::Instance().New();

			// set the level to first
			ChangeLevel(0);

			// find suitable position for the player (entry staircase)
			const auto& entities = Game::Instance().CurrentLevel().Entities();
			auto itFound = std::find_if(entities.begin(), entities.end(), [](const EntityId& entityId) {
				return entityId.Entity()->DbCfg() == DbIndex::StairsUp();
			});
			auto startPosition = itFound->Entity()->GetLocation().position;

			// Create the player entity
			EntityDynamicConfig dcfg;
			dcfg.position = startPosition;
			dcfg.nameOverride = charName;
			// add one of each item, for debugging purposes!
			for (const auto& kv : Db::Instance().All())
				if (kv.second.allowRandomSpawn && kv.second.type == EntityType::Item)
					dcfg.inventory.emplace_back(kv.first);
			DbIndex cfgdb{ "player" };
			auto player = Game::Instance().CreateEntity(cfgdb, dcfg, true).Entity();
			Game::Instance().SetPlayer(*player);
		}

		void Menu::ContinueGame()
		{
			if (Game::Instance().Load())
			{
				std::unique_ptr<State> uptr(new state::MainGame());
				Game::Instance().PushState(uptr);
			}
		}

		void Menu::Render()
		{
			// if we're about to change to the death state, don't Render anything. 
			// If this is removed, at death the screen flashes for a single instant, displaying the menu screen, before starting the death screen state
			if (changeToDeathState)
				return;

			auto& gfx = Graphics::Instance();
			// Get a buffer to write to 
			auto& sparseBuffer = gfx.RequestBuffer("menu");
			if (!sparseBuffer.IsInitialized())
			{
				// Build the cpu data buffer
				std::vector<glm::uvec4> buffer;

				// Add a title screen from file
				auto text = ReadTextFile(MediaSearch("misc/titlescreen.txt"));
				buffer.reserve(text.size());
				auto screenSize = gfx.ScreenSize();
				int row = screenSize.y - 1;
				int col = 0;
				for (const auto& c : text)
				{
					if (c == '\n')
					{
						--row;
						col = 0;
					}
					else
					{
						buffer.push_back(TileData(c, glm::vec4(1)).PackSparse({ col,row }));
						++col;
					}
				}
				// Add a few lines after that
				AddSeparatorLine(buffer, row - 4, glm::vec4(1), screenSize.x, "   [1. New Game ]   ",' ');
				AddSeparatorLine(buffer, row - 5, glm::vec4(1), screenSize.x, "   [2. Continue ]   ", ' ');
				AddSeparatorLine(buffer, row - 6, glm::vec4(1), screenSize.x, "   [3.   Exit   ]   ", ' ');
				// Set the gpu data
				sparseBuffer.Init(sizeof(glm::uvec4), 4000);
				sparseBuffer.Set(buffer.size(), buffer.data());
			}
			gfx.RenderMenu(sparseBuffer);
		}
	}
}