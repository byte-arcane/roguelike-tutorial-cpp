#include "game.h"
#include "entity.h"
#include "graphics.h"
#include "utility.h"
#include "dungen.h"

namespace rlf
{
	Entity* GameState::GetEntity(const EntityId& entityId)
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

	EntityId GameState::CreateEntity(const DbIndex& cfg, const EntityDynamicConfig& dcfg)
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
		return entityId;
	}

	// Remove an entity from the game
	void GameState::RemoveEntity(const Entity& e)
	{
		invalidPoolIndices.insert(e.Id().id);
	}

	void GameState::SetCurrentLevelIndex(int iLevel)
	{
		currentLevelIndex = iLevel;
		if (levels.size() <= currentLevelIndex)
		{
			// singlegen_OpenCavern_v0.txt
#if 0
			auto levelConfig = LoadLevelFromTxtFile(rlf::mediaSearch("maps/map2.txt"));
			const auto& layout = levelConfig.first;
			const auto& entityConfigs = levelConfig.second;
#else
			auto layout = generateDungeon({ 64,32 });
			auto entityConfigs = populateDungeon(layout, 10, 5, 10, true, true);
#endif
			levels.push_back({});
			levels.back().Init(layout, entityConfigs, currentLevelIndex);
		}
	}

	void GameState::SetPlayer(const Entity& entity) 
	{ 
		playerId = entity.Id();
		CurrentLevel().UpdateFogOfWar();
		Graphics::Instance().CenterCameraAtPoint(entity.GetLocation().position);
		Graphics::Instance().OnGuiUpdated();
	}

	void GameState::WriteToMessageLog(const std::string& msg)
	{
		if (!messageLog.empty() && messageLog.back().first == msg)
			++messageLog.back().second;
		else
			messageLog.emplace_back(msg, 1);
		Graphics::Instance().OnGuiUpdated();
	}

	void GameState::ExecuteActionData()
	{
		// Execute the action
		ExecuteAction(actionData.type);
		// Store the action in history
		actionHistory.push_back(actionData);
		// tell the turn system that the player has played
		turnSystem.SetWaitingForPlayerAction(false);
		// process everybody else in the turn system
		turnSystem.Process();
		// reset the action data
		actionData = {};
	}
}