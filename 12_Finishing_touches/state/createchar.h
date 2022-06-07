#pragma once

#include "state.h"

namespace rlf
{
	namespace state
	{
		class CreateChar : public State
		{
		public:
			CreateChar(std::function<void(bool, const State*)> onDone) :State(onDone) {}
			std::string charName;
		private:

			void Render() override;
			Status UpdateImpl() override;

		private:
			bool isGuiDirty = true;
		};
	}
}