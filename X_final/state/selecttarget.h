#pragma once

#include "state.h"

#include <glm/glm.hpp>

namespace rlf
{
	namespace state
	{
		class SelectTarget : public IState
		{
		public:
			SelectTarget(const std::vector<glm::ivec2>& validTargets) :validTargets(validTargets) {}
			bool update(StateStack& stateStack) override;
			void render() override;

			glm::ivec2 targetPosition = { -1,-1 };
		private:
			std::vector<glm::ivec2> validTargets;
		};
	}
}