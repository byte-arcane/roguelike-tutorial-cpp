#pragma once

#include <vector>

#include "state.h"

#include <glm/glm.hpp>

namespace rlf
{
	namespace state
	{
		class Inventory : public State
		{
		public:

			enum Mode
			{
				EquipOrUse = 0,
				PickUp,
				Drop
			};

			Inventory(Mode mode, std::function<void(bool, const State*)> onDone) :State(onDone),mode(mode) {}

			
			int PageIndex() const { return pageIndex; }

		private:
			void render() override;
			Status updateImpl(StateStack& stateStack) override;

		private:
			Mode mode;
			int pageIndex = 0;
			// set true if we need to rebuild the gui
			bool isGuiDirty = true;
			std::vector<glm::uvec4> bufferMain;
			std::vector<glm::uvec4> bufferHeader;
		};
	}
}