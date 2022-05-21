#pragma once

#include "state.h"

namespace rlf
{
	namespace state
	{
		// When the player dies, we end up in this state
		class Death : public State
		{
		private:
			// Display death info
			void Render() override;
			// wait for ENTER key to go back to menu
			Status UpdateImpl() override;
		private:
			// set this to true if we need to rebuild the view
			bool isGuiDirty = true;
		};
	}
}