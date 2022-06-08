#include <unordered_set>

#include "level.h"
#include "entity.h"
#include "state/state.h"

namespace rlf
{
	class Entity;

	// The game class, storing the game state, and providing functionality for interacting with the stored data
	class Game
	{
	public:
		// Main way of accessing the game data
		static Game& Instance() { static Game gameState; return gameState; }

		// Initialization code -- run once after application starts and window is set up
		void Init();

		// Get an entity pointer using an id
		Entity * GetEntity(const EntityId& entityId);

		// Get the current level
		Level& CurrentLevel() { return level; }

		// Change the current level to a new index. If that index does not correspond to a created level, one will be generated
		void EnterLevel();
		
		// Set the player entity
		void SetPlayer(const Entity& entity);
		
		// Get the player entity id. 
		EntityId PlayerId() const { return playerId; }

		// Check if the provided entity is the player
		bool IsPlayer(const Entity& e) const { return PlayerId().Entity() == &e; }

		// Remove an entity from the game
		void RemoveEntity(const Entity& e);

		// Create a new entity using a static configuration reference, and dynamic configuration data. We may or may not want to fire a message that the entity was created
		EntityId CreateEntity(const DbIndex& cfg, const EntityDynamicConfig& dcfg, bool fireMessage);

		// Get the message log
		const std::vector<std::pair<std::string, int>>& MessageLog() const { return messageLog; }

		// Write an entry into the message log
		void WriteToMessageLog(const std::string& msg);

		// Start a new game
		void New();

		// Render the current game state
		void RenderCurrentState();

		// Update the current game state
		void UpdateCurrentState();

	private:

		// entities. Stored as uptr, so that when the vector is resized and the memory is reallocated, our data is not invalidated
		std::vector<std::unique_ptr<Entity>> poolEntities;
		// keep a list of deleted entities in the pool, so that we can reuse the indices
		std::unordered_set<int> invalidPoolIndices;
		// store the player entity id
		EntityId playerId;

		// the list of levels
		Level level;

		// message log: messages and how many times each is encountered
		std::vector<std::pair<std::string,int>> messageLog;
	};
}