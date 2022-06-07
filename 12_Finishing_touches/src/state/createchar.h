#pragma once

#include "state.h"

namespace rlf
{
	namespace state
	{
		// When we start a new game, we end up in this state
		class CreateChar : public State
		{
		public:
			CreateChar(std::function<void(bool, const State*)> onDone) : State(onDone) {}

			const std::string& CharName() const { return charName; }
		private:
			void Render() override;
			Status UpdateImpl() override;
		private:
			// the name of the character that we've entered
			std::string charName;

			// if this is true, rebuild this screen
			bool isGuiDirty = true;
		};
	}
}