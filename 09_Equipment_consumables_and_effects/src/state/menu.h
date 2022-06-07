#pragma once

#include "state.h"

namespace rlf
{
	namespace state
	{
		// The starting game state, a menu with a few options
		class Menu : public State
		{
		private:

			// Start a new game, given character creation results (just name for now)
			void StartNewGame(const std::string& charName);
			// Continue a game from a save file on disk
			void ContinueGame();
		
			void Render() override;
			Status UpdateImpl() override;

		private:
			
		};
	}
}