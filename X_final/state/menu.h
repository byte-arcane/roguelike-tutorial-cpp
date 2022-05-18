#pragma once

#include "state.h"

namespace rlf
{
	namespace state
	{
		class Menu : public IState
		{
		public:
			bool update(StateStack& stateStack) override;
			void render() override;

			int option = -1;
		};
	}
}