#pragma once

#include <deque>

#include "entityid.h"

namespace rlf
{
	// Entities have "energy recovery rate". When they take an action, they spend energy that they have to recover first before playing again
	class TurnSystem
	{
	public:
		// This is called every frame, and processes all turns until it reaches the player
		void Process();
		void SetWaitingForPlayerAction(bool value)  { waitingForPlayerAction = value; }
		bool WaitingForPlayerAction() const  { return waitingForPlayerAction; }
	private:
		bool waitingForPlayerAction = true;
	};
}