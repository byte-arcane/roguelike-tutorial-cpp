#pragma once

#include <vector>

#include "state.h"

#include <glm/glm.hpp>

namespace rlf
{
	namespace state
	{
		// Use this state when interacting with the inventory
		class Inventory : public State
		{
		public:

			// The "mode" of this state
			enum class Mode
			{
				EquipOrUse = 0,
				PickUp,
				Drop
			};

			Inventory(Mode mode, std::function<void(bool, const State*)> onDone) :State(onDone),mode(mode) {}

			int PageIndex() const { return pageIndex; }

		private:
			void Render() override;
			Status UpdateImpl() override;

		private:
			// Which mode we're in?
			Mode mode;
			// The current page index (if we have more items than it's possible to display on the page)
			int pageIndex = 0;
			// set true if we need to rebuild the gui
			bool isGuiDirty = true;
			// temp cache for the font buffer of the main render area
			std::vector<glm::uvec4> bufferMain;
			// temp cache for the font buffer of the header
			std::vector<glm::uvec4> bufferHeader;
		};
	}
}