#include <glm/glm.hpp>

namespace rlf
{
	class Input
	{
	public:

		//Is key pressed?
		static bool GetKey(int key);
		//Was the key just pressed?
		static bool GetKeyDown(int key);
		// Get the mouse cursor position
		static glm::vec2 MouseCursor();
		// Get the delta movement of the mouse cursor since last frame
		static glm::vec2 MouseCursorDelta();
		// check if a mouse button is pressed
		static bool GetMouseButton(int btn);
		// check if a mouse button was just pressed
		static bool GetMouseButtonDown(int btn);
		// Initialize input state
		static void Initialize();
		// Reset the state -- do that before polling
		static void ResetState();
		// The callback functions for when a key is pressed
		static void KeyCallback(int key, int action);
		// The callback functions for when a mouse button is pressed
		static void MouseButtonCallback(int button, int action);
		// The callback functions for when a mouse cursor has moved
		static void MouseCursorCallback(float x, float y);
	};
}