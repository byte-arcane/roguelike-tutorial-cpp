#pragma once

#include "state.h"

namespace rlf
{
	namespace state
	{
		class MainGame : public IState
		{
		public:
			bool update() override;
			void render() override;
		};
	}
}