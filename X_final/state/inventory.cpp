#include "inventory.h"

#include <array>
#include <algorithm>
#include <unordered_map>

#include <GLFW/glfw3.h>
#include <fmt/format.h>

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

			if (centeredText.empty())
				for (int i = 0; i < numCols; ++i)
					buf.push_back(dashTileData.PackSparse({ i,row }));
			else
			{
				auto numColsEachSide = (numCols - (centeredText.size() + 2)) / 2;
				for (int i = 0; i < numColsEachSide; ++i)
				{
					buf.push_back(dashTileData.PackSparse({ i,row }));
					buf.push_back(dashTileData.PackSparse({ numCols - 1 - i,row }));
				}
				for (int i = 0; i<int(centeredText.size()); ++i)
				{
					auto c = centeredText[i];
					if (c != ' ')
					{
						TileData td{ c, color };
						buf.push_back(td.PackSparse({ numColsEachSide + 1 + i,row }));
					}
				}
			}
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

		void inventoryAction(int itemIdx, Inventory::Mode inventoryMode, Entity& entity)
		{
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
			}
			else // pick up or drop
			{
				auto player = GameState::Instance().Player();
				auto isPlayer = GameState::Instance().IsPlayer(entity);
				if (isPlayer)
				{
					// can't drop equipped items!
					if (!item->GetItemData()->equipped)
						Drop(entity, items[itemIdx]);
				}
				else
					PickUp(*player.Entity(), entity, items[itemIdx]);
			}
		}

		Entity& GetRelevantEntity(Inventory::Mode mode)
		{
			auto player = GameState::Instance().Player().Entity();
			if (mode != Inventory::Mode::PickUp)
				return *player;
			else
			{
				auto itemPile = GameState::Instance().CurrentLevel().GetEntity(player->GetLocation().position, false);
				assert(itemPile->DbCfg() == DbIndex::ItemPile());
				return *itemPile;
			}
		}

		bool Inventory::update()
		{
			auto& entity = GetRelevantEntity(mode);
			auto& items = entity.GetInventory()->items;
			auto firstIdxAtPage = pageIndex * ITEMS_PER_PAGE;
			auto itemsInPage = glm::min(ITEMS_PER_PAGE, int(items.size() - firstIdxAtPage));
			bool hasPrevPage = pageIndex != 0;
			bool hasNextPage = (firstIdxAtPage + ITEMS_PER_PAGE) < items.size();
			for (int i = 0; i < itemsInPage; ++i)
			{
				if (cgf::Input::GetKeyDown(GLFW_KEY_A + i))
				{
					inventoryAction(firstIdxAtPage + i, mode, entity);
					isGuiDirty = true;
				}
			}
			if (hasNextPage && cgf::Input::GetKeyDown(GLFW_KEY_KP_ADD))
			{
				pageIndex++;
				isGuiDirty = true;
			}
			if (hasPrevPage && cgf::Input::GetKeyDown(GLFW_KEY_KP_SUBTRACT))
			{
				pageIndex--;
				isGuiDirty = true;
			}
			if (cgf::Input::GetKeyDown(GLFW_KEY_ESCAPE))
			{
				pageIndex = -1;
				return true;
			}
			return false;
		}

		void Inventory::render()
		{
			auto& entity = GetRelevantEntity(mode);
			if (isGuiDirty)
			{
				isGuiDirty = false;
				
				buffer.resize(0);
				auto isPlayer = GameState::Instance().IsPlayer(entity);
				auto& items = entity.GetInventory()->items;
				sortItems(items);
				auto numPages = (items.size() + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE;
				auto firstIdxAtPage = pageIndex * ITEMS_PER_PAGE;
				auto itemsInPage = glm::min(ITEMS_PER_PAGE, int(items.size() - firstIdxAtPage));
				bool hasPrevPage = pageIndex != 0;
				bool hasNextPage = pageIndex < (numPages - 1);

				std::string inventoryModeText;
				switch (mode)
				{
				case rlf::state::Inventory::EquipOrUse:
					inventoryModeText = "Equip/Use";
					break;
				case rlf::state::Inventory::PickUp:
					inventoryModeText = "Pick up";
					break;
				case rlf::state::Inventory::Drop:
					inventoryModeText = "Drop";
					break;
				default:
					break;
				}
				addSeparatorLine(buffer, guiNumRows + gameGridSize.y - 1, color::BROWN, gameGridSize.x, "Inventory: " + inventoryModeText);
				addTextToLine(buffer, fmt::format("Total weight: {0} stones", entity.GetInventory()->Weight()), 0, guiNumRows + gameGridSize.y - 3, color::BROWN);
				int rowItems0 = guiNumRows + gameGridSize.y - 5;
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
					addTextToLine(buffer, text, 0, rowItems0 - iItem, color::BROWN);
				}
				auto lastLine = fmt::format("[a-{0}] {1}", char('a' + itemsInPage - 1), inventoryModeText);
				if (hasPrevPage && hasNextPage)
					lastLine += " - [+/-] Next/previous page";
				else if (hasPrevPage)
					lastLine += " - [-] Previous page";
				if (hasNextPage)
					lastLine += " - [+] Next page";
				lastLine += fmt::format(" - Page {0}/{1} - [Esc] Exit inventory", pageIndex + 1, glm::max(int(numPages), 1));
				addTextToLine(buffer, lastLine, 0, guiNumRows, color::BROWN);
				addSeparatorLine(buffer, guiNumRows - 1, color::BROWN, gameGridSize.x);
				// TODO: save to a umap<string,sparsebuffer> w/ name=="inventory"
			}

			// TODO: render
			assert(false);
			Graphics::Instance().BeginRender();
			Graphics::Instance().RenderGui();
			Graphics::Instance().EndRender();

			/*
			*	Examples:
			*	BeginRender();
			*		RenderGui(rect);
			*		RenderGameView
			*		RenderGameOverlay(rect, inventorySparseBuffer)
			*	EndRender();
			*/
		}
	}
}