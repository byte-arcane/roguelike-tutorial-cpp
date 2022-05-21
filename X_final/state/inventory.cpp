#include "inventory.h"

#include <array>
#include <algorithm>
#include <unordered_map>

#include <GLFW/glfw3.h>
#include <fmt/format.h>
#include <magic_enum.hpp>

#include "input.h"

#include "../graphics.h"
#include "../game.h"
#include "../entity.h"
#include "../eventhandlers.h"

using namespace std;
using namespace glm;

namespace rlf
{
	namespace state
	{
		// As per below, we need 6 rows for padding things nicely
		int ItemsPerPage() { return Graphics::Instance().RowStartAndNum("main").y - 6; }; 

		namespace color // TODO: move out of here, in util/color.h or constants.h
		{
			constexpr glm::vec4 BROWN(1, .545, 0, 1);
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

		bool inventoryAction(int itemIdx, Inventory::Mode inventoryMode, Entity& entity)
		{
			bool doneSomething = true;
			const auto& items = entity.GetInventory()->items;
			auto item = items[itemIdx].Entity();
			if (inventoryMode == Inventory::Mode::EquipOrUse)
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
				else
					doneSomething = false;
			}
			else // pick up or drop
			{
				auto player = Game::Instance().PlayerId();
				auto isPlayer = Game::Instance().IsPlayer(entity);
				if (isPlayer)
				{
					// can't drop equipped items!
					if (!item->GetItemData()->equipped)
						Drop(entity, items[itemIdx]);
					else
						doneSomething = false;
				}
				else
					PickUp(*player.Entity(), entity, items[itemIdx]);
			}
			return doneSomething;
		}

		Entity& GetRelevantEntity(Inventory::Mode mode)
		{
			auto player = Game::Instance().PlayerId().Entity();
			if (mode != Inventory::Mode::PickUp)
				return *player;
			else
			{
				auto itemPile = Game::Instance().CurrentLevel().GetEntity(player->GetLocation().position, false);
				assert(itemPile->DbCfg() == DbIndex::ItemPile());
				return *itemPile;
			}
		}

		Status Inventory::UpdateImpl()
		{
			if (rlf::Input::GetKeyDown(GLFW_KEY_ESCAPE))
				return Status::Abort;

			const auto itemsPerPage = ItemsPerPage();
			auto& entity = GetRelevantEntity(mode);
			auto& items = entity.GetInventory()->items;
			auto firstIdxAtPage = pageIndex * itemsPerPage;
			auto itemsInPage = glm::min(itemsPerPage, int(items.size() - firstIdxAtPage));
			bool hasPrevPage = pageIndex != 0;
			bool hasNextPage = (firstIdxAtPage + itemsPerPage) < items.size();
			for (int i = 0; i < itemsInPage; ++i)
			{
				if (rlf::Input::GetKeyDown(GLFW_KEY_A + i))
				{
					auto doneSomething = inventoryAction(firstIdxAtPage + i, mode, entity);
					if (doneSomething)
						Game::Instance().EndTurn();
					isGuiDirty = true;
					if (entity.GetInventory()->items.empty())
						return Status::Abort;
				}
			}
			if (hasNextPage && rlf::Input::GetKeyDown(GLFW_KEY_KP_ADD))
			{
				pageIndex++;
				isGuiDirty = true;
			}
			if (hasPrevPage && rlf::Input::GetKeyDown(GLFW_KEY_KP_SUBTRACT))
			{
				pageIndex--;
				isGuiDirty = true;
			}
			return Status::Running;
		}

		void Inventory::Render()
		{
			const auto itemsPerPage = ItemsPerPage();
			auto& gfx = Graphics::Instance();
			auto& sparseBufferInv = gfx.RequestBuffer("inventory");
			if (!sparseBufferInv.IsInitialized())
				sparseBufferInv.Init(sizeof(uvec4), 2000);
			auto& sparseBufferHeader = gfx.RequestBuffer("header");
			if (!sparseBufferHeader.IsInitialized())
				sparseBufferHeader.Init(sizeof(uvec4), 200);

			auto& entity = GetRelevantEntity(mode);
			if (isGuiDirty)
			{
				isGuiDirty = false;
				
				bufferMain.resize(0);
				bufferHeader.resize(0);
				auto isPlayer = Game::Instance().IsPlayer(entity);
				auto& items = entity.GetInventory()->items;
				sortItems(items);
				auto numPages = (items.size() + itemsPerPage - 1) / itemsPerPage;
				auto firstIdxAtPage = pageIndex * itemsPerPage;
				auto itemsInPage = glm::min(itemsPerPage, int(items.size() - firstIdxAtPage));
				bool hasPrevPage = pageIndex != 0;
				bool hasNextPage = pageIndex < (numPages - 1);

				std::string inventoryModeText;
				switch (mode)
				{
				case rlf::state::Inventory::Mode::EquipOrUse:
					inventoryModeText = "Equip/Use";
					break;
				case rlf::state::Inventory::Mode::PickUp:
					inventoryModeText = "Pick up";
					break;
				case rlf::state::Inventory::Mode::Drop:
					inventoryModeText = "Drop";
					break;
				default:
					break;
				}
				auto rowStartAndNum = Graphics::Instance().RowStartAndNum("main");
				auto screenSize = gfx.ScreenSize();
				AddSeparatorLine(bufferHeader, 0, color::BROWN, screenSize.x, "Inventory: " + inventoryModeText);
				AddTextToLine(bufferMain, fmt::format("Total weight: {0} stones", entity.GetInventory()->Weight()), 0, rowStartAndNum.y-2, color::BROWN);
				int rowItems0 = rowStartAndNum.y - 4;
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
					const auto stackSize = item->GetItemData()->stackSize;
					if (stackSize > 1)
						text += fmt::format(" x{0}", stackSize);
					int pad = maxNameSize - item->Name().size();
					for (int i = 0; i < pad; ++i)
						text.push_back(' ');
					if (changedCategory)
						text += fmt::format(" [{0}]", magic_enum::enum_name(category));
					AddTextToLine(bufferMain, text, 0, rowItems0 - iItem, color::BROWN);
				}
				auto lastLine = fmt::format("[a-{0}] {1}", char('a' + itemsInPage - 1), inventoryModeText);
				if (hasPrevPage && hasNextPage)
					lastLine += " - [+/-] Next/previous page";
				else if (hasPrevPage)
					lastLine += " - [-] Previous page";
				if (hasNextPage)
					lastLine += " - [+] Next page";
				lastLine += fmt::format(" - Page {0}/{1} - [Esc] Exit inventory", pageIndex + 1, glm::max(int(numPages), 1));
				AddTextToLine(bufferMain, lastLine, 0, 1, color::BROWN);
				AddSeparatorLine(bufferMain, 0, color::BROWN, screenSize.x);
				// Update the GPU buffers
				sparseBufferInv.Set(bufferMain.size(), bufferMain.data());
				sparseBufferHeader.Set(bufferHeader.size(), bufferHeader.data());
			}
			
			Graphics::Instance().RenderGui();
			Graphics::Instance().RenderGameOverlay(sparseBufferInv);
			Graphics::Instance().RenderHeader();
		}
	}
}