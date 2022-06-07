#include "selecttarget.h"

#include <GLFW/glfw3.h>
#include "input.h"
#include "../graphics.h"

namespace rlf
{
	namespace state
	{
		Status SelectTarget::UpdateImpl()
		{
			// Escape cancels targetting
			if (rlf::Input::GetKeyDown(GLFW_KEY_ESCAPE))
			{
				targetIndex = -1;
				return Status::Abort;
			}
			// TAB moves through targets
			if (rlf::Input::GetKeyDown(GLFW_KEY_TAB))
				targetIndex = (targetIndex+1)%validTargets.size();
			// Enter confirms target
			if (rlf::Input::GetKeyDown(GLFW_KEY_ENTER))
				return Status::Success;
			return Status::Running;
		}

		void SelectTarget::Render()
		{
			auto& gfx = Graphics::Instance();
			auto& sparseBuffer = gfx.RequestBuffer("target");

			// Build the GPU data the first time only
			if (isDirty)
			{
				isDirty = false;
				
				// Build the buffer
				std::vector<glm::uvec4> buffer;
				auto screenSize = gfx.ScreenSize();
				AddSeparatorLine(buffer, 0, glm::vec4(1, 1, 1, 1), screenSize.x, "Select target");

				auto& sparseBufferHeader = gfx.RequestBuffer("header");
				if (!sparseBufferHeader.IsInitialized())
					sparseBufferHeader.Init(sizeof(glm::uvec4), 200);
				sparseBufferHeader.Set(buffer.size(), buffer.data());

				// Build the highlighted tiles
				buffer.resize(0);
				for (const auto& p : validTargets)
				{
					// Transform p to screen coordinates
					auto q = gfx.WorldToScreen(p);
					// Later on you might want to parameterize the targetting state in what symbol is used to mark the target and the color. Here it's a fixed red X
					buffer.push_back(TileData('X', glm::vec4(1, 0, 0, 1)).PackSparse(q));
				}
					
				// set the data
				if (!sparseBuffer.IsInitialized())
					sparseBuffer.Init(sizeof(glm::uvec4), 1000);
				sparseBuffer.Set(buffer.size(), buffer.data());
			}

			// For the targetting state we need to render everything that the main game state renders, plus the targets
			gfx.RenderGame();
			gfx.RenderGui();
			gfx.RenderHeader();
			gfx.RenderTargets(sparseBuffer, targetIndex);
		}
	}
}