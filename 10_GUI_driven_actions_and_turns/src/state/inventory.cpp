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
#include "../commands.h"

using namespace std;
using namespace glm;

namespace rlf
{
	namespace color // Ideally move this out of here, somewhere where we have a list of common colors, e.g. terminal colors
	{
		constexpr glm::vec4 BROWN(1, .545, 0, 1);
	}

	namespace state
	{
		// We need 6 rows for padding things nicely, but a maximum of 26 (a-z)
		int ItemsPerPage() 
		{ 
			return glm::min(Graphics::Instance().RowStartAndNum("main").y - 6,26);
		}; 	

		// Sort the list of items based on their category first, and their name later
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

		// Take an action on an inventory item, based on inventory mode
		bool inventoryAction(int itemIdx, Inventory::Mode inventoryMode, Entity& entity)
		{
			bool actionTaken = true;
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
					auto oldEquippedIdx = entity.GetInventory()->EquippedItemAtSlot(itemCategory);
					ChangeEquippedItem(entity, itemIdx, oldEquippedIdx);
				}
				else if (itemCategory == ItemCategory::Consumable)
					UseItem(entity, itemIdx);
				else
					actionTaken = false;
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
						actionTaken = false;
				}
				else
					PickUp(*player.Entity(), entity, items[itemIdx]);
			}
			return actionTaken;
		}

		// Based on the mode, get the relevant inventory entity
		// if we drop or equip or use, we want to show the player inventory
		// if we want to pick up stuff, we want to show the inventory of the item pile we're over
		Entity& GetRelevantEntity(Inventory::Mode mode)
		{
			auto player = Game::Instance().PlayerId().Entity();
			if (mode != Inventory::Mode::PickUp)
				return *player;
			else
			{
				// in the future, this could be an adjacent container that we interact with, not just the item pile! But we'd need a hint for its location
				auto itemPile = Game::Instance().CurrentLevel().GetEntity(player->GetLocation().position, false);
				assert(itemPile->DbCfg() == DbIndex::ItemPile());
				return *itemPile;
			}
		}

		Status Inventory::UpdateImpl()
		{
			// ESC goes back to the game mode
			if (rlf::Input::GetKeyDown(GLFW_KEY_ESCAPE))
				return Status::Abort;

			// if we die while we're in the inventory, return!
			if( Game::Instance().PlayerId().Entity()->GetCreatureData()->hp <= 0)
				return Status::Abort;

			const auto itemsPerPage = ItemsPerPage();
			auto& entity = GetRelevantEntity(mode);
			auto& items = entity.GetInventory()->items;
			// calc the item index that corresponds to the first selectable option
			auto firstIdxAtPage = pageIndex * itemsPerPage;
			auto itemsInPage = glm::min(itemsPerPage, int(items.size() - firstIdxAtPage));
			bool hasPrevPage = pageIndex != 0;
			bool hasNextPage = (firstIdxAtPage + itemsPerPage) < items.size();
			// Check if we press any key that corresponds to an item in the page. 
			for (int i = 0; i < itemsInPage; ++i)
			{
				if (rlf::Input::GetKeyDown(GLFW_KEY_A + i))
				{
					//run the action(if any), and if we did run an action, end the turn.
					auto doneSomething = inventoryAction(firstIdxAtPage + i, mode, entity);
					if (doneSomething)
						Game::Instance().EndTurn();
					isGuiDirty = true;
					// Also, if the inventory is now empty, no point in being here, so go back to main game state
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
			// Get the buffers to write to
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

				// Write the header first
				bufferHeader.resize(0);
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
				auto screenSize = gfx.ScreenSize();
				AddSeparatorLine(bufferHeader, 0, color::BROWN, screenSize.x, "Inventory: " + inventoryModeText);
				
				// Now write the main buffer
				bufferMain.resize(0);
				auto isPlayer = Game::Instance().IsPlayer(entity);
				auto& items = entity.GetInventory()->items;
				sortItems(items);
				auto numPages = (items.size() + itemsPerPage - 1) / itemsPerPage;
				auto firstIdxAtPage = pageIndex * itemsPerPage;
				auto itemsInPage = glm::min(itemsPerPage, int(items.size() - firstIdxAtPage));
				bool hasPrevPage = pageIndex != 0;
				bool hasNextPage = pageIndex < (numPages - 1);				

				auto rowStartAndNum = Graphics::Instance().RowStartAndNum("main");
				// Write a line 2 rows under the header, about weight
				AddTextToLine(bufferMain, fmt::format("Total weight: {0} stones", entity.GetInventory()->Weight()), 0, rowStartAndNum.y-2, color::BROWN);
				// 2 rows later, start listing the items
				int rowItems0 = rowStartAndNum.y - 4;
				ItemCategory category = ItemCategory(-1);
				const int maxNameSize = 40; // use this for column alignment
				for (int iItem = 0; iItem < itemsInPage; ++iItem)
				{
					const auto& item = items[iItem + firstIdxAtPage].Entity();
					auto isEquipped = item->GetItemData()->equipped;
					auto newCategory = item->DbCfg().Cfg()->itemCfg.category;
					bool changedCategory = category != newCategory;
					category = newCategory;

					std::string text;
					text.push_back('a' + iItem); // the actual letter
					text += isEquipped ? ") [E] " : ")     "; // different tail text if it's equipped or not (same length though)
					text += item->Name();
					const auto stackSize = item->GetItemData()->stackSize;
					// add an indicator if it's a stack w/ >1 items
					if (stackSize > 1)
						text += fmt::format(" x{0}", stackSize);
					// add padding
					int pad = maxNameSize - item->Name().size();
					for (int i = 0; i < pad; ++i)
						text.push_back(' ');
					// write the category if it differs from the last item
					if (changedCategory)
						text += fmt::format(" [{0}]", magic_enum::enum_name(category));
					AddTextToLine(bufferMain, text, 0, rowItems0 - iItem, color::BROWN);
				}
				// Add the bottom line that tells the player which chars they can press
				auto lastLine = fmt::format("[a-{0}] {1}", char('a' + itemsInPage - 1), inventoryModeText);
				if (hasPrevPage && hasNextPage)
					lastLine += " - [+/-] Next/previous page";
				else if (hasPrevPage)
					lastLine += " - [-] Previous page";
				if (hasNextPage)
					lastLine += " - [+] Next page";
				lastLine += fmt::format(" - Page {0}/{1} - [Esc] Exit inventory", pageIndex + 1, glm::max(int(numPages), 1));
				AddTextToLine(bufferMain, lastLine, 0, 1, color::BROWN);
				// Add a separator line at the bottom
				AddSeparatorLine(bufferMain, 0, color::BROWN, screenSize.x);
				// Update the GPU buffers
				sparseBufferInv.Set(bufferMain.size(), bufferMain.data());
				sparseBufferHeader.Set(bufferHeader.size(), bufferHeader.data());
			}
			
			// Render the character info, the inventory, and the header
			Graphics::Instance().RenderGui();
			Graphics::Instance().RenderGameOverlay(sparseBufferInv);
			Graphics::Instance().RenderHeader();
		}
	}
}