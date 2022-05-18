#pragma once

#include "state.h"

namespace rlf
{
	namespace state
	{
		class MainGame : public IState
		{
		public:
			bool update(StateStack& stateStack) override;
			void render() override;
			void onResumeFrom(const IState* state) override { isHeaderDirty = true; }
		private:
			bool isHeaderDirty = true;
		};
	}
}