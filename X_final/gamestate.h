#pragma once

namespace rlf
{
	// The game's state
	class IState
	{
	public:
		virtual void Update() = 0;
		virtual void Render() = 0;
	};

	class MenuState : IState
	{
		void Update() override;
		void Render() override;
	};

	class GameState : IState
	{
		void Update() override;
		void Render() override;
	};

	class InventoryState : IState
	{
		void Update() override;
		void Render() override;
	};
}