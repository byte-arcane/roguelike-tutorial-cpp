#include "input.h"

#include <unordered_map>
#include <array>

#include <GLFW/glfw3.h>

using namespace glm;

namespace rlf
{
	struct KeyBtnState
	{
		bool isPressed = false;
		bool consumedPress = false;

		bool GetKeyBtn() 
		{ 
			consumedPress = true;
			return isPressed;
		}

		bool GetKeyBtnDown() 
		{ 
			auto ret = isPressed && !consumedPress;
			consumedPress = true;
			return ret;
		}
		
		void Set(int value)
		{
			isPressed = value != GLFW_RELEASE;
			consumedPress = false;
		}
	};

	std::array<KeyBtnState, 512> keyState;
	std::array<KeyBtnState, 8> mouseBtnState;
	vec2 mouseCursor;
	vec2 mouseCursorDelta;


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

	void Input::Initialise()
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