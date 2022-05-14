#include "graphics.h"

#include <algorithm>
#include <vector>

#include <gl/glew.h>
#include <fmt/format.h>

#include "utility.h"
#include "framework.h"
#include "entity.h"
#include "game.h"

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
		VBO = cgf::buildVBO(&quadVertices[0].x, int(quadVertices.size() * sizeof(vec3)));
		// create the VAO which stores the 
		VAO = cgf::buildVAO(VBO, sizeof(vec3));
		numVerticesQuad = int(quadVertices.size());

		// build and compile our shader program
		shaderTilemapDense = cgf::buildShader(cgf::readTextFile(cgf::mediaSearch("shaders/tilemap_dense.vert")).c_str(), cgf::readTextFile(cgf::mediaSearch("shaders/tilemap_dense.frag")).c_str());
		shaderTilemapSparse = cgf::buildShader(cgf::readTextFile(cgf::mediaSearch("shaders/tilemap_sparse.vert")).c_str(), cgf::readTextFile(cgf::mediaSearch("shaders/tilemap_sparse.frag")).c_str());
		shaderTilemapSparseGui = cgf::buildShader(cgf::readTextFile(cgf::mediaSearch("shaders/tilemap_sparse_gui.vert")).c_str(), cgf::readTextFile(cgf::mediaSearch("shaders/tilemap_sparse_gui.frag")).c_str());

		// Tilemap setup
		tilemap.Load(cgf::mediaSearch("textures/Curses_1920x900.png").c_str(), ivec2(24, 36));
		//tilemap.Load(cgf::mediaSearch("textures/Acorntileset8x8.png").c_str(), ivec2(8, 8));
		//tilemap.Load(cgf::mediaSearch("textures/Kjammer_square_1616_v02.png").c_str(), ivec2(16, 16));
		//tilemap.Load(cgf::mediaSearch("textures/Curses-square-24.png").c_str(), ivec2(24, 24));
		
		glTextureParameteri(tilemap.Texture(), GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(tilemap.Texture(), GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		const ivec2 screenSize = ivec2(cgf::FrameworkApp::ViewportWidth(), cgf::FrameworkApp::ViewportHeight());
		screenGridSize = screenSize / tilemap.TileSize();
		ivec2 viewSize = screenGridSize * tilemap.TileSize();
		ivec2 screenOffset = (screenSize - viewSize) / 2;


		// Allocate the last 4 rows for gui
		auto guiHeight = guiNumRows * tilemap.TileSize().y;

		// The game view starts from the top
		gameViewOffset = screenOffset + ivec2(0, guiHeight);
		gameViewSize = viewSize - ivec2(0, guiHeight); // allow 3 lines for gui, at the end

		guiOffset = screenOffset;
		guiSize = ivec2(viewSize.x, guiHeight);

		// Support up to 1024 elements for each, per level
		bufferCreatures.Init(sizeof(uvec4), 1024);
		bufferObjects.Init(sizeof(uvec4), 1024);
		bufferEffects.Init(sizeof(uvec4), 1024);
		bufferGui.Init(sizeof(uvec4), 16384);
	}

	void Graphics::Dispose()
	{
		texBg.Dispose();
		bufferCreatures.Dispose();
		bufferObjects.Dispose();
		bufferEffects.Dispose();

		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteProgram(shaderTilemapDense);
		glDeleteProgram(shaderTilemapSparse);
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
		glViewport(screenOffset.x, screenOffset.y, viewSize.x, viewSize.y);
	}

	void Graphics::SetupShaderCommon(uint32_t program)
	{
		// set uniforms for the dense pass
		glUniform2i(glGetUniformLocation(program, "screen_grid_size"), screenGridSize.x, screenGridSize.y- guiNumRows);
		glUniform2i(glGetUniformLocation(program, "camera_offset"), cameraOffset.x, cameraOffset.y);
		glBindTextureUnit(0, tilemap.Texture());
		glUniform1i(glGetUniformLocation(program, "tilemap"), 0);
		glUniform2i(glGetUniformLocation(program, "tilemap_tile_num"), tilemap.TileNum().x, tilemap.TileNum().y);
		glUniform2i(glGetUniformLocation(program, "tilemap_tile_size"), tilemap.TileSize().x, tilemap.TileSize().y);

		glBindTextureUnit(2, texFogOfWar);
		glUniform1i(glGetUniformLocation(program, "fow"), 2);
	}

	uint32_t Graphics::BeginDenseShader()
	{
		glUseProgram(shaderTilemapDense);
		SetupShaderCommon(shaderTilemapDense);
		return shaderTilemapDense;
	}

	uint32_t Graphics::BeginSparseShader()
	{
		// activate the sparse shader
		glUseProgram(shaderTilemapSparse);
		SetupShaderCommon(shaderTilemapSparse);
		return shaderTilemapSparse;
	}

	void Graphics::CenterCameraAtPoint(const glm::ivec2& point)
	{
		auto halfScreenGridSize = screenGridSize / 2;
		cameraOffset = point - halfScreenGridSize;
		const auto& levelSize = GameState::Instance().CurrentLevel().Bg().Size();
		cameraOffset = clamp(cameraOffset, ivec2(0), levelSize- screenGridSize);
	}

	void Graphics::RenderLevel()
	{
		// set the viewport so that we don't render the padding
		glViewport(gameViewOffset.x, gameViewOffset.y, gameViewSize.x, gameViewSize.y);

		// render bg layer(s) first
		auto program = Graphics::Instance().BeginDenseShader();
		texBg.Draw(program);

		// render all sparse buffers using given order
		program = Graphics::Instance().BeginSparseShader();
		glUniform1f(glGetUniformLocation(program, "show_in_explored_areas"), 1.0f);
		bufferObjects.Draw();
		glUniform1f(glGetUniformLocation(program, "show_in_explored_areas"), 0.0f);
		bufferCreatures.Draw();
		bufferEffects.Draw();
	}

	void Graphics::RenderGui()
	{
		// set the viewport so that we don't render the padding
		glViewport(guiOffset.x, guiOffset.y, guiSize.x, guiSize.y);

		auto program = shaderTilemapSparseGui;
		glUseProgram(program);
		glUniform2i(glGetUniformLocation(program, "screen_grid_size"), screenGridSize.x, guiNumRows);
		glBindTextureUnit(0, tilemap.Texture());
		glUniform1i(glGetUniformLocation(program, "tilemap"), 0);
		glUniform2i(glGetUniformLocation(program, "tilemap_tile_num"), tilemap.TileNum().x, tilemap.TileNum().y);
		glUniform2i(glGetUniformLocation(program, "tilemap_tile_size"), tilemap.TileSize().x, tilemap.TileSize().y);
		
		bufferGui.Draw();
	}

	void Graphics::AddTextSprites(const std::string& text, int lineIndex, const glm::vec4& color)
	{
		int x = 0;
		for (const auto& c : text)
		{
			if (c != ' ')
			{
				auto td = TileData{ c,color };
				auto bufferData = td.PackSparse({ x,lineIndex });
				bufferGui.Add(&bufferData);
			}
			++x;
		}
	}

	void Graphics::BuildLevelGui()
	{
		bufferGui.Clear();
		auto player = GameState::Instance().Player().Entity();
		if (player != nullptr)
		{
			auto loc = player->GetLocation();
			auto p = loc.position;
			AddTextSprites(fmt::format("{0} - {1},{2} Lvl:{3} ATT:0 DMG:1 DEF:2 RES:3 HP:10(15) XP:0 $30", player->Name(), p.x,p.y,loc.levelId), 2, glm::vec4(1));
		}
		const auto& messages = GameState::Instance().MessageLog();
		const int maxShownLines = guiNumRows - 2;
		vec4 color{ .7,.7, .7, 1 };
		for(int iLine = 0; iLine < maxShownLines; ++iLine)
		{
			if(messages.size() > iLine)
			{
				const auto& textAndRepeats = messages.rbegin() + iLine;
				std::string message = textAndRepeats->first;
				if (textAndRepeats->second > 1)
					message += fmt::format(" (x{0})", textAndRepeats->second);
				AddTextSprites(message, maxShownLines-iLine-1, color);
				color *= 0.8f;
				color.w = 1.0f;
			}
		}
	}

	void Graphics::UpdateRenderableEntity(const Entity& e)
	{
		auto position = e.GetLocation().position;
		auto bufferData = e.CurrentTileData().PackSparse(position);

		auto& buffer = e.BaseType() == EntityType::Creature ? bufferCreatures : bufferObjects;

		const auto& eid = e.Id();
		auto it = entityToBufferIndex.find(eid);
		if (it == entityToBufferIndex.end())
			entityToBufferIndex[eid] = buffer.Add(&bufferData);
		else
			buffer.Update(it->second, &bufferData);
		
		if (GameState::Instance().IsPlayer(e))
			CenterCameraAtPoint(position);
	}

	void Graphics::OnEntityMoved(const Entity& e)
	{
		UpdateRenderableEntity(e);
	}

	void Graphics::OnEntityAdded(Entity& e)
	{
		UpdateRenderableEntity(e);
	}

	void Graphics::OnEntityRemoved(Entity& e)
	{
		auto it = entityToBufferIndex.find(e.Id());
		if (it != entityToBufferIndex.end())
		{
			auto& buffer = e.BaseType() == EntityType::Creature ? bufferCreatures : bufferObjects;
			constexpr uvec4 noData{ 0,0,0,0 };
			buffer.Update(it->second, &noData);
			entityToBufferIndex.erase(it);
		}
	}

	void Graphics::OnLevelChanged(const Level& level)
	{
		// Clear the old data
		bufferCreatures.Clear();
		bufferObjects.Clear();
		bufferEffects.Clear();
		entityToBufferIndex.clear();
		texBg.Dispose();
		cgf::deleteTexture(texFogOfWar);

		// Populate with new data
		const auto& bg = level.Bg();
		std::vector<uvec2> renderData(bg.Size().x * bg.Size().y);
		std::transform(bg.RawVec().begin(), bg.RawVec().end(), renderData.begin(), [](const LevelBgElement& elem) {
			TileData td{ elem.glyph, elem.color };
			return td.PackDense();
		});
		texBg.Init(bg.Size(), renderData.data());	
		texFogOfWar = cgf::createTexture(bg.Size(),GL_RED, GL_R8);

		for (const auto& entityId : level.Entities())
			UpdateRenderableEntity(*entityId.Entity());
	}

	void Graphics::OnFogOfWarChanged()
	{
		const auto& fogOfWar = GameState::Instance().CurrentLevel().FogOfWar();
		auto size = fogOfWar.Size();
		glTextureSubImage2D(texFogOfWar, 0, 0, 0, size.x, size.y, GL_RED, GL_UNSIGNED_BYTE, fogOfWar.Raw());
	}

	void Graphics::OnObjectStateChanged(const Entity& object)
	{
		UpdateRenderableEntity(object);
	}
}