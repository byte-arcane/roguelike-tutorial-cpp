#include "entityid.h"
#include "game.h"

namespace rlf
{
	Entity* EntityId::Entity() const
	{
		// ask gamestate to do it
		return Game::Instance().GetEntity(*this);
	}
}