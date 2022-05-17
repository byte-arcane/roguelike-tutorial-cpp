#pragma once

#include <string>
#include <unordered_map>

namespace rlf
{
	class EntityConfig;

	class Db
	{
	public:
		static Db& Instance() { static Db instance; return instance; }

		void LoadFromDisk();

		const EntityConfig * Get(const std::string& name) const { return &db.at(name); }
		void Add(const std::string& name, EntityConfig& cfg) { db.emplace(name, std::move(cfg)); }
		const std::unordered_map<std::string, EntityConfig>& All() const { return db; }
		
	private:
		std::unordered_map<std::string, EntityConfig> db;
	};

	struct DbIndex
	{
		DbIndex() = default;
		DbIndex(const std::string& name) :name(name) {};
		bool IsValid() const { return !name.empty(); }
		bool operator == (const DbIndex& other) const { return name == other.name; }
		const EntityConfig* Cfg() const { return Db::Instance().Get(name); }

		// Special ones
		static const DbIndex Door() { return DbIndex("door"); }
		static const DbIndex StairsUp() { return DbIndex("stairs_up"); }
		static const DbIndex StairsDown() { return DbIndex("stairs_down"); }
		static const DbIndex ItemPile() { return DbIndex("item_pile"); }

		std::string name;
	};
}