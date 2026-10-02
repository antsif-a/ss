#pragma once

#include "enemy.hpp"

namespace biv {
	class JumpableEnemy : public Enemy {
		protected:
			static constexpr float ENEMY_JUMP_SPEED = -0.7f;
			static constexpr int JUMP_INTERVAL = 30;

			int frames_on_ground = 0;

		public:
			JumpableEnemy(const Coord& top_left, const int width, const int height);

			void move_vertically() noexcept override;

		protected:
			void stand_on(Rect*) noexcept override;
	};
}
