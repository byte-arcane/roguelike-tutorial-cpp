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
		// Stack of game states, top of the stack is the back of the vector
		using StateStack = std::vector<std::unique_ptr<State>>;

		class State
		{
		public:
			virtual ~State() = default;

			State(std::function<void(bool, const State *)> onDone = {}) :onDone(onDone) {}

			virtual void StartListening() {}
			virtual void StopListening() {}
			virtual void Render() = 0;
			void Update(StateStack& stateStack);

		protected:
			virtual void ResumeFrom(const State* state) {}
			virtual Status UpdateImpl() = 0;
		private:
			
		private:
			std::function<void(bool, const State*)> onDone;
		};

		int AddTextToLine(std::vector<glm::uvec4>& buf, const std::string& text, int col, int row, const glm::vec4& color);
		void AddSeparatorLine(std::vector<glm::uvec4>& buf, int row, const glm::vec4& color, int numCols, const std::string& centeredText = "", char fillChar = '-');
	}
}