#include "signals.h"

namespace rlf
{
	Nano::Signal<void(const Entity&)> sig::onEntityMoved;
	Nano::Signal<void(Entity&)> sig::onEntityAdded;
	Nano::Signal<void(Entity&)> sig::onEntityRemoved;
	Nano::Signal<void(const Level&)> sig::onLevelChanged;
	Nano::Signal<void()> sig::onFogOfWarChanged;
	Nano::Signal<void(const Entity&)> sig::onObjectStateChanged;
	Nano::Signal<void()> sig::onGuiUpdated;
	Nano::Signal<void()> sig::onPlayerDied;
	Nano::Signal<void()> sig::onGameLoaded;
}