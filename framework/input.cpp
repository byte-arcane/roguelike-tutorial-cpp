#include "input.h"

#include <unordered_map>
#include <array>

#include <GLFW/glfw3.h>

using namespace glm;

namespace rlf
{
	// The state for a key or a button
	struct KeyBtnState
	{
		// was it pressed last frame?
		bool wasPressed = false;
		// is it pressed now?
		bool isPressed = false;

		// return if key/btn is pressed
		bool GetKeyBtn() 
		{ 
			return isPressed;
		}

		// return if key/btn is pressed, but wasn't earlier
		bool GetKeyBtnDown() 
		{ 
			return isPressed && !wasPressed;
		}
		
		// set pressed status. values are pressed/release/repeat
		void Set(int value)
		{
			isPressed = value != GLFW_RELEASE;
		}

		void OnNewFrame()
		{
			wasPressed = isPressed;
		}
	};
	
	// State for each key
	std::array<KeyBtnState, 512> keyState;
	// State for each mouse button
	std::array<KeyBtnState, 8> mouseBtnState;
	// mouse cursor position
	vec2 mouseCursor;
	// mouse cursor offset since last position
	vec2 mouseCursorDelta;

	void Input::ResetState()
	{
		for (auto& v : keyState)
			v.OnNewFrame();
		for (auto& v : mouseBtnState)
			v.OnNewFrame();
	}

	bool Input::GetKey(int key)
	{
		return keyState[key].GetKeyBtn();
	}

	bool Input::GetKeyDown(int key)
	{
		return keyState[key].GetKeyBtnDown();
	}

	glm::vec2 Input::MouseCursor()
	{
		return mouseCursor;
	}

	glm::vec2 Input::MouseCursorDelta()
	{
		return mouseCursorDelta;
	}

	bool Input::GetMouseButton(int btn)
	{
		return mouseBtnState[btn].GetKeyBtn();
	}

	bool Input::GetMouseButtonDown(int btn)
	{
		return mouseBtnState[btn].GetKeyBtnDown();
	}

	void Input::Initialize()
	{
		keyState.fill({});
		mouseBtnState.fill({});
	}
	
	void Input::KeyCallback(int key, int action)
	{
		keyState[key].Set(action);
	}

	void Input::MouseButtonCallback(int button, int action)
	{
		mouseBtnState[button].Set(action);
	}

	void Input::MouseCursorCallback(float x, float y)
	{
		vec2 v(x, y);
		mouseCursorDelta = v - mouseCursor;
		mouseCursor = v;
	}
}