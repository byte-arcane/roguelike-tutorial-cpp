#pragma once

#include <nano_signal_slot.hpp>

namespace rlf
{
	class Entity;
	class Level;
	struct sig
	{
		// Declare Nano::Signals using function signature syntax
		static Nano::Signal<void(const Entity&)> onEntityMoved;
		static Nano::Signal<void(Entity&)> onEntityAdded;
		static Nano::Signal<void(Entity&)> onEntityRemoved;
		static Nano::Signal<void(const Level&)> onLevelChanged;
		static Nano::Signal<void()> onFogOfWarChanged;
		static Nano::Signal<void(const Entity&)> onObjectStateChanged;
		static Nano::Signal<void()> onPlayerDied;
		static Nano::Signal<void()> onGuiUpdated;
		static Nano::Signal<void()> onGameLoaded;
	};
}