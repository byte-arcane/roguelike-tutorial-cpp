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

	// High-level graphics code
	class Graphics
	{
	public:

		static Graphics& Instance() { static Graphics instance; return instance; }

		void Init();
		void Dispose();
		void BeginRender();
		void EndRender();
		
		void RenderGui();
		void RenderHeader();
		void RenderGame();
		void RenderGameOverlay(const SparseBuffer& buffer);
		void RenderTargets(const SparseBuffer& buffer, int targetIdx);
		void RenderMenu(const SparseBuffer& buffer);

		SparseBuffer& RequestBuffer(const std::string& name) { return bufferMap[name];  }
		Spritemap RequestSpritemap(const std::string& name) { return spritemapMap[name];  }

		void CenterCameraAtPoint(const glm::ivec2& point);
		glm::ivec2 MouseCursorTile() const;
		glm::ivec2 WorldToScreen(const glm::ivec2& point) const;
		glm::ivec2 RowStartAndNum(const std::string& guiSegment) const;
		const glm::ivec2& ScreenSize() const { return screenSize; }

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
		
		uint32_t BeginDenseShader(int numRows);
		uint32_t BeginSparseShader(int numRows);
		void UpdateRenderableEntity(const Entity& e);

	private:
		
	private:

		// gui element info and how big each is. Order is bottom to top. -1 (main) will auto-fill based on remaining rows
		std::vector<std::pair<std::string, int>> guiSegments = { {"char",5}, {"main",-1}, {"status",1} };

		bool isGuiDirty = true;
		glm::ivec2 cameraOffset = {0,0};

		// viewport-specific, measured in CELLs
		glm::ivec2 screenSize = {0,0};

		// The viewport coordinates for the entire window
		glm::ivec2 screenOffsetPx = {0,0};
		glm::ivec2 viewSizePx = {0,0};

		// Quad
		unsigned int VBO = 0, VAO = 0;
		int numVerticesQuad = 0;

		// shaders
		std::unordered_map<std::string, uint32_t> shaderDb;

		// tilemap
		rlf::Tilemap tilemap;

		// Current level data
		Spritemap texBg;
		uint32_t texFogOfWar=0;
		SparseBuffer bufferObjects;
		SparseBuffer bufferCreatures;
		std::unordered_map<EntityId, int> entityToBufferIndex;
		
		// cpu gui data
		std::vector<glm::uvec4> guiBuffer;

		// maps to gpu data buffers/textures
		std::unordered_map<std::string, SparseBuffer> bufferMap;
		std::unordered_map<std::string, Spritemap> spritemapMap;
	};
}