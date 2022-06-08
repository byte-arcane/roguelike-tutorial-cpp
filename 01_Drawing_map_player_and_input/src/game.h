#include <unordered_set>

#include "level.h"

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

		// Get the current level
		Array2D<LevelBgElement>& Bg() { return bg; }

		// Change the current level to a new index. If that index does not correspond to a created level, one will be generated
		void EnterLevel();

		// Write an entry into the message log
		void WriteToMessageLog(const std::string& msg);

		// Start a new game
		void New();

		// Render the current game state
		void RenderCurrentState();

		// Update the current game state
		void UpdateCurrentState();

	private:
		Array2D<LevelBgElement> bg;
	};
}