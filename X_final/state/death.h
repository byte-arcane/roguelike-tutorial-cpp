#pragma once

#include "state.h"

namespace rlf
{
	namespace state
	{
		class Death : public State
		{
		public:
			int option = 0;
		private:
			void Render() override;
			Status UpdateImpl() override;
		private:
			bool isGuiDirty = true;
		};
	}
}