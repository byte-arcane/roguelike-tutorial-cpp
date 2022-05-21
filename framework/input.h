#include <glm/glm.hpp>

namespace rlf
{
	class Input
	{
	public:

		static bool GetKey(int key);
		static bool GetKeyDown(int key);

		static glm::vec2 MouseCursor();
		static glm::vec2 MouseCursorDelta();
		static bool GetMouseButton(int btn);
		static bool GetMouseButtonDown(int btn);

		static void Initialise();
		static void OnNewFrame();
		static void KeyCallback(int key, int action);
		static void MouseButtonCallback(int button, int action);
		static void MouseCursorCallback(float x, float y);
	};
}