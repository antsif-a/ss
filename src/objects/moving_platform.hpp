#pragma once

#include "box.hpp"
#include "movable.hpp"

namespace biv {
	class MovingPlatform : public Box, public Movable {
		protected:
			static constexpr int STEP_INTERVAL = 8;
			static constexpr float STEP = 1.0f;

			Movable* rider;
			const float left_bound;
			const float right_bound;
			int frames_left = STEP_INTERVAL - 1;
			int direction = 1;

			bool is_rider_standing() const noexcept;
			void step_sideways() noexcept;

		public:
			MovingPlatform(
				const Coord& top_left, 
				const int width, const int height, 
				const int right_bound, Movable* rider
			);

			void move_horizontally() noexcept override;
			void move_vertically() noexcept override;
	};
}