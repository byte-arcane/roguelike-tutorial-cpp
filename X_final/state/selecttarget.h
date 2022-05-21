#pragma once

#include "state.h"

#include <glm/glm.hpp>

namespace rlf
{
	namespace state
	{
		class SelectTarget : public State
		{
		public:
			SelectTarget(const std::vector<glm::ivec2>& validTargets, std::function<void(bool, const State*)> onDone) :State(onDone), validTargets(validTargets) {}
			
			int targetIndex = 0;
		private:
			Status UpdateImpl() override;
			void Render() override;
		private:
			bool isDirty = true;
			std::vector<glm::ivec2> validTargets;
		};
	}
}