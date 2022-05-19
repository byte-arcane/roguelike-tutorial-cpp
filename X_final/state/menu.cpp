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
		Status Menu::updateImpl(StateStack& stateStack)
		{
			if (Input::GetKeyDown(GLFW_KEY_ESCAPE))
				return Status::Abort;
			else if (Input::GetKeyDown(GLFW_KEY_1))
			{
				std::unique_ptr<State> uptr(new CreateChar([&](bool success, const State* state) {
					if (success)
					{
						auto charName = static_cast<const CreateChar*>(state)->charName;
						startNewGame(charName, stateStack);
					}
					}));
				pushToStack(stateStack, uptr);
			}
			else if (Input::GetKeyDown(GLFW_KEY_2))
			{
				// continue game
				continueGame(stateStack);
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
				stateStack.emplace_back(new Death());
			}

			return Status::Running;
		}

		void Menu::startListening()
		{
			sig::onPlayerDied.connect<Menu,&Menu::onPlayerDied>(this);
		}
		void Menu::stopListening()
		{
			sig::onPlayerDied.disconnect<&Menu::onPlayerDied>(this);
		}

		void Menu::onPlayerDied()
		{
			changeToDeathState = true;
		}

		void Menu::startNewGame(const std::string& charName, StateStack& stateStack)
		{
			GameState::Instance() = GameState();

			// Set level
			ChangeLevel(0);

			stateStack.emplace_back(new state::MainGame());

			// find suitable position (entry staircase)
			const auto& entities = GameState::Instance().CurrentLevel().Entities();
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
			auto& player = SpawnEntity(cfgdb, dcfg);
			GameState::Instance().SetPlayer(player);
		}

		void Menu::continueGame(StateStack& stateStack)
		{
			assert(false);
		}

		void Menu::render()
		{
			// if we're about to change to the death state, don't render anything. 
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
				addSeparatorLine(buffer, row - 4, glm::vec4(1), screenSize.x, "   [1. New Game ]   ",' ');
				addSeparatorLine(buffer, row - 5, glm::vec4(1), screenSize.x, "   [2. Continue ]   ", ' ');
				addSeparatorLine(buffer, row - 6, glm::vec4(1), screenSize.x, "   [3.   Exit   ]   ", ' ');
				sparseBuffer.Set(buffer.size(), buffer.data());
			}
			gfx.RenderMenu(sparseBuffer);
		}
	}
}