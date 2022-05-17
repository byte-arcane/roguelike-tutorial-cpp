#include "inventory.h"

#include <array>
#include <algorithm>
#include <unordered_map>

#include <GLFW/glfw3.h>
#include <fmt/format.h>

#include "game.h"
#include "entity.h"
#include "input.h"
#include "eventhandlers.h"

using namespace std;
using namespace glm;

namespace rlf
{
	constexpr int ITEMS_PER_PAGE = 16; // TODO: Dependent on vert rows

	constexpr char* ITEM_CATEGORY_STRINGS[] = {
		"Weapon",
		"Shield",
		"Armor",
		"Accessory",
		"Consumable",
		"Other"
	};

	namespace color
	{
		constexpr glm::vec4 BROWN(1, .545, 0, 1);
	}

	// return the next writeable column for this line
	int addTextToLine(vector<uvec4>& buf, const string& text, int col, int row, const vec4& color)
	{
		for (const auto c : text)
		{
			if (c != ' ')
			{
				auto td = TileData{ c,color };
				auto bufferData = td.PackSparse({ col,row });
				buf.push_back(bufferData);
			}
			++col;
		}
		return col;
	}

	void addSeparatorLine(vector<uvec4>& buf, int row, const vec4& color, int numCols, const string& centeredText = "")
	{
		const auto dashTileData = TileData{ '-',color };

		if(centeredText.empty())
			for (int i = 0; i < numCols; ++i)
				buf.push_back(dashTileData.PackSparse({ i,row }));
		else
		{
			auto numColsEachSide = (numCols - (centeredText.size() + 2))/2;
			for (int i = 0; i < numColsEachSide; ++i)
			{
				buf.push_back(dashTileData.PackSparse({ i,row }));
				buf.push_back(dashTileData.PackSparse({ numCols-1-i,row }));
			}
			for (int i = 0; i<int(centeredText.size()); ++i)
			{
				auto c = centeredText[i];
				if (c != ' ')
				{
					TileData td{ c, color };
					buf.push_back(td.PackSparse({ numColsEachSide+1+i,row }));
				}
			}
		}
	}

	array<vector<EntityId>, NUM_ITEM_CATEGORIES> inventoryByItemCategory(const vector<EntityId>& items)
	{
		array<vector<EntityId>, NUM_ITEM_CATEGORIES> itemsByCategory;
		for (const auto& itemId : items)
		{
			auto category = itemId.Entity()->DbCfg().Cfg()->itemCfg.category;
			itemsByCategory[int(category)].push_back(itemId);
		}
		// sort alphabetically
		for (auto& vec : itemsByCategory)
			std::sort(vec.begin(), vec.end(), [&](const EntityId& lhs, const EntityId& rhs) {return lhs.Entity()->Name() < rhs.Entity()->Name(); });
		return itemsByCategory;
	}

	void sortItems(vector<EntityId>& items)
	{
		std::sort(items.begin(), items.end(), [](const EntityId& lhs, const EntityId& rhs) -> bool {
			auto lhse = lhs.Entity();
			auto rhse = rhs.Entity();
			auto lhs_cat = lhse->DbCfg().Cfg()->itemCfg.category;
			auto rhs_cat = rhse->DbCfg().Cfg()->itemCfg.category;
			// strict weak ordering
			return (lhs_cat < rhs_cat) || (lhs_cat == rhs_cat && lhse->Name() < rhse->Name());
		});
	}

	void inventoryAction(int itemIdx, InventoryMode inventoryMode, Entity& entity)
	{
		const auto& items = entity.GetInventory()->items;
		auto item = items[itemIdx].Entity();
		if (inventoryMode == InventoryMode::EquipOrUse)
		{
			auto itemCategory = item->DbCfg().Cfg()->itemCfg.category;
			// is it equipped? Unequip
			if (item->GetItemData()->equipped)
				ChangeEquippedItem(entity, -1, itemIdx);
			// is it equippable? equip
			else if (int(itemCategory) < NUM_EQUIPMENT_SLOTS)
			{
				// find which was previously equipped in that slot
				auto it_found = std::find_if(items.begin(), items.end(), [itemCategory](const EntityId& itemId) {
					auto curItem = itemId.Entity();
					return curItem->GetItemData()->equipped && curItem->DbCfg().Cfg()->itemCfg.category == itemCategory;
					});
				int oldEquippedIdx = it_found != items.end() ? int(std::distance(items.begin(), it_found)) : -1;
				ChangeEquippedItem(entity, itemIdx, oldEquippedIdx);
			}
			else if (itemCategory == ItemCategory::Consumable)
				UseItem(entity, itemIdx);
		}
		else if (inventoryMode == InventoryMode::PickUpOrDrop)
		{
			auto player = GameState::Instance().Player();
			auto isPlayer = GameState::Instance().IsPlayer(entity);
			if (isPlayer)
			{
				// can't drop equipped items!
				if(!item->GetItemData()->equipped)
					Drop(entity, items[itemIdx]);
			}
			else
				PickUp(*player.Entity(), entity, items[itemIdx]);
		}
	}

	bool inventoryHandleInput(int& pageIdx, InventoryMode inventoryMode, Entity& entity)
	{	
		auto& items = entity.GetInventory()->items;
		auto firstIdxAtPage = pageIdx * ITEMS_PER_PAGE;
		auto itemsInPage = glm::min(ITEMS_PER_PAGE, int(items.size() - firstIdxAtPage));
		bool hasPrevPage = pageIdx != 0;
		bool hasNextPage = (firstIdxAtPage + ITEMS_PER_PAGE) < items.size();
		for (int i = 0; i < itemsInPage; ++i)
		{
			if (cgf::Input::GetKeyDown(GLFW_KEY_A + i))
			{
				inventoryAction(firstIdxAtPage + i, inventoryMode, entity);
				return true;
			}
		}
		if (hasNextPage && cgf::Input::GetKeyDown(GLFW_KEY_KP_ADD))
		{
			pageIdx++;
			return true;
		}
		if (hasPrevPage && cgf::Input::GetKeyDown(GLFW_KEY_KP_SUBTRACT))
		{
			pageIdx--;
			return true;
		}
		if (cgf::Input::GetKeyDown(GLFW_KEY_ESCAPE))
		{
			pageIdx = -1;
			return true;
		}
		return false;
	}

	void inventoryBuildGuiBuffer(vector<uvec4>& buffer, int pageIdx, InventoryMode inventoryMode, const Entity& entity, int guiNumRows, const glm::ivec2& gameGridSize)
	{
		buffer.resize(0);
		auto isPlayer = GameState::Instance().IsPlayer(entity);
		auto& items = entity.GetInventory()->items;
		sortItems(items);
		auto numPages = (items.size() + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE;
		auto firstIdxAtPage = pageIdx * ITEMS_PER_PAGE;
		auto itemsInPage = glm::min(ITEMS_PER_PAGE, int(items.size() - firstIdxAtPage));
		bool hasPrevPage = pageIdx != 0;
		bool hasNextPage = pageIdx < (numPages-1);

		std::string inventoryModeText = inventoryMode == InventoryMode::EquipOrUse ? "Equip/Use" : (isPlayer ? "Drop" : "Pick up");
		addSeparatorLine(buffer, guiNumRows+gameGridSize.y-1, color::BROWN, gameGridSize.x, "Inventory: " + inventoryModeText);
		addTextToLine(buffer, fmt::format("Total weight: {0} stones", entity.GetInventory()->Weight()),0, guiNumRows+gameGridSize.y-3, color::BROWN);
		int rowItems0 = guiNumRows+gameGridSize.y - 5;
		ItemCategory category = ItemCategory(-1);
		const int maxNameSize = 40;
		for (int iItem = 0; iItem < itemsInPage; ++iItem)
		{
			const auto& item = items[iItem + firstIdxAtPage].Entity();
			auto isEquipped = item->GetItemData()->equipped;
			auto newCategory = item->DbCfg().Cfg()->itemCfg.category;
			bool changedCategory = category != newCategory;
			category = newCategory;

			std::string text;
			text.push_back('a' + iItem);
			
			text += isEquipped ? ") [E] " : ")     ";
			text += item->Name();
			int pad = maxNameSize - item->Name().size();
			for (int i = 0; i < pad; ++i)
				text.push_back(' ');
			if (changedCategory)
				text += fmt::format(" [{0}]", ITEM_CATEGORY_STRINGS[int(category)]);
			addTextToLine(buffer, text, 0, rowItems0-iItem, color::BROWN);
		}
		auto lastLine = fmt::format("[a-{0}] {1}", char('a' + itemsInPage - 1), inventoryModeText);
		if (hasPrevPage && hasNextPage)
			lastLine += " - [+/-] Next/previous page";
		else if (hasPrevPage)
			lastLine += " - [-] Previous page";
		if (hasNextPage)
			lastLine += " - [+] Next page";
		lastLine += fmt::format(" - Page {0}/{1} - [Esc] Exit inventory", pageIdx + 1, glm::max(int(numPages),1));
		addTextToLine(buffer, lastLine, 0, guiNumRows, color::BROWN);
		addSeparatorLine(buffer, guiNumRows-1, color::BROWN, gameGridSize.x);
	}
}