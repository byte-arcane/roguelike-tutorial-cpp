#pragma once

namespace rlf
{
	class Entity;

	enum class Effect
	{
		MinorHeal=0,
		Heal,
		MajorHeal,
		MinorDamage,
	};

	void ApplyEffect(Entity& entity, Effect effect);
}