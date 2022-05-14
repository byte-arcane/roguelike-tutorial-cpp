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

		const EntityConfig * Get(const std::string& name) const { return &db.at(name); }
		void Add(const std::string& name, const EntityConfig& cfg) { db.emplace(name, cfg); }
		
	private:
		std::unordered_map<std::string, EntityConfig> db;
	};

	struct DbIndex
	{
		DbIndex() = default;
		DbIndex(const std::string& name) :name(name) {};
		bool IsValid() const { return !name.empty(); }
		const EntityConfig* Cfg() const { return Db::Instance().Get(name); }

		std::string name;
	};
}