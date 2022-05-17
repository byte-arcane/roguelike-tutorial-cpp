#pragma once

#include "state.h"

#include <glm/glm.hpp>

namespace rlf
{
	namespace state
	{
		class Inventory : public IState
		{
		public:

			enum Mode
			{
				EquipOrUse = 0,
				PickUp,
				Drop
			};

			Inventory(Mode mode) :mode(mode) {}

			bool update() override;
			void render() override;
		
			int PageIndex() const { return pageIndex; }

		private:
			Mode mode;
			int pageIndex = 0;
			// set true if we need to rebuild the gui
			bool isGuiDirty = true;
			std::vector<glm::uvec4> buffer;
		};
	}
}