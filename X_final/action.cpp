#include "action.h"

#include "entity.h"
#include "game.h"
#include "eventhandlers.h"

namespace rlf
{
	// The actual actions
	void Move()
	{
		// todo: put everything into ActionData. Register action (and set "waitingForPlayer" to false) and execute the command
	}

	void Handle()
	{
		// calc handle targets
		// if a single target, just like move above
		// if many targets:
		//	push a new target selection state with all positions (and given euclidean range)
		// onSuccess: register action and execute command
		// onFail (e.g. Escape without having chosen): invalidate current action type in ActionData, so that we go again from the beginning
	}

	void PickUp()
	{
		// if there's stuff underneath
		//	push new inventory state
		// onAction: register action and execute command
		// onFail (e.g. Escape without having chosen): invalidate current action type in ActionData, so that we go again from the beginning
	}

	void Drop()
	{

	}

	void EquipUse()
	{

	}

	void RangedAttack()
	{

	}

	void ExecuteAction(ActionType type)
	{
		switch (type)
		{
		case ActionType::Move:
			Move();
			break;
		case ActionType::Handle:
			Handle();
			break;
		case ActionType::EquipUse:
			EquipUse();
			break;
		case ActionType::PickUp:
			PickUp();
			break;
		case ActionType::Drop:
			Drop();
			break;
		case ActionType::RangedAttack:
			RangedAttack();
			break;
		default:
			assert(false);
			break;
		}
	}
}