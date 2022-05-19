#pragma once

#include <vector>
#include <functional>
#include <memory>
#include <string>

#include <glm/glm.hpp>

namespace rlf
{
	namespace state
	{
		enum class Status
		{
			Running = 0,
			Success,
			Abort
		};

		class State;
		using StateStack = std::vector<std::unique_ptr<State>>;

		class State
		{
		public:
			virtual ~State() = default;

			State(std::function<void(bool, const State *)> onDone = {}) :onDone(onDone) {}

			static void updateStack(StateStack& stateStack);
			static void renderStack(StateStack& stateStack);

		protected:
			virtual void onResumeFrom(const State* state) {}
			virtual void render() = 0;
			virtual Status updateImpl(StateStack& stateStack) = 0;
		private:
			void update(StateStack& stateStack);
		private:
			std::function<void(bool, const State*)> onDone;
		};

		int addTextToLine(std::vector<glm::uvec4>& buf, const std::string& text, int col, int row, const glm::vec4& color);
		void addSeparatorLine(std::vector<glm::uvec4>& buf, int row, const glm::vec4& color, int numCols, const std::string& centeredText = "");
	}
}