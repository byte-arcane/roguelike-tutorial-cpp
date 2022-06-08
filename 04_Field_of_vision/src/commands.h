#pragma once

#include <string>
#include <glm/glm.hpp>

namespace rlf
{
	// forward declarations
	class Entity;
	struct EntityId;

	// Teleport entity to target position, if possible
	void Teleport(Entity& entity, const glm::ivec2& position);
	// Move entity towards an adjacent direction, if possible. Apply contextual actions (e.g. open a door if closed, or attack a creature)
	void MoveAdj(Entity& entity, const glm::ivec2& direction);
	// Handle something that's on the floor (e.g. stairs)
	void HandleOnFloor(Entity& handler);
	// Handle an object
	void Handle(Entity& handled, Entity& handler);
	// Modify a creature's hitpoints by a specific amount
	bool ModifyHp(Entity& entity, int hpMod);
	// Attack a target entity using equipped weapons (or "natural" weapons if nothing is equipped)
	void AttackEntity(Entity& attacker, Entity& defender);
	// Destroy an entity and remove it from the game
	void DestroyEntity(Entity& e);
	// Change to the given level index
	void EnterLevel();
}