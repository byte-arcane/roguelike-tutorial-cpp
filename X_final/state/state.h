#pragma once

#include <vector>
#include <functional>
#include <memory>
#include <string>

#include <glm/glm.hpp>

namespace rlf
{
	class Game;
	namespace state
	{
		// State update status
		enum class Status
		{
			Running = 0,
			Success,
			Abort
		};

		class State;
		// Stack of game states, top of the stack is the back of the vector
		using StateStack = std::vector<std::unique_ptr<State>>;

		// Base class for a game state, representing what input we're expecting, what do we display, and actions upon completion
		class State
		{
		public:
			virtual ~State() = default;

			State(std::function<void(bool, const State *)> onDone = {}) :onDone(onDone) {}

			// start listening to any signals
			virtual void StartListening() {}
			// stop listening signals
			virtual void StopListening() {}
			// display the view for the current state
			virtual void Render() = 0;

		protected:
			friend class Game;
			// update this state given the current state stack. This is called from Game::UpdateCurrentState()
			void Update(StateStack& stateStack);
			// define what happens when we resume from a child state
			virtual void ResumeFrom(const State* state) {}
			// implementation details for part of the Update() function
			virtual Status UpdateImpl() = 0;
		private:
			
		private:
			// callack for when we're done with this state
			std::function<void(bool, const State*)> onDone;
		};

		// Helper that adds text to a buffer, to be used for rendering fonts on screen. Specify row, starting column and color
		int AddTextToLine(std::vector<glm::uvec4>& buf, const std::string& text, int col, int row, const glm::vec4& color);
		// Helper that adds a separator line to a buffer, as above. Optionally specify some text that should be centered, and the character to be replicated along the separator line
		void AddSeparatorLine(std::vector<glm::uvec4>& buf, int row, const glm::vec4& color, int numCols, const std::string& centeredText = "", char fillChar = '-');
	}
}