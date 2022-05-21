#pragma once

#include <xhash>

namespace rlf
{
	class Entity; 

	// Serializable reference to an entity
	struct EntityId
	{
		int version = 0;
		int id = 0;

		bool operator == (const EntityId& other) const  { return id == other.id && version == other.version; }

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