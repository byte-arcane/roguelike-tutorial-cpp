#include "game.h"

#include <gl/glew.h>
#include <GLFW/glfw3.h>

#include <input.h>
#include <utility.h>

#include "entity.h"
#include "graphics.h"
#include "signals.h"

#include "state/maingame.h"

namespace rlf
{
	void Game::Init()
	{
		state::MainGameStart();
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
		// ok, all good, get the entity
		return entity.get();
	}

	EntityId Game::CreateEntity(const DbIndex& cfg, const EntityDynamicConfig& dcfg, bool fireMessage)
	{
		// Get an appropriate entity id. 
		EntityId entityId;
		// if we have some invalid ones
		if (!invalidPoolIndices.empty())
		{
			// Get the first invalid one and increment the version
			auto it = invalidPoolIndices.begin();
			entityId.version = poolEntities[*it]->Id().version + 1;
			entityId.id = *it;
			invalidPoolIndices.erase(it);
		}
		// no invalid ones
		else
		{
			// create a new one at the end of the pool
			entityId.id = poolEntities.size();
			entityId.version = 1;
			poolEntities.resize(poolEntities.size() + 1);
		}
		// create a new entity and initialize it
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

	void Game::EnterLevel()
	{
		Array2D<LevelBgElement> layout;
		std::vector<std::pair<DbIndex, EntityDynamicConfig>> entityConfigs;
#if 1
		auto levelConfig = LoadLevelFromTxtFile(rlf::MediaSearch("maps/starting_map.txt"));
		layout = std::move(levelConfig.first);
		entityConfigs = std::move(levelConfig.second);
#else
		auto numMonsters = 5 + iLevel;
		auto numTreasures = 5 + iLevel;
		auto numFeatures = glm::min(1 + iLevel, 10);
		layout = GenerateDungeon({ 64,32 });
		entityConfigs = PopulateDungeon(layout, numMonsters, numFeatures, numTreasures, true, true);
#endif
		level.Init(layout, entityConfigs, 0);
		level.StartListening();
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
		// if this message is the same as the last one, increment the number of repeats of the last entry
		if (!messageLog.empty() && messageLog.back().first == msg)
			++messageLog.back().second;
		// otherwise add it to the list
		else
			messageLog.emplace_back(msg, 1);
		sig::onGuiUpdated.fire();
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