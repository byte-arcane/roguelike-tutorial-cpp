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
			void StartListening() override;
			void StopListening() override;

			// signals-slots
			void OnPlayerDied();

			void StartNewGame(const std::string& charName);
			void ContinueGame();
		
			void Render() override;
			Status UpdateImpl() override;

		private:

			bool changeToDeathState = false;
		};
	}
}