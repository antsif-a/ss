#pragma once

#include "enemy.hpp"

namespace biv {
	class WalkingEnemy : public Enemy {
		public:
			WalkingEnemy(const Coord& top_left, const int width, const int height);

		protected:
			void stand_on(Rect*) noexcept override;
	};
}