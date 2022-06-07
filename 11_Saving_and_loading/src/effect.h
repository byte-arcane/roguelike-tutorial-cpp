#pragma once

namespace rlf
{
	class Entity;

	// Effects, for consumables
	enum class Effect
	{
		MinorHeal=0,
		Heal,
		MajorHeal,
		MinorDamage,
	};

	// Apply a given effect to an entity
	void ApplyEffect(Entity& entity, Effect effect);
}