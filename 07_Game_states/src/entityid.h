#pragma once

#include <xhash>

namespace rlf
{
	class Entity; 

	// Serializable reference to an entity
	struct EntityId
	{
		// the id is the index in the pool where all entities are stored
		int id = 0;
		// the version is incremented every time we re-use a pool element to allocate a new entity (when an old one at that position gets destroyed)
		int version = 0;

		bool operator == (const EntityId& other) const  { return id == other.id && version == other.version; }

		// get the entity pointer, return nullptr if it's not active anymore
		Entity* Entity() const;
	};
}

// Simple hash object for EntityId, https://en.cppreference.com/w/cpp/utility/hash
template<>
struct std::hash<rlf::EntityId>
{
	std::size_t operator()(rlf::EntityId const& eid) const 
	{
		const std::size_t h1 = std::hash<int>{}(eid.id);
		const std::size_t h2 = std::hash<int>{}(eid.version);
		return h1 ^ (h2 << 1); // or use boost::hash_combine
	}
};