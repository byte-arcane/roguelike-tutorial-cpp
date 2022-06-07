#pragma once

#include <deque>

#include "entityid.h"

namespace rlf
{
	// An extremely simple turn ordering system: the player plays first, then all enemies in level play.
	class TurnSystem
	{
	public:
		// This is called every frame, and processes all turns until it reaches the player,where it waits until a player marks themselves as ready
		void Process();
		
		// Accessors
		void SetWaitingForPlayerAction(bool value)  { waitingForPlayerAction = value; }
		bool WaitingForPlayerAction() const  { return waitingForPlayerAction; }
	private:
		// set this to true if Process should NOT iterate over enemies, but it should wait until player is done, e.g. with selecting a target from the gui
		bool waitingForPlayerAction = true;
	};
}