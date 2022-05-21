#include "menu.h"

#include <gl/glew.h>
#include <GLFW/glfw3.h>

#include "input.h"
#include "utility.h"


#include "death.h"
#include "maingame.h"
#include "createchar.h"

#include "../eventhandlers.h"
#include "../graphics.h"
#include "../signals.h"
#include "../game.h"

namespace rlf
{
	namespace state
	{
		Status Menu::UpdateImpl()
		{
			if (Input::GetKeyDown(GLFW_KEY_ESCAPE))
				return Status::Abort;
			else if (Input::GetKeyDown(GLFW_KEY_1))
			{
				std::unique_ptr<State> newState(new CreateChar([&](bool success, const State* state) {
					if (success)
					{
						auto charName = static_cast<const CreateChar*>(state)->charName;
						StartNewGame(charName);
					}
					}));
				Game::Instance().PushState(newState);
			}
			else if (Input::GetKeyDown(GLFW_KEY_2))
			{
				// continue game
				ContinueGame();
				
			}
			else if (Input::GetKeyDown(GLFW_KEY_3))
			{
				// quit game
				option = Option::Exit;
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
			Game::Instance().New();

			// Set level
			ChangeLevel(0);

			std::unique_ptr<State> newState(new state::MainGame());
			Game::Instance().PushState(newState);

			// find suitable position (entry staircase)
			const auto& entities = Game::Instance().CurrentLevel().Entities();
			auto itFound = std::find_if(entities.begin(), entities.end(), [](const EntityId& entityId) {
				return entityId.Entity()->DbCfg() == DbIndex::StairsUp();
			});
			auto startPosition = itFound->Entity()->GetLocation().position;
			EntityDynamicConfig dcfg;
			dcfg.position = startPosition;
			dcfg.nameOverride = charName;
			// add one of each item
			for (const auto& kv : Db::Instance().All())
				if (kv.second.allowRandomSpawn && kv.second.type == EntityType::Item)
					dcfg.inventory.emplace_back(kv.first);
			DbIndex cfgdb{ "player" };
			auto player = Game::Instance().CreateEntity(cfgdb, dcfg, true).Entity();
			Game::Instance().SetPlayer(*player);
		}

		void Menu::ContinueGame()
		{
			Game::Instance().Load();
			std::unique_ptr<State> uptr(new state::MainGame());
			Game::Instance().PushState(uptr);
		}

		void Menu::Render()
		{
			// if we're about to change to the death state, don't Render anything. 
			// If this is removed, at death the screen flashes for a single instant, displaying the menu screen, before starting the death screen state
			if (changeToDeathState)
				return;

			auto& gfx = Graphics::Instance();
			auto& sparseBuffer = gfx.RequestBuffer("menu");
			if (!sparseBuffer.IsInitialized())
			{
				sparseBuffer.Init(sizeof(glm::uvec4), 4000);
				std::vector<glm::uvec4> buffer;
				auto text = readTextFile(mediaSearch("misc/titlescreen.txt"));
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
				AddSeparatorLine(buffer, row - 4, glm::vec4(1), screenSize.x, "   [1. New Game ]   ",' ');
				AddSeparatorLine(buffer, row - 5, glm::vec4(1), screenSize.x, "   [2. Continue ]   ", ' ');
				AddSeparatorLine(buffer, row - 6, glm::vec4(1), screenSize.x, "   [3.   Exit   ]   ", ' ');
				sparseBuffer.Set(buffer.size(), buffer.data());
			}
			gfx.RenderMenu(sparseBuffer);
		}
	}
}