#include "game.h"

#include <gl/glew.h>
#include <GLFW/glfw3.h>

#include <input.h>
#include <utility.h>

#include "entity.h"
#include "graphics.h"
#include "dungen.h"
#include "signals.h"
#include "state/menu.h"

namespace rlf
{
	void Game::Init()
	{
		std::unique_ptr<state::State> menu = std::make_unique<state::Menu>();
		PushState(menu);
	}

	Entity* Game::GetEntity(const EntityId& entityId)
	{
		// is it in invalid list?
		if (invalidPoolIndices.find(entityId.id) != invalidPoolIndices.end())
			return nullptr;
		// out of bounds?
		if (entityId.id < 0 || entityId.id >= poolEntities.size())
			return nullptr;
		auto& entity = poolEntities[entityId.id];
		// is it an old version?
		if (!entity || entity->Id().version != entityId.version)
			return nullptr;
		return entity.get();
	}

	EntityId Game::CreateEntity(const DbIndex& cfg, const EntityDynamicConfig& dcfg, bool fireMessage)
	{
		EntityId entityId;
		if (!invalidPoolIndices.empty())
		{
			auto it = invalidPoolIndices.begin();
			entityId.version = poolEntities[*it]->Id().version + 1;
			entityId.id = *it;
			invalidPoolIndices.erase(it);
		}
		else
		{
			entityId.id = poolEntities.size();
			entityId.version = 1;
			poolEntities.resize(poolEntities.size() + 1);
		}
		poolEntities[entityId.id].reset(new Entity());
		poolEntities[entityId.id]->Initialize(entityId,cfg, dcfg);

		if (fireMessage)
			sig::onEntityAdded.fire(*entityId.Entity());

		return entityId;
	}

	// Remove an entity from the game
	void Game::RemoveEntity(const Entity& e)
	{
		invalidPoolIndices.insert(e.Id().id);
	}

	void Game::ChangeLevel(int iLevel)
	{
		if (currentLevelIndex >= 0)
			levels[currentLevelIndex].StopListening();
		currentLevelIndex = iLevel;
		if (levels.size() <= currentLevelIndex)
		{
			// singlegen_OpenCavern_v0.txt
#if 0
			auto levelConfig = LoadLevelFromTxtFile(rlf::mediaSearch("maps/map2.txt"));
			const auto& layout = levelConfig.first;
			const auto& entityConfigs = levelConfig.second;
#else
			auto numMonsters = 5 + iLevel;
			auto numTreasures = 5 + iLevel;
			auto numFeatures = glm::min(1 + iLevel, 10);
			auto layout = GenerateDungeon({ 64,32 });
			auto entityConfigs = PopulateDungeon(layout, numMonsters, numFeatures, numTreasures, true, true);
#endif
			levels.push_back({});
			levels.back().Init(layout, entityConfigs, currentLevelIndex);
		}
		levels[iLevel].StartListening();
	}

	void Game::SetPlayer(const Entity& entity) 
	{ 
		playerId = entity.Id();
		CurrentLevel().UpdateFogOfWar();
		Graphics::Instance().CenterCameraAtPoint(entity.GetLocation().position);
		sig::onGuiUpdated.fire();
	}

	void Game::WriteToMessageLog(const std::string& msg)
	{
		if (!messageLog.empty() && messageLog.back().first == msg)
			++messageLog.back().second;
		else
			messageLog.emplace_back(msg, 1);
		sig::onGuiUpdated.fire();
	}

	void Game::EndTurn()
	{
		// tell the turn system that the player has played
		turnSystem.SetWaitingForPlayerAction(false);
		// process everybody else in the turn system
		turnSystem.Process();
	}

	// Render the current game state
	void Game::RenderCurrentState()
	{
		if (!gameStates.empty())
		{
			Graphics::Instance().BeginRender();
			gameStates.back()->Render();
			Graphics::Instance().EndRender();
		}
	}

	// Update the current game state
	void Game::UpdateCurrentState()
	{
		
		// Ctrl-L reloads all shaders
		if (Input::GetKeyDown(GLFW_KEY_L) && Input::GetKeyDown(GLFW_KEY_LEFT_CONTROL))
			Graphics::Instance().ReloadShaders();

		if (!gameStates.empty())
			gameStates.back()->Update(gameStates);
		else
			exit(0); // Be nicer!
	}

	void Game::PushState(std::unique_ptr<state::State>& state)
	{
		gameStates.push_back(std::move(state));
		gameStates.back()->StartListening();
	}
}