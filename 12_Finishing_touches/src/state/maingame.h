#pragma once

#include "state.h"

namespace rlf
{
	namespace state
	{
		// The main game state
		class MainGame : public State
		{			
		private:
			void Render() override;
			void ResumeFrom(const State* state) override { isHeaderDirty = true; }
			Status UpdateImpl() override;
		private:
			// should we update the header? (top line of display)
			bool isHeaderDirty = true;
			// if we have a current projectile path, have the remaining points stored here
			std::vector<glm::ivec2> projectilePath;
			// if we have a current projectile path, what was the firing time?
			float projectileFireTime = 0.0f;
		};
	}
}