#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace rlf
{
	class Entity;
	enum InventoryMode 
	{
		EquipOrUse=0,
		PickUpOrDrop,
	};

	bool inventoryHandleInput(int& pageIdx, InventoryMode inventoryMode, Entity& entity);
	void inventoryBuildGuiBuffer(std::vector<glm::uvec4>& buffer, int pageIdx, InventoryMode inventoryMode, const Entity& entity, int guiNumRows, const glm::ivec2& gameGridSize);
}