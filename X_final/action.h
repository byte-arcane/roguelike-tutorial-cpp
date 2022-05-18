#pragma once

#include <glm/glm.hpp>
#include "entityid.h"

namespace rlf
{
	enum class ActionType : int
	{
		Move=0,
		Handle, // e.g. staircase, or even open adjacent doors ()
		RangedAttack,
		PickUp,
		Drop,
		EquipUse,
	};

	struct ActionData
	{
		ActionType type = ActionType(-1);
		glm::ivec2 targetPosition = { -1,-1 };
		EntityId targetEntityId;

		bool IsValid() const { return int(type) >= 0; }
	};

	void ExecuteAction(ActionType type);
}