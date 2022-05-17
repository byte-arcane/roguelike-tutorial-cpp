#pragma once

namespace rlf
{
	namespace state
	{
		class IState
		{
		public:
			// return true when done
			virtual bool update() = 0;
			virtual void render() = 0;
		};
	}
}