#include "game.h"
#include "entity.h"
#include "graphics.h"

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
		if (entity.Id().version != entityId.version)
			return nullptr;
		return &entity;
	}

	EntityId GameState::CreateEntity(const DbIndex& cfg, const EntityDynamicConfig& dcfg)
	{
		EntityId entityId;
		if (!invalidPoolIndices.empty())
		{
			auto it = invalidPoolIndices.begin();
			entityId.version = poolEntities[*it].Id().version + 1;
			entityId.id = *it;
			invalidPoolIndices.erase(it);
		}
		else
		{
			entityId.id = poolEntities.size();
			entityId.version = 1;
			poolEntities.push_back({});
		}
		poolEntities[entityId.id].Initialize(entityId,cfg, dcfg);
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
			auto levelConfig = LoadLevelFromTxtFile("D:\\Games\\REXPaint-v1.50\\images\\singlegen_OpenCavern_v0.txt");
			levels.push_back({});
			levels.back().Init(levelConfig.first, levelConfig.second, currentLevelIndex);
		}
	}

	void GameState::SetPlayer(const Entity& entity) 
	{ 
		playerId = entity.Id();
		CurrentLevel().UpdateFogOfWar();
		Graphics::Instance().CenterCameraAtPoint(entity.GetLocation().position);
		Graphics::Instance().OnGuiUpdated();
	}
}