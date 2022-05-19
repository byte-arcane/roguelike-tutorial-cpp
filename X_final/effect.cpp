#include "effect.h"
#include "entity.h"
#include "game.h"

#include <glm/glm.hpp>
#include <fmt/format.h>

#include "eventhandlers.h"

namespace rlf
{
	void ApplyEffect(Entity& entity, Effect effect)
	{
		auto& g = GameState::Instance();
		switch (effect)
		{
		case rlf::Effect::MinorHeal:
			ModifyHp(entity, 1);
			break;
		case rlf::Effect::Heal:
			ModifyHp(entity, 4);
			break;
		case rlf::Effect::MajorHeal:
			ModifyHp(entity, 10);
			break;
		case rlf::Effect::MinorDamage:
			ModifyHp(entity, -1);
			break;
		default:
			assert(false); // Need to implement the effect!
			break;
		}
	}
}