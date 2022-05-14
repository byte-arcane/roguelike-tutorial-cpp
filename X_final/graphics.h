#pragma once

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
	class Graphics
	{
	public:

		static Graphics& Instance() { static Graphics instance; return instance; }

		void Init();
		void Dispose();
		
		void BeginRender();
		void EndRender();
		uint32_t BeginDenseShader();
		uint32_t BeginSparseShader();
		void RenderLevel();
		void RenderGui();

		// Response functions
		void OnEntityMoved(const Entity& e);
		void OnEntityAdded(Entity& e);
		void OnEntityRemoved(Entity& e);
		void OnLevelChanged(const Level& level);
		void OnFogOfWarChanged();
		void OnObjectStateChanged(const Entity& e);
		void OnGuiUpdated() { BuildLevelGui(); }
		void CenterCameraAtPoint(const glm::ivec2& point);
	private:
		void UpdateRenderableEntity(const Entity& e);
		void SetupShaderCommon(uint32_t program);
		void BuildLevelGui();
		void AddTextSprites(const std::string& text, int lineIndex, const glm::vec4& color);

	private:
		
	private:
		glm::ivec2 cameraOffset = {0,0};

		// viewport-specific, measured in CELLs
		glm::ivec2 screenGridSize;
		int guiNumRows = 4;

		// The viewport coordinates for the entire window
		glm::ivec2 screenOffset;
		glm::ivec2 viewSize;

		// which subrect from screenGridSize we're using
		glm::ivec2 gameViewOffset;
		glm::ivec2 gameViewSize;

		glm::ivec2 guiOffset;
		glm::ivec2 guiSize;


		// Quad
		unsigned int VBO = 0, VAO = 0;
		int numVerticesQuad = 0;

		// shaders
		uint32_t shaderTilemapDense = 0;
		uint32_t shaderTilemapSparse = 0;
		uint32_t shaderTilemapSparseGui = 0;

		// tilemap
		rlf::Tilemap tilemap;

		// Current level data
		Spritemap texBg;
		uint32_t texFogOfWar=0;
		SparseBuffer bufferObjects;
		SparseBuffer bufferCreatures;
		std::unordered_map<EntityId, int> entityToBufferIndex;
		SparseBuffer bufferEffects; // spells/projectiles
		SparseBuffer bufferGui; // The user interface
	};
}