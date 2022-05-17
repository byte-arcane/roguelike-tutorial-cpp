#pragma once

#include "state.h"

namespace rlf
{
	namespace state
	{
		class Menu : public IState
		{
		public:
			bool update() override;
			void render() override;
		};
	}
}