#pragma once

#include <vector>
#include <functional>
#include <unordered_map>

#include <glm/glm.hpp>

#include "tilemap.h"
#include "spritemap.h"
#include "sparsebuffer.h"

template <class T>
class MyHash;

namespace rlf
{
	// Graphics-related methods and state
	class Graphics
	{
	public:

		static Graphics& Instance() { static Graphics instance; return instance; }

		// Initialize the graphics subsystem
		void Init();
		// Clear any graphics resources (opengl buffers/textures/etc)
		void Dispose();
		// Begin rendering any game elements (this sets a quad as the main rendering primitive, that we use throughout)
		void BeginRender();
		// End rendering 
		void EndRender();
		// Render the game data in the game area (here, above the gui)
		void RenderGame();
		// the centered point is usually the player. This makes the view follow the player's positin
		void CenterCameraAtPoint(const glm::ivec2& point);
		// From a point in "world" space (e.g. level coordinates), calculate the cell coordinates for gui display, taking into account camera offset
		glm::ivec2 WorldToScreen(const glm::ivec2& point) const;
		// given the name of a segment, get the index of the first row, and how many rows does the segment occupy
		glm::ivec2 RowStartAndNum(const std::string& guiSegment) const;
		// Get the size of the screen in terms of tiles
		const glm::ivec2& ScreenSize() const { return screenSize; }
		// Reloads all shaders, allowing you to change the code and see the results immediately during the game
		void ReloadShaders();

		// Helper to setup the viewport to render over a specific subgrid in the display
		void SetupViewport(const glm::ivec2& tileStart, const glm::ivec2& tileNum);
		
	private:
		
		// camera moves with player in larger maps. Store the offset here
		glm::ivec2 cameraOffset = {0,0};

		// viewport-specific, measured in CELLs
		glm::ivec2 screenSize = {0,0};

		// The viewport coordinates for the entire window
		glm::ivec2 screenOffsetPx = {0,0};
		glm::ivec2 viewSizePx = {0,0};

		// Quad data for opengl
		unsigned int VBO = 0, VAO = 0;
		int numVerticesQuad = 0;

		// shaders
		std::unordered_map<std::string, uint32_t> shaderDb;

		// tilemap
		rlf::Tilemap tilemap;

		// Current level data
		Spritemap texBg;
		// gpu buffer for level creatures
		SparseBuffer bufferCreatures;
	};
}