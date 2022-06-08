#pragma once

#include <string>
#include <unordered_map>

namespace rlf
{
	class EntityConfig;

	// A class that stores our configuration database, for spawning entities
	class Db
	{
	public:

		static Db& Instance() { static Db instance; return instance; }

		// Load the database from a .json file
		void LoadFromDisk();
		// Load the database from code (hard-code configurations)
		void LoadFromCode();

		// Get a configuration given a name. Return nullptr if name was not found
		const EntityConfig * Get(const std::string& name) const { return &db.at(name); }
		// Add a configuration dynamically
		void Add(const std::string& name, EntityConfig& cfg) { db.emplace(name, std::move(cfg)); }
		// Return the map of names -> configurations
		const std::unordered_map<std::string, EntityConfig>& All() const  { return db; }
		
	private:
		// The map of names -> configurations
		std::unordered_map<std::string, EntityConfig> db;
	};

	// A struct that can be used to access an entity configuration using its name. 
	struct DbIndex
	{
		// Construct an invalid object
		DbIndex() = default;
		// Construct an object using the configuration name
		DbIndex(const std::string& name) :name(name) {};
		// Check if this object is valid 
		bool IsValid() const  { return !name.empty() && Cfg() != nullptr; }
		// Get the configuration as a const pointer
		const EntityConfig* Cfg() const { return Db::Instance().Get(name); }

		bool operator == (const DbIndex& other) const  { return name == other.name; }

		std::string name;
	};
}