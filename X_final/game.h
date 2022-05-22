#include <unordered_set>

#include "level.h"
#include "entity.h"
#include "turn.h"
#include "state/state.h"

namespace rlf
{
	class Entity;

	// The data required for a savegame. It's most of the game state (except turn logic and game states)
	// See Game member variables for information on the below
	struct SaveData
	{
		std::vector<std::unique_ptr<Entity>> poolEntities;
		std::unordered_set<int> invalidPoolIndices;
		EntityId playerId;
		std::vector<Level> levels;
		int currentLevelIndex = -1;
		std::vector<std::pair<std::string, int>> messageLog;
	};

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

		// Get all levels
		const std::vector<Level>& Levels() const { return levels; }

		// Get the index of the current level
		int GetCurrentLevelIndex() const { return currentLevelIndex; }

		// Get the current level
		Level& CurrentLevel() { return levels.at(currentLevelIndex); }

		// Change the current level to a new index. If that index does not correspond to a created level, one will be generated
		void ChangeLevel(int iLevel);
		
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

		// Finish the turn and play all monsters
		void EndTurn();

		// Start a new game
		void New();

		// Load a saved game. Return if successful
		bool Load();

		// Save the game
		void Save();

		// Render the current game state
		void RenderCurrentState();

		// Update the current game state
		void UpdateCurrentState();

		// Push a new state onto the game state stack
		void PushState(std::unique_ptr<state::State>& state);

	private:

		// entities. Stored as uptr, so that when the vector is resized and the memory is reallocated, our data is not invalidated
		std::vector<std::unique_ptr<Entity>> poolEntities;
		// keep a list of deleted entities in the pool, so that we can reuse the indices
		std::unordered_set<int> invalidPoolIndices;
		// store the player entity id
		EntityId playerId;

		// the list of levels
		std::vector<Level> levels;
		// the current level index
		int currentLevelIndex = -1;

		// message log: messages and how many times each is encountered
		std::vector<std::pair<std::string,int>> messageLog;

		// NON SERIALIZABLE DATA
		
		// Turn logic
		TurnSystem turnSystem;

		// The game state stack
		state::StateStack gameStates;
	};
}