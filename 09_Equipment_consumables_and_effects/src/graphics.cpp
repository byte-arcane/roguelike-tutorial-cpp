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
#include "commands.h"
#include "signals.h"

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
		const int maxShownLogLines = numRows - 2;
		
		// Player info: a single line (2nd from top of the segment)
		auto player = Game::Instance().PlayerId().Entity();
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
			AddTextSprites(buffer, fmt::format("{0} - {1},{2} Lvl:{3} ATT:{4} DEF:{5} DMG:{6} RES:{7} HP:{8}({9}) XP:{10} ${11}", player->Name(), p.x, p.y, loc.levelId + 1, cs.x, cs.y, cs.z, cs.w, cd->hp, hpMax, cd->xp, gold), maxShownLogLines, glm::vec4(1));
		}

		// after the player info, display a few log messages
		const auto& messages = Game::Instance().MessageLog();
		vec4 color{ .7,.7, .7, 1 };
		for (int iLine = 0; iLine < maxShownLogLines; ++iLine)
		{
			if (messages.size() > iLine)
			{
				const auto& textAndRepeats = messages.rbegin() + iLine;
				std::string message = textAndRepeats->first;
				if (textAndRepeats->second > 1)
					message += fmt::format(" (x{0})", textAndRepeats->second);
				AddTextSprites(buffer, message, maxShownLogLines - iLine - 1, color);
				// further back messages get darker and darker
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
		VBO = BuildVBO(&quadVertices[0].x, int(quadVertices.size() * sizeof(vec3)));
		// create the VAO which stores the 
		VAO = BuildVAO(VBO, sizeof(vec3));
		numVerticesQuad = int(quadVertices.size());

		// specify the shader names (with an invalid associated program object), and then load them all
		shaderDb = {
			{"tilemap_dense",0},
			{"tilemap_sparse",0},
			{"tilemap_sparse_gui",0},
			{"tilemap_sparse_gui_highlight",0},
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

		// Start listening to signals here
		sig::onEntityMoved.connect<Graphics, &Graphics::OnEntityMoved>(this);
		sig::onEntityAdded.connect<Graphics, &Graphics::OnEntityAdded>(this);
		sig::onEntityRemoved.connect<Graphics, &Graphics::OnEntityRemoved>(this);
		sig::onLevelChanged.connect<Graphics, &Graphics::OnLevelChanged>(this);
		sig::onFogOfWarChanged.connect<Graphics, &Graphics::OnFogOfWarChanged>(this);
		sig::onObjectStateChanged.connect<Graphics, &Graphics::OnObjectStateChanged>(this);
		sig::onGuiUpdated.connect<Graphics, &Graphics::OnGuiUpdated>(this);
		sig::onGameLoaded.connect<Graphics, &Graphics::OnGameLoaded>(this);
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
		bufferObjects.Dispose();
		for (auto& kv : bufferMap)
			kv.second.Dispose();
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

	void SetupCameraAndFow(uint32_t program, const glm::ivec2& cameraOffset, uint32_t fow)
	{
		glUniform2i(glGetUniformLocation(program, "camera_offset"), cameraOffset.x, cameraOffset.y);
		glBindTextureUnit(2, fow);
		glUniform1i(glGetUniformLocation(program, "fow"), 2);
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

	void Graphics::CenterCameraAtPoint(const glm::ivec2& point)
	{
		auto gameRowStartAndNum = RowStartAndNum("main");
		auto gameAreaGridSize = ivec2{screenSize.x, gameRowStartAndNum.y};
		auto halfScreenGridSize = gameAreaGridSize / 2;
		cameraOffset = point - halfScreenGridSize;
		const auto& levelSize = Game::Instance().CurrentLevel().Bg().Size();
		cameraOffset = clamp(cameraOffset, ivec2(0), levelSize- gameAreaGridSize);
	}

	glm::ivec2 Graphics::WorldToScreen(const glm::ivec2& point) const
	{
		// top-left based coordinates
		auto q = point - cameraOffset;
		return q;
	}

	void Graphics::UpdateRenderableEntity(const Entity& e)
	{
		// calculate the new data
		auto position = e.GetLocation().position;
		auto bufferData = e.CurrentTileData().PackSparse(position);
		// get the buffer and the iterator that points to the relevant element in the buffer
		auto& buffer = e.Type() == EntityType::Creature ? bufferCreatures : bufferObjects;
		const auto& eid = e.Id();
		auto it = entityToBufferIndex.find(eid);
		// if we didn't find it, add a new element, otherwise update the old one
		if (it == entityToBufferIndex.end())
			entityToBufferIndex[eid] = buffer.Add(&bufferData);
		else
			buffer.Update(it->second, &bufferData);
		// if we're updating player data, center the camera at the player
		if (Game::Instance().IsPlayer(e))
			CenterCameraAtPoint(position);
	}

	void Graphics::OnEntityMoved(const Entity& e)
	{
		UpdateRenderableEntity(e);
	}

	void Graphics::OnEntityAdded(Entity& e)
	{
		if(e.Type() != EntityType::Item)
			UpdateRenderableEntity(e);
	}

	void Graphics::OnEntityRemoved(Entity& e)
	{
		// when we remove an entity (objects and creatures, that can be visible in the map), we need to free the corresponding buffer element
		if (e.Type() != EntityType::Item)
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
	}

	void Graphics::OnLevelChanged(const Level& level)
	{
		// Clear the old data
		bufferCreatures.Clear();
		bufferObjects.Clear();
		entityToBufferIndex.clear();
		texBg.Dispose();
		DeleteTexture(texFogOfWar);

		// Populate with new data
		const auto& bg = level.Bg();
		std::vector<uvec2> renderData(bg.Size().x * bg.Size().y);
		std::transform(bg.Data().begin(), bg.Data().end(), renderData.begin(), [](const LevelBgElement& elem) {
			TileData td{ elem.glyph, elem.color };
			return td.PackDense();
		});
		texBg.Init(bg.Size(), renderData.data());	
		texFogOfWar = CreateTexture(bg.Size(),GL_RED, GL_R8);

		for (const auto& entityId : level.Entities())
			UpdateRenderableEntity(*entityId.Entity());
	}

	void Graphics::OnFogOfWarChanged()
	{
		const auto& fogOfWar = Game::Instance().CurrentLevel().FogOfWar();
		auto size = fogOfWar.Size();
		glTextureSubImage2D(texFogOfWar, 0, 0, 0, size.x, size.y, GL_RED, GL_UNSIGNED_BYTE, fogOfWar.Data().data());
	}

	void Graphics::OnObjectStateChanged(const Entity& object)
	{
		UpdateRenderableEntity(object);
	}

	void Graphics::SetupViewport(const glm::ivec2& tileStart, const glm::ivec2& tileNum)
	{
		// set the viewport so that we don't render the margin area
		auto guiOffsetPx = screenOffsetPx + tilemap.TileSize() * tileStart;
		auto guiSizePx = tilemap.TileSize() * tileNum;
		glViewport(guiOffsetPx.x, guiOffsetPx.y, guiSizePx.x, guiSizePx.y);
	}

	void Graphics::RenderGui()
	{
		// First update the gui buffer if needed
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

		SetupViewport({ 0,rowStartAndNum.x }, { screenSize.x, rowStartAndNum.y });

		auto program = shaderDb["tilemap_sparse_gui"];
		glUseProgram(program);
		SetupTilemapAndGrid(program, tilemap, { screenSize.x, rowStartAndNum.y });
		guiSparseBuffer.Draw();
	}

	void Graphics::RenderHeader()
	{
		auto& guiSparseBuffer = RequestBuffer("header");
		auto rowStartAndNum = RowStartAndNum("status");
		
		SetupViewport({ 0,rowStartAndNum.x }, { screenSize.x, rowStartAndNum.y });

		auto program = shaderDb["tilemap_sparse_gui"];
		glUseProgram(program);
		SetupTilemapAndGrid(program, tilemap, { screenSize.x, rowStartAndNum.y });
		guiSparseBuffer.Draw();
	}

	void Graphics::RenderGame()
	{
		// set the viewport so that we don't render the margin area
		auto rowStartAndNum = RowStartAndNum("main");
		SetupViewport({ 0,rowStartAndNum.x }, { screenSize.x, rowStartAndNum.y });

		// Render bg layer(s) first
		auto shaderTilemapDense = shaderDb.at("tilemap_dense");
		glUseProgram(shaderTilemapDense);
		SetupTilemapAndGrid(shaderTilemapDense, tilemap, { screenSize.x, rowStartAndNum.y });
		SetupCameraAndFow(shaderTilemapDense, cameraOffset, texFogOfWar);
		texBg.Draw(shaderTilemapDense);

		// Render all sparse buffers using given order
		auto shaderTilemapSparse = shaderDb.at("tilemap_sparse");
		glUseProgram(shaderTilemapSparse);
		SetupTilemapAndGrid(shaderTilemapSparse, tilemap, { screenSize.x, rowStartAndNum.y });
		SetupCameraAndFow(shaderTilemapSparse, cameraOffset, texFogOfWar);
		glUniform1f(glGetUniformLocation(shaderTilemapSparse, "show_in_explored_areas"), 1.0f);
		bufferObjects.Draw();
		glUniform1f(glGetUniformLocation(shaderTilemapSparse, "show_in_explored_areas"), 0.0f);
		bufferCreatures.Draw();
	}

	void Graphics::RenderGameOverlay(const SparseBuffer& guiSparseBuffer)
	{
		auto rowStartAndNum = RowStartAndNum("main");
		SetupViewport({ 0,rowStartAndNum.x }, { screenSize.x, rowStartAndNum.y });

		auto program = shaderDb["tilemap_sparse_gui"];
		glUseProgram(program);
		SetupTilemapAndGrid(program, tilemap, { screenSize.x, rowStartAndNum.y });
		guiSparseBuffer.Draw();
	}

	void Graphics::RenderTargets(const SparseBuffer& guiSparseBuffer, int targetIdx)
	{
		auto rowStartAndNum = RowStartAndNum("main");
		SetupViewport({ 0,rowStartAndNum.x }, { screenSize.x, rowStartAndNum.y });

		auto program = shaderDb["tilemap_sparse_gui_highlight"];
		glUseProgram(program);
		SetupTilemapAndGrid(program, tilemap, { screenSize.x, rowStartAndNum.y });
		auto blink = ((int(FrameworkApp::Time() * 1000) / 530) % 2) != 0;
		glUniform1i(glGetUniformLocation(program, "targetIdx"), blink ? -1 : targetIdx);
		guiSparseBuffer.Draw();
	}

	void Graphics::RenderMenu(const SparseBuffer& buffer)
	{
		SetupViewport({ 0,0 }, screenSize);

		auto program = shaderDb["tilemap_sparse_gui"];
		glUseProgram(program);
		SetupTilemapAndGrid(program, tilemap, screenSize);
		buffer.Draw();
	}

	void Graphics::OnGameLoaded()
	{
		// redo the level and gui
		const auto& level = Game::Instance().CurrentLevel();
		OnLevelChanged(level);
		OnFogOfWarChanged();
		isGuiDirty = true;
	}
}