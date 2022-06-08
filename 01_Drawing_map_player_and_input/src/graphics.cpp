#include "graphics.h"

#include <algorithm>
#include <vector>

#include <gl/glew.h>
#include <fmt/format.h>

#include "utility.h"
#include "framework.h"
#include "game.h"
#include "input.h"
#include "signals.h"

using namespace glm;

namespace rlf
{
	void Graphics::Init()
	{
		// create a number of 3D vertices that form a triangle, specified in a counter-clockwise manner 
		std::vector<vec3> quadVertices =
		{
			vec3(0,0,0), // lower left
			vec3(1,0,0), // lower right
			vec3(1,1,0), // upper right
			vec3(1,1,0), // upper right
			vec3(0,1,0), // upper left
			vec3(0,0,0), // lower left
		};
		// provide the address of the first float element in the vertices array, and the TOTAL size in bytes
		VBO = BuildVBO(&quadVertices[0].x, int(quadVertices.size() * sizeof(vec3)));
		// create the VAO which stores the 
		VAO = BuildVAO(VBO, sizeof(vec3));
		numVerticesQuad = int(quadVertices.size());

		// specify the shader names (with an invalid associated program object), and then load them all
		shaderDb = {
			{"tilemap_dense_nofow",0},
			{"tilemap_sparse_nofow",0},
		};
		ReloadShaders();

		// Useful if we want to allocate 2D textures that have odd dimensions and store 1 byte per pixel.
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		// Tilemap setup -- ideally parameterize! from some application config object
		tilemap.Load(MediaSearch("textures/Curses_1920x900.png").c_str(), ivec2(24, 36));
		//tilemap.Load(MediaSearch("textures/Acorntileset8x8.png").c_str(), ivec2(8, 8));
		//tilemap.Load(MediaSearch("textures/Kjammer_square_1616_v02.png").c_str(), ivec2(16, 16));
		//tilemap.Load(MediaSearch("textures/Curses-square-24.png").c_str(), ivec2(24, 24));
		
		// we want pixel-crispy fonts, so, no filtering
		glTextureParameteri(tilemap.Texture(), GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(tilemap.Texture(), GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		// calculate a few useful values
		const ivec2 screenSizePx = ivec2(FrameworkApp::ViewportWidth(), FrameworkApp::ViewportHeight());
		const auto& tileSize = tilemap.TileSize();
		screenSize = screenSizePx / tileSize;
		viewSizePx = screenSize * tileSize;
		screenOffsetPx = (screenSizePx - viewSizePx) / 2;

		// Support up to 1024 elements for each, per level
		bufferCreatures.Init(sizeof(uvec4), 1024);
	}

	void Graphics::ReloadShaders()
	{
		for (auto& nameAndProgram : shaderDb)
		{
			auto vertexShaderFilename = MediaSearch(fmt::format("shaders/{0}.vert", nameAndProgram.first));
			auto fragmentShaderFilename = MediaSearch(fmt::format("shaders/{0}.frag", nameAndProgram.first));
			auto newProgram = BuildShader(ReadTextFile(vertexShaderFilename).c_str(), ReadTextFile(fragmentShaderFilename).c_str());;
			// if the shader loaded successfully, replace the old one
			if (newProgram != 0)
			{
				// if the old shader is valid, release the resource
				if (nameAndProgram.second != 0)
					glDeleteProgram(nameAndProgram.second);
				nameAndProgram.second = newProgram;
			}
		}
	}

	void Graphics::Dispose()
	{
		texBg.Dispose();
		bufferCreatures.Dispose();
		for (auto& kv : shaderDb)
			if(kv.second != 0)
				glDeleteProgram(kv.second);
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		tilemap.Dispose();
	}

	void Graphics::BeginRender()
	{
		// the frame starts with a clean scene
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// Bind the quad -- we'll keep reusing this, either stretched to fill the entire view, or using instancing (drawing many quads with a single call)
		glBindVertexArray(VAO);
	}

	void Graphics::EndRender()
	{
		// unbind the geometry
		glBindVertexArray(0);

		// deactivate the shader
		glUseProgram(0);

		// Allow rendering to entire window
		glViewport(screenOffsetPx.x, screenOffsetPx.y, viewSizePx.x, viewSizePx.y);
	}

	void SetupTilemapAndGrid(uint32_t program, const Tilemap& tilemap, const ivec2& screenGridSize)
	{
		// set uniforms for the dense pass
		glUniform2i(glGetUniformLocation(program, "screen_grid_size"), screenGridSize.x, screenGridSize.y);
		glBindTextureUnit(0, tilemap.Texture());
		glUniform1i(glGetUniformLocation(program, "tilemap"), 0);
		glUniform2i(glGetUniformLocation(program, "tilemap_tile_num"), tilemap.TileNum().x, tilemap.TileNum().y);
		glUniform2i(glGetUniformLocation(program, "tilemap_tile_size"), tilemap.TileSize().x, tilemap.TileSize().y);
	}

	void SetupCamera(uint32_t program, const glm::ivec2& cameraOffset)
	{
		glUniform2i(glGetUniformLocation(program, "camera_offset"), cameraOffset.x, cameraOffset.y);
	}

	void Graphics::SetupViewport(const glm::ivec2& tileStart, const glm::ivec2& tileNum)
	{
		// set the viewport so that we don't render the margin area
		auto guiOffsetPx = screenOffsetPx + tilemap.TileSize() * tileStart;
		auto guiSizePx = tilemap.TileSize() * tileNum;
		glViewport(guiOffsetPx.x, guiOffsetPx.y, guiSizePx.x, guiSizePx.y);
	}

	void Graphics::RenderGame()
	{
		// set the viewport so that we don't render the margin area
		auto rowStartAndNum = RowStartAndNum("main");
		SetupViewport({ 0,rowStartAndNum.x }, { screenSize.x, rowStartAndNum.y });

		// Render bg layer(s) first
		auto shaderTilemapDense = shaderDb.at("tilemap_dense_nofow");
		glUseProgram(shaderTilemapDense);
		SetupTilemapAndGrid(shaderTilemapDense, tilemap, { screenSize.x, rowStartAndNum.y });
		SetupCamera(shaderTilemapDense, cameraOffset);
		texBg.Draw(shaderTilemapDense);

		// Render all sparse buffers using given order
		auto shaderTilemapSparse = shaderDb.at("tilemap_sparse_nofow");
		glUseProgram(shaderTilemapSparse);
		SetupTilemapAndGrid(shaderTilemapSparse, tilemap, { screenSize.x, rowStartAndNum.y });
		SetupCamera(shaderTilemapSparse, cameraOffset);
		bufferCreatures.Draw();
	}
}