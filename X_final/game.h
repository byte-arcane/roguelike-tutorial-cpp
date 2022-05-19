#include <unordered_set>

#include "level.h"
#include "entity.h"
#include "turn.h"
#include "action.h"

namespace rlf
{
	class Entity;

	// The game state. [De]serializing this results in our savegame data
	class GameState
	{
	public:
		static GameState& Instance() { static GameState gameState; return gameState; }

		Entity * GetEntity(const EntityId& er);

		const std::vector<Level>& Levels() const { return levels; }
		int GetCurrentLevelIndex() const { return currentLevelIndex; }
		Level& CurrentLevel() { return levels[currentLevelIndex]; }
		void SetCurrentLevelIndex(int iLevel);

		void SetPlayer(const Entity& entity);
		EntityId Player() const { return playerId; }
		bool IsPlayer(const Entity& e) const { return Player().Entity() == &e; }

		// Remove an entity from the game
		void RemoveEntity(const Entity& e);

		EntityId CreateEntity(const DbIndex& cfg, const EntityDynamicConfig& dcfg);

		const std::vector<std::pair<std::string, int>>& MessageLog() const { return messageLog; }
		void WriteToMessageLog(const std::string& msg);

		void EndTurn();

	private:

		// entities. Stored as uptr, so that when the vector is resized and the memory is reallocated, our data is not invalidated
		std::vector<std::unique_ptr<Entity>> poolEntities;
		std::unordered_set<int> invalidPoolIndices;
		EntityId playerId;

		// levels
		std::vector<Level> levels;
		int currentLevelIndex = -1;

		TurnSystem turnSystem;

		// message log: messages and how many times each is encountered
		std::vector<std::pair<std::string,int>> messageLog;
	};
}