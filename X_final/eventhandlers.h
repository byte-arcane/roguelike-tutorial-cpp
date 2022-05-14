#pragma once

#include <string>
#include <glm/glm.hpp>

namespace rlf
{
	// forward declarations
	class Entity;
	struct EntityDynamicConfig;
	struct DbIndex;

	Entity& SpawnEntity(const DbIndex& cfg, const EntityDynamicConfig& dcfg);

	// entity moves 
	void Move(Entity& entity, const glm::ivec2& position);
	void MoveAdj(Entity& entity, const glm::ivec2& direction);
	//void EnterLevel(const Level& level, const glm::ivec2& position);
	//void Handle(Entity& handler, Entity& handled);

	void HandleOnGround(Entity& handler);

	void DestroyEntity(Entity& e);
	void ChangeLevel(int levelIndex);
}