#pragma once

#include "enemy.hpp"

namespace biv {
	class FlyableEnemy : public Enemy {
		protected:
			const float left_bound;
			const float right_bound;
			const float origin_y;
			const float amplitude;
			float phase = 0;

		public:
			FlyableEnemy(
				const Coord& top_left, 
				const int width, const int height, 
				const int right_bound, const int amplitude
			);

			void move_horizontally() noexcept override;
			void move_vertically() noexcept override;

		protected:
			void stand_on(Rect*) noexcept override;
	};
}