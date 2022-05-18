#pragma once

#include <vector>
#include <memory>
#include <string>

#include <glm/glm.hpp>

namespace rlf
{
	namespace state
	{
		class IState;
		using StateStack = std::vector<std::unique_ptr<IState>>;

		class IState
		{
		public:
			virtual ~IState() = default;
			// return true when done
			virtual bool update(StateStack& stateStack) = 0;
			virtual void render() = 0;
			virtual void onResumeFrom(const IState* state) {}

			static void terminate(StateStack& stateStack) {
				auto thisState = std::move(stateStack.back());
				stateStack.pop_back();
				if(!stateStack.empty())
					stateStack.back()->onResumeFrom(thisState.get());
			}
		};

		int addTextToLine(std::vector<glm::uvec4>& buf, const std::string& text, int col, int row, const glm::vec4& color);
		void addSeparatorLine(std::vector<glm::uvec4>& buf, int row, const glm::vec4& color, int numCols, const std::string& centeredText = "");
	}
}