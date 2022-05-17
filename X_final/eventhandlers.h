#pragma once

#include <string>
#include <glm/glm.hpp>

namespace rlf
{
	// forward declarations
	class Entity;
	struct EntityId;
	struct EntityDynamicConfig;
	struct DbIndex;

	Entity& SpawnEntity(const DbIndex& cfg, const EntityDynamicConfig& dcfg);

	// entity moves 
	void Teleport(Entity& entity, const glm::ivec2& position);
	void MoveAdj(Entity& entity, const glm::ivec2& direction);
	//void EnterLevel(const Level& level, const glm::ivec2& position);
	//void Handle(Entity& handler, Entity& handled);

	void PickUpEverythingOrHandle(Entity& handler);
	void PickUp(Entity& handler, Entity& itemPile, const EntityId& itemId);
	void Drop(Entity& handler, const EntityId& itemId);

	glm::ivec4 AccumulateCombatStats(const Entity& creature);
	bool ModifyHp(Entity& entity, int hpMod);
	void AttackEntity(Entity& attacker, Entity& defender);
	void DestroyEntity(Entity& e);
	void ChangeLevel(int levelIndex);

	void ChangeEquippedItem(Entity& owner, int newEquippedIdx, int oldEquippedIdx);
	void UseItem(Entity& owner, int itemIdx);
}