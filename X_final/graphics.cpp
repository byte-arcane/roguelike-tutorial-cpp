#include "graphics.h"

#include <algorithm>
#include <vector>

#include <gl/glew.h>
#include <fmt/format.h>

#include "utility.h"
#include "framework.h"
#include "entity.h"
#include "game.h"
#include "input.h"
#include "eventhandlers.h"

using namespace glm;

namespace rlf
{
	void AddTextSprites(std::vector<uvec4>& buffer, const std::string& text, int row, const glm::vec4& color)
	{
		int x = 0;
		for (const auto& c : text)
		{
			if (c != ' ')
			{
				auto td = TileData{ c,color };
				auto bufferData = td.PackSparse({ x,row });
				buffer.push_back(bufferData);
			}
			++x;
		}
	}

	void BuildPlayerGui(std::vector<uvec4>& buffer, int numRows)
	{
		buffer.resize(0);
		const int maxShownLines = numRows - 2;
		auto player = GameState::Instance().Player().Entity();
		if (player != nullptr)
		{
			auto loc = player->GetLocation();
			auto p = loc.position;
			int gold = 0;
			auto it_gold = std::find_if(player->GetInventory()->items.begin(), player->GetInventory()->items.end(), [](const EntityId& itemId) {return itemId.Entity()->Name() == "gold"; });
			if (it_gold != player->GetInventory()->items.end())
				gold = it_gold->Entity()->GetItemData()->stackSize;
			const auto& cd = player->GetCreatureData();
			auto hpMax = player->DbCfg().Cfg()->creatureCfg.hp;
			auto cs = AccumulateCombatStats(*player);
			AddTextSprites(buffer, fmt::format("{0} - {1},{2} Lvl:{3} ATT:{4} DEF:{5} DMG:{6} RES:{7} HP:{8}({9}) XP:{10} ${11}", player->Name(), p.x, p.y, loc.levelId + 1, cs.x, cs.y, cs.z, cs.w, cd->hp, hpMax, cd->xp, gold), maxShownLines, glm::vec4(1));
		}
		const auto& messages = GameState::Instance().MessageLog();
		
		vec4 color{ .7,.7, .7, 1 };
		for (int iLine = 0; iLine < maxShownLines; ++iLine)
		{
			if (messages.size() > iLine)
			{
				const auto& textAndRepeats = messages.rbegin() + iLine;
				std::string message = textAndRepeats->first;
				if (textAndRepeats->second > 1)
					message += fmt::format(" (x{0})", textAndRepeats->second);
				AddTextSprites(buffer, message, maxShownLines - iLine - 1, color);
				color *= 0.8f;
				color.w = 1.0f;
			}
		}
	}

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
		VBO = rlf::buildVBO(&quadVertices[0].x, int(quadVertices.size() * sizeof(vec3)));
		// create the VAO which stores the 
		VAO = rlf::buildVAO(VBO, sizeof(vec3));
		numVerticesQuad = int(quadVertices.size());

		// build and compile our shader program
		shaderTilemapDense = rlf::buildShader(rlf::readTextFile(rlf::mediaSearch("shaders/tilemap_dense.vert")).c_str(), rlf::readTextFile(rlf::mediaSearch("shaders/tilemap_dense.frag")).c_str());
		shaderTilemapSparse = rlf::buildShader(rlf::readTextFile(rlf::mediaSearch("shaders/tilemap_sparse.vert")).c_str(), rlf::readTextFile(rlf::mediaSearch("shaders/tilemap_sparse.frag")).c_str());
		shaderDb["gui"] = rlf::buildShader(rlf::readTextFile(rlf::mediaSearch("shaders/tilemap_sparse_gui.vert")).c_str(), rlf::readTextFile(rlf::mediaSearch("shaders/tilemap_sparse_gui.frag")).c_str());
		shaderDb["gui_highlight"] = rlf::buildShader(rlf::readTextFile(rlf::mediaSearch("shaders/tilemap_sparse_guih.vert")).c_str(), rlf::readTextFile(rlf::mediaSearch("shaders/tilemap_sparse_guih.frag")).c_str());
		// TODO: add highlighting shader

		// Tilemap setup
		tilemap.Load(rlf::mediaSearch("textures/Curses_1920x900.png").c_str(), ivec2(24, 36));
		//tilemap.Load(rlf::mediaSearch("textures/Acorntileset8x8.png").c_str(), ivec2(8, 8));
		//tilemap.Load(rlf::mediaSearch("textures/Kjammer_square_1616_v02.png").c_str(), ivec2(16, 16));
		//tilemap.Load(rlf::mediaSearch("textures/Curses-square-24.png").c_str(), ivec2(24, 24));
		
		glTextureParameteri(tilemap.Texture(), GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(tilemap.Texture(), GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		const ivec2 screenSizePx = ivec2(rlf::FrameworkApp::ViewportWidth(), rlf::FrameworkApp::ViewportHeight());
		const auto& tileSize = tilemap.TileSize();
		screenSize = screenSizePx / tileSize;
		viewSizePx = screenSize * tileSize;
		screenOffsetPx = (screenSizePx - viewSizePx) / 2;

		// Support up to 1024 elements for each, per level
		bufferCreatures.Init(sizeof(uvec4), 1024);
		bufferObjects.Init(sizeof(uvec4), 1024);

		// Calculate the number of rows for the flexible element (typically the main screen)
		int numRowsFixed = 0;
		auto itUnfixed = guiSegments.end();
		for (auto it = guiSegments.begin(); it != guiSegments.end(); ++it)
			if (it->second >= 0)
				numRowsFixed += it->second;
			else
				itUnfixed = it;
		itUnfixed->second = screenSize.y - numRowsFixed;
	}

	void Graphics::Dispose()
	{
		texBg.Dispose();
		bufferCreatures.Dispose();
		bufferObjects.Dispose();

		for (auto& kv : bufferMap)
			kv.second.Dispose();
		for (auto& kv : spritemapMap)
			kv.second.Dispose();

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

	void SetupCameraAndFow(uint32_t program, const glm::ivec2& cameraOffset, uint32_t fow)
	{
		glUniform2i(glGetUniformLocation(program, "camera_offset"), cameraOffset.x, cameraOffset.y);
		glBindTextureUnit(2, fow);
		glUniform1i(glGetUniformLocation(program, "fow"), 2);
	}

	uint32_t Graphics::BeginDenseShader(int numRows)
	{
		glUseProgram(shaderTilemapDense);
		SetupTilemapAndGrid(shaderTilemapDense, tilemap, { screenSize.x, numRows });
		SetupCameraAndFow(shaderTilemapDense, cameraOffset, texFogOfWar);
		return shaderTilemapDense;
	}

	uint32_t Graphics::BeginSparseShader(int numRows)
	{
		// activate the sparse shader
		glUseProgram(shaderTilemapSparse);
		SetupTilemapAndGrid(shaderTilemapSparse, tilemap, { screenSize.x, numRows });
		SetupCameraAndFow(shaderTilemapDense, cameraOffset, texFogOfWar);
		return shaderTilemapSparse;
	}

	ivec2 Graphics::RowStartAndNum(const std::string& guiSegment) const
	{
		int iRow = 0;
		for (const auto& seg : guiSegments)
			if (seg.first == guiSegment)
				return { iRow, seg.second };
			else
				iRow += seg.second;
		// should not be here
		assert(false);
		return { -1,-1 };
	}

	ivec2 Graphics::MouseCursorTile() const {
		auto gameRowStartAndNum = RowStartAndNum("main");
		auto gameViewOffsetPx = screenOffsetPx + tilemap.TileSize() * ivec2(0, gameRowStartAndNum.x);
		auto gameViewSizePx = tilemap.TileSize() * ivec2(screenSize.x, gameRowStartAndNum.y);
		auto cursor = ivec2(rlf::Input::MouseCursor());
		// mouse cursor from GLFW starts at top-left. Change the .y so that it starts from bottom-left
		cursor.y = rlf::FrameworkApp::ViewportHeight() - 1 - cursor.y;
		// calculate the coordinates relative to the game view
		auto p = (cursor - gameViewOffsetPx);
		// if we're outside of the game view, return invalid coordinates
		if (p.x < 0 || p.y < 0 || p.x >= gameViewSizePx.x || p.y >= gameViewSizePx.y)
			return { -1,-1 }; // return invalid coordinates
		// from pixel coordinates, change to tile coordinates
		p /= tilemap.TileSize();
		// add the camera offset to calculate the level coordinates
		p += cameraOffset;
		// if we're out of map bounds, return invalid coordinates
		auto mapSize = GameState::Instance().CurrentLevel().Bg().Size();
		if (p.x < 0 || p.y < 0 || p.x >= mapSize.x || p.y >= mapSize.y)
			return { -1,-1 }; // return invalid coordinates
		printf("cursor: %d %d\n", p.x, p.y);
		return p;
	}

	void Graphics::CenterCameraAtPoint(const glm::ivec2& point)
	{
		auto gameRowStartAndNum = RowStartAndNum("main");
		auto viewGridSize = ivec2{screenSize.x, gameRowStartAndNum.y};
		auto halfScreenGridSize = viewGridSize / 2;
		cameraOffset = point - halfScreenGridSize;
		const auto& levelSize = GameState::Instance().CurrentLevel().Bg().Size();
		cameraOffset = clamp(cameraOffset, ivec2(0), levelSize- viewGridSize);
	}

	glm::ivec2 Graphics::WorldToScreen(const glm::ivec2& point) const
	{
		// top-left based coordinates
		auto q = point - cameraOffset;
		//q.y = RowStartAndNum("main").y - 1 - q.y;
		return q;
	}

	void Graphics::UpdateRenderableEntity(const Entity& e)
	{
		auto position = e.GetLocation().position;
		auto bufferData = e.CurrentTileData().PackSparse(position);

		auto& buffer = e.Type() == EntityType::Creature ? bufferCreatures : bufferObjects;

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
			auto& buffer = e.Type() == EntityType::Creature ? bufferCreatures : bufferObjects;
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
		entityToBufferIndex.clear();
		texBg.Dispose();
		rlf::deleteTexture(texFogOfWar);

		// Populate with new data
		const auto& bg = level.Bg();
		std::vector<uvec2> renderData(bg.Size().x * bg.Size().y);
		std::transform(bg.RawVec().begin(), bg.RawVec().end(), renderData.begin(), [](const LevelBgElement& elem) {
			TileData td{ elem.glyph, elem.color };
			return td.PackDense();
		});
		texBg.Init(bg.Size(), renderData.data());	
		texFogOfWar = rlf::createTexture(bg.Size(),GL_RED, GL_R8);

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

	//
	void Graphics::RenderGui()
	{
		auto& guiSparseBuffer = RequestBuffer("gui");
		auto rowStartAndNum = RowStartAndNum("char");
		if (isGuiDirty)
		{
			isGuiDirty = false;
			BuildPlayerGui(guiBuffer, rowStartAndNum.y);
			if (!guiSparseBuffer.IsInitialized())
				guiSparseBuffer.Init(sizeof(uvec4), 8192);
			guiSparseBuffer.Set(guiBuffer.size(), guiBuffer.data());
		}
		// set the viewport so that we don't render the padding
		auto guiOffsetPx = screenOffsetPx + tilemap.TileSize() * ivec2(0, rowStartAndNum.x);
		auto guiSizePx = tilemap.TileSize() * ivec2(screenSize.x, rowStartAndNum.y);
		glViewport(guiOffsetPx.x, guiOffsetPx.y, guiSizePx.x, guiSizePx.y);

		auto program = shaderDb["gui"];
		glUseProgram(program);
		SetupTilemapAndGrid(program, tilemap, { screenSize.x, rowStartAndNum.y });
		guiSparseBuffer.Draw();
	}

	void Graphics::RenderHeader()
	{
		auto& guiSparseBuffer = RequestBuffer("header");
		auto rowStartAndNum = RowStartAndNum("status");
		
		// set the viewport so that we don't render the padding
		auto guiOffsetPx = screenOffsetPx + tilemap.TileSize() * ivec2(0, rowStartAndNum.x);
		auto guiSizePx = tilemap.TileSize() * ivec2(screenSize.x, rowStartAndNum.y);
		glViewport(guiOffsetPx.x, guiOffsetPx.y, guiSizePx.x, guiSizePx.y);

		auto program = shaderDb["gui"];
		glUseProgram(program);
		SetupTilemapAndGrid(program, tilemap, { screenSize.x, rowStartAndNum.y });
		guiSparseBuffer.Draw();
	}

	void Graphics::RenderGame()
	{
		// set the viewport so that we don't render the padding
		auto rowStartAndNum = RowStartAndNum("main");
		auto gameViewOffsetPx = screenOffsetPx + ivec2(0, rowStartAndNum.x) * tilemap.TileSize();
		auto gameViewSizePx = ivec2(screenSize.x, rowStartAndNum.y) * tilemap.TileSize();
		glViewport(gameViewOffsetPx.x, gameViewOffsetPx.y, gameViewSizePx.x, gameViewSizePx.y);

		// render bg layer(s) first
		auto program = Graphics::Instance().BeginDenseShader(rowStartAndNum.y);
		texBg.Draw(program);

		// render all sparse buffers using given order
		program = Graphics::Instance().BeginSparseShader(rowStartAndNum.y);
		glUniform1f(glGetUniformLocation(program, "show_in_explored_areas"), 1.0f);
		bufferObjects.Draw();
		glUniform1f(glGetUniformLocation(program, "show_in_explored_areas"), 0.0f);
		bufferCreatures.Draw();
	}

	void Graphics::RenderGameOverlay(const SparseBuffer& guiSparseBuffer)
	{
		auto rowStartAndNum = RowStartAndNum("main");

		// set the viewport so that we don't render the padding
		auto guiOffsetPx = screenOffsetPx + tilemap.TileSize() * ivec2(0, rowStartAndNum.x);
		auto guiSizePx = tilemap.TileSize() * ivec2(screenSize.x, rowStartAndNum.y);
		glViewport(guiOffsetPx.x, guiOffsetPx.y, guiSizePx.x, guiSizePx.y);

		auto program = shaderDb["gui"];
		glUseProgram(program);
		SetupTilemapAndGrid(program, tilemap, { screenSize.x, rowStartAndNum.y });
		guiSparseBuffer.Draw();
	}

	void Graphics::RenderTargets(const SparseBuffer& guiSparseBuffer, int targetIdx)
	{
		auto rowStartAndNum = RowStartAndNum("main");

		// set the viewport so that we don't render the padding
		auto guiOffsetPx = screenOffsetPx + tilemap.TileSize() * ivec2(0, rowStartAndNum.x);
		auto guiSizePx = tilemap.TileSize() * ivec2(screenSize.x, rowStartAndNum.y);
		glViewport(guiOffsetPx.x, guiOffsetPx.y, guiSizePx.x, guiSizePx.y);

		auto program = shaderDb["gui_highlight"];
		glUseProgram(program);
		SetupTilemapAndGrid(program, tilemap, { screenSize.x, rowStartAndNum.y });
		auto blink = ((int(FrameworkApp::Time() * 1000) / 530) % 2) != 0;
		glUniform1i(glGetUniformLocation(program, "targetIdx"), blink ? -1 : targetIdx);
		guiSparseBuffer.Draw();
	}

	void Graphics::RenderBg(const Spritemap& spritemap)
	{
		// TODO: variant of a dense pass
	}

	void Graphics::RenderMenu(const SparseBuffer& buffer)
	{
		// set the viewport so that we don't render the padding
		auto guiOffsetPx = screenOffsetPx;
		auto guiSizePx = tilemap.TileSize() * screenSize;
		glViewport(guiOffsetPx.x, guiOffsetPx.y, guiSizePx.x, guiSizePx.y);

		auto program = shaderDb["gui"];
		glUseProgram(program);
		SetupTilemapAndGrid(program, tilemap, screenSize);
		buffer.Draw();
	}
}