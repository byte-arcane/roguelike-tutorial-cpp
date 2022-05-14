#include "entityid.h"
#include "game.h"

namespace rlf
{
	Entity* EntityId::Entity() const
	{
		// ask gamestate to do it
		return GameState::Instance().GetEntity(*this);
	}
}