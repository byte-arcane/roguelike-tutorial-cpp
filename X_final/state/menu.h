#pragma once

#include "state.h"

namespace rlf
{
	namespace state
	{
		class Menu : public State
		{
		public:
			enum class Option
			{
				NewGame=0,
				Continue,
				Exit
			};
			Option option = Option(-1);
		private:
			void startListening() override;
			void stopListening() override;

			// signals-slots
			void onPlayerDied();

			void startNewGame(const std::string& charName, StateStack& stateStack);
			void continueGame(StateStack& stateStack);
		
			void render() override;
			Status updateImpl(StateStack& stateStack) override;

		private:

			bool changeToDeathState = false;
		};
	}
}