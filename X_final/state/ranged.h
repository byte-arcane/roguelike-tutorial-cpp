#pragma once

#include "state.h"

#include <glm/glm.hpp>

namespace rlf
{
	namespace state
	{
		class Ranged : public IState
		{
		public:
			bool update() override;
			void render() override;

			glm::ivec2 targetPosition = { -1,-1 };
		};
	}
}