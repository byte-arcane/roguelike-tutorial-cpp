#pragma once

#include <vector>
#include <functional>
#include <unordered_map>

#include <glm/glm.hpp>

#include "entityid.h"
#include "tilemap.h"
#include "spritemap.h"
#include "sparsebuffer.h"

template <class T>
class MyHash;

namespace rlf
{
	class Entity; 
	class Level;

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
		// Render the typical GUI, which is some character info and a few lines of the log
		void RenderGui();
		// Render the header, here a simple row at the top of the screen
		void RenderHeader();
		// Render the game data in the game area (here, above the gui)
		void RenderGame();
		// Render some data in the game area (e.g. inventory).
		void RenderGameOverlay(const SparseBuffer& buffer);
		// Render targets and one of them is optioally marked as the currently selected (-1 for none selected)
		void RenderTargets(const SparseBuffer& buffer, int targetIdx);
		// Render the main menu
		void RenderMenu(const SparseBuffer& buffer);
		// Get a sparse buffer using a name
		SparseBuffer& RequestBuffer(const std::string& name) { return bufferMap[name];  }
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

	private:
		// Signal-slots
		void OnEntityMoved(const Entity& e);
		void OnEntityAdded(Entity& e);
		void OnEntityRemoved(Entity& e);
		void OnLevelChanged(const Level& level);
		void OnFogOfWarChanged();
		void OnObjectStateChanged(const Entity& e);
		void OnGuiUpdated() { isGuiDirty = true; }
		void OnGameLoaded();

	private:
		// Calculate color/glyph used for this entity
		void UpdateRenderableEntity(const Entity& e);

		// Helper to setup the viewport to render over a specific subgrid in the display
		void SetupViewport(const glm::ivec2& tileStart, const glm::ivec2& tileNum);
		
	private:

		// gui element info and how big each is, in terms of rows. Order is bottom to top. -1 (main) will auto-fill based on remaining rows, so that the total is equal to screenSize.y
		std::vector<std::pair<std::string, int>> guiSegments = { {"char",5}, {"main",-1}, {"status",1} };

		// do we need to update the gpu buffer for the gui data?
		bool isGuiDirty = true;
		
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
		// fog of war texture
		uint32_t texFogOfWar=0;
		// gpu buffer for level objects
		SparseBuffer bufferObjects;
		// gpu buffer for level creatures
		SparseBuffer bufferCreatures;
		// map from entity id (object/creature) to gpu buffer index
		std::unordered_map<EntityId, int> entityToBufferIndex;
		
		// cpu gui data
		std::vector<glm::uvec4> guiBuffer;

		// maps to gpu data buffers.
		std::unordered_map<std::string, SparseBuffer> bufferMap;
	};
}