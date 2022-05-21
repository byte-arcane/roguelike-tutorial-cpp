#pragma once

#include <unordered_map>

// Roguelike framework
namespace rlf
{
	// This class is just an organisational unit for some basic functions
	class FrameworkApp
	{
	public:

		struct WindowSettings
		{
			bool fullscreen = false;
			int samples = -1; // for multisampling. By default don't force any option
		};

		~FrameworkApp() = default;

		virtual void configure(int argc, char ** argv) {}
		
		// Put here any initialisation code. Happens once, before the main loop and after initialisation of GLFW/GLEW/ImGui
		virtual void onInit() {}
		
		// Put here any termination code. Happens once, before termination of GLFW/GLEW/ImGui
		virtual void onTerminate() {}

		// Put here any update code. Called every frame
		virtual void onUpdate() {}

		// Put here any rendering code. Called every frame
		virtual void onRender() {}

		// Put here any GUI related code. Called after render
		virtual void onGui() {}

		// Function that runs the main loop
		int run();

		// Send a message to GLFW to quit the application
		void quit();

		static int ViewportWidth() { return viewportWidth; }
		static int ViewportHeight() { return viewportHeight; }
		static double Time();

	protected: 
		// Allow subclasses to modify settings, e.g. via the configure method
		WindowSettings settings;
	
	private:

		static int viewportWidth;
		static int viewportHeight;

	};
}