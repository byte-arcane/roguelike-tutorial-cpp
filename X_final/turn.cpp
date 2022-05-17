#include "turn.h"
#include "entity.h"

#include "game.h"
#include "eventhandlers.h"

namespace rlf
{
	void TurnSystem::Process()
	{
		// Don't process anything anymore if player is dead
		auto player = GameState::Instance().Player().Entity();
		if (player == nullptr || waitingForPlayerAction)
			return;

		// Play all creature entities except player
		const auto& level = GameState::Instance().CurrentLevel();
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
						MoveAdj(*entity, path[0] - entity->GetLocation().position);
				}
			}
		}

		// now wait for player action
		waitingForPlayerAction = true;
	}
}