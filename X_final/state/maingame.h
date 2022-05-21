#pragma once

#include "state.h"

namespace rlf
{
	namespace state
	{
		class MainGame : public State
		{			
		private:
			void Render() override;
			void ResumeFrom(const State* state) override { isHeaderDirty = true; }
			Status UpdateImpl() override;
		private:
			bool isHeaderDirty = true;
			std::vector<glm::ivec2> projectilePath;
			float projectileFireTime = 0.0f;
		};
	}
}