#include "eventhandlers.h"

#include "game.h"
#include "entity.h"
#include "graphics.h"

namespace rlf
{
	Entity& SpawnEntity(const DbIndex& cfg, const EntityDynamicConfig& dcfg)
	{
		auto& e = *GameState::Instance().CreateEntity(cfg, dcfg).Entity();
		if (e.BaseType() != EntityType::Item)
		{
			GameState::Instance().CurrentLevel().OnEntityAdded(e);
			Graphics::Instance().OnEntityAdded(e);
		}
		return e;
	}

	// Kill a creature
	void DestroyEntity(Entity& e)
	{
		// TODO: message
		if (e.BaseType() != EntityType::Item)
		{
			GameState::Instance().CurrentLevel().OnEntityRemoved(e);
			Graphics::Instance().OnEntityRemoved(e);
		}
		GameState::Instance().RemoveEntity(e);
	}

	void Move(Entity& entity, const glm::ivec2& position)
	{
		assert(entity.BaseType() != EntityType::Item);
		const auto& level = GameState::Instance().CurrentLevel();
		
		// Check if we can! If not, spawn a message
		if (!level.EntityCanMoveTo(entity, position))
			return;

		auto loc = entity.GetLocation();
		loc.position = position;
		entity.SetLocation(loc);
		Graphics::Instance().OnEntityMoved(entity);

		if (GameState::Instance().IsPlayer(entity))
		{
			GameState::Instance().CurrentLevel().UpdateFogOfWar();
			Graphics::Instance().OnGuiUpdated(); // movement should cause GUI update
		}
	}

	void MoveAdj(Entity& entity, const glm::ivec2& direction)
	{
		assert(entity.BaseType() != EntityType::Item);
		const auto& level = GameState::Instance().CurrentLevel();

		auto position = entity.GetLocation().position + direction;

		// Check if we can move
		if (level.EntityCanMoveTo(entity, position))
		{
			auto loc = entity.GetLocation();
			loc.position = position;
			entity.SetLocation(loc);
			Graphics::Instance().OnEntityMoved(entity);

			if (GameState::Instance().IsPlayer(entity))
			{
				GameState::Instance().CurrentLevel().UpdateFogOfWar();
				Graphics::Instance().OnGuiUpdated(); // movement should cause GUI update
			}
		}
		else // ok, we can't move. Get entity at the tile
		{
			auto entityAtPosition = level.GetEntity(position, true);
			if (entityAtPosition != nullptr)
			{
				switch (entityAtPosition->BaseType())
				{
					case EntityType::Creature:
						DestroyEntity(*entityAtPosition);
						break;
					case EntityType::Object:
						entityAtPosition->GetObjectData()->Handle(*entityAtPosition, entity);
						break;
				}
			}
		}
	}

	void HandleOnGround(Entity& handler)
	{
		const auto& level = GameState::Instance().CurrentLevel();
		auto position = handler.GetLocation().position;
		auto entityAtPosition = level.GetEntity(position, false);
		if (entityAtPosition != nullptr)
			entityAtPosition->GetObjectData()->Handle(*entityAtPosition, handler);
	}

	void ChangeLevel(int levelIndex)
	{
		auto& g = GameState::Instance();

		auto delveDirectionForward = g.GetCurrentLevelIndex() < levelIndex;

		auto playerId = g.Player();

		// remove player from old level
		if (playerId.Entity() != nullptr && g.GetCurrentLevelIndex() >= 0)
			g.CurrentLevel().OnEntityRemoved(*playerId.Entity());

		// Change the level
		g.SetCurrentLevelIndex(levelIndex);

		auto& level = g.CurrentLevel();
		Graphics::Instance().OnLevelChanged(level);		

		// put player in new level
		auto placeAtStairsEntityType = delveDirectionForward ? EntityType::Object_StairsUp : EntityType::Object_StairsDown;
		if (playerId.Entity() != nullptr)
		{
			for (const auto& entity : level.Entities())
				if (entity.Entity()->Type() == placeAtStairsEntityType)
				{
					auto position = entity.Entity()->GetLocation().position;
					playerId.Entity()->SetLocation({ levelIndex, position });
				}

			level.OnEntityAdded(*playerId.Entity());
			Graphics::Instance().OnEntityAdded(*playerId.Entity());
			level.UpdateFogOfWar();
		}

		Graphics::Instance().OnGuiUpdated(); // level change should cause GUI update
	}
}