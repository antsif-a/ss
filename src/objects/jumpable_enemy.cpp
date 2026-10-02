#include "jumpable_enemy.hpp"

using biv::JumpableEnemy;

JumpableEnemy::JumpableEnemy(
	const Coord& top_left, const int width, const int height
) : Enemy(top_left, width, height, 0) {}

void JumpableEnemy::move_vertically() noexcept {
	// vspeed == 0 только после приземления: враг стоит и ждёт следующего прыжка.
	// Сбитый враг больше не прыгает, а просто падает вниз.
	if (is_active() && vspeed == 0 && ++frames_on_ground >= JUMP_INTERVAL) {
		frames_on_ground = 0;
		vspeed = ENEMY_JUMP_SPEED;
	}
	Movable::move_vertically();
}

// Прыгающий враг не ходит, поэтому стоять на месте ему достаточно.
void JumpableEnemy::stand_on(Rect* obj) noexcept {}
