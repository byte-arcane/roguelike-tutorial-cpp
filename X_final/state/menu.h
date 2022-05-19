#pragma once

#include "state.h"

namespace rlf
{
	namespace state
	{
		class Menu : public State
		{
		public:
			int option = 0;
		private:
			void render() override;
			Status updateImpl(StateStack& stateStack) override;
		};
	}
}