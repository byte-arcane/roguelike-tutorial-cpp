#include "turn.h"
#include "entity.h"

#include "game.h"
#include "commands.h"

namespace rlf
{
	void TurnSystem::Process()
	{
		// Don't process anything anymore if player is dead. Also don't process anything if we're waiting for some player UI action (select a target, etc)
		auto player = Game::Instance().PlayerId().Entity();
		if (player == nullptr || waitingForPlayerAction)
			return;

		// Play all creature entities except player
		const auto& level = Game::Instance().CurrentLevel();
		for(const auto& entityId : level.Entities())
		{
			auto entity = entityId.Entity();
			if (entity != nullptr && entity->Type() == EntityType::Creature && entity != player)
			{
				// Run AI: move and bump attack
				if (level.EntityHasLineOfSightTo(*entity, player->GetLocation().position))
				{
					auto path = level.CalcPath(*entity, player->GetLocation().position);
					if (!path.empty())
						MoveAdj(*entity, path.at(0) - entity->GetLocation().position);
				}
			}
		}

		// now wait for player action
		waitingForPlayerAction = true;
	}
}