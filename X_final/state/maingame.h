#pragma once

#include "state.h"

namespace rlf
{
	namespace state
	{
		class MainGame : public State
		{			
		private:
			void render() override;
			void onResumeFrom(const State* state) override { isHeaderDirty = true; }
			Status updateImpl(StateStack& stateStack) override;
		private:
			bool isHeaderDirty = true;
			std::vector<glm::ivec2> projectilePath;
			float projectileFireTime;
		};
	}
}