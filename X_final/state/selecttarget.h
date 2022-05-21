#pragma once

#include "state.h"

#include <glm/glm.hpp>

namespace rlf
{
	namespace state
	{
		// Use this state when during the game we want to take an action for which we need to select a target, e.g. a ranged attack or a look command
		class SelectTarget : public State
		{
		public:
			SelectTarget(const std::vector<glm::ivec2>& validTargets, std::function<void(bool, const State*)> onDone) :State(onDone), validTargets(validTargets) {}
			
			// The target index in the list of valid targets
			int targetIndex = 0;
		private:
			Status UpdateImpl() override;
			void Render() override;
		private:
			// set true if we need to rebuild our display view for this state
			bool isDirty = true;
			// a list of valid targets (display with 'X')
			std::vector<glm::ivec2> validTargets;
		};
	}
}