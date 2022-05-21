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
			if (rlf::Input::GetKeyDown(GLFW_KEY_ESCAPE))
			{
				targetIndex = -1;
				return Status::Abort;
			}
			if (rlf::Input::GetKeyDown(GLFW_KEY_TAB))
				targetIndex = (targetIndex+1)%validTargets.size();
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
				
				// Build the header
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
					buffer.push_back(TileData('X', glm::vec4(1, 0, 0, 1)).PackSparse(q));
				}
					
				if (!sparseBuffer.IsInitialized())
					sparseBuffer.Init(sizeof(glm::uvec4), 1000);
				sparseBuffer.Set(buffer.size(), buffer.data());
			}

			gfx.RenderGame();
			gfx.RenderGui();
			gfx.RenderHeader();
			gfx.RenderTargets(sparseBuffer, targetIndex);
		}
	}
}