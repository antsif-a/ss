/**
	- Если создавать класс FlyableEenemy, JumpableEnemy и другие виды врагов, 
		то почему их нельзя наследовать от Enemy?
*/

#pragma once

#include "collisionable.hpp"
#include "movable.hpp"
#include "rect.hpp"
#include "rect_map_movable_adapter.hpp"
#include "speed.hpp"

namespace biv {
	class Enemy : public RectMapMovableAdapter, public Movable, public Collisionable {
		protected:
			Enemy(
				const Coord& top_left, 
				const int width, const int height, 
				const float hspeed
			);

			virtual void stand_on(Rect*) noexcept = 0;

		public:
			Rect get_rect() const noexcept override;
			Speed get_speed() const noexcept override;

			void process_horizontal_static_collision(Rect*) noexcept override;
			void process_mario_collision(Collisionable*) noexcept override;
			void process_vertical_static_collision(Rect*) noexcept override;
	};
}