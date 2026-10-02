#include "enemy.hpp"

#include "map_movable.hpp"

using biv::Enemy;

Enemy::Enemy(
	const Coord& top_left, 
	const int width, const int height, 
	const float hspeed
) : RectMapMovableAdapter(top_left, width, height) {
	vspeed = 0;
	this->hspeed = hspeed;
}

biv::Rect Enemy::get_rect() const noexcept {
	return {top_left, width, height};
}

biv::Speed Enemy::get_speed() const noexcept {
	return {vspeed, hspeed};
}

void Enemy::process_horizontal_static_collision(Rect* obj) noexcept {
	hspeed = -hspeed;
	move_horizontally();
}

void Enemy::process_mario_collision(Collisionable* mario) noexcept {
	if (mario->get_speed().v > 0 && mario->get_speed().v != V_ACCELERATION) {
		kill();
	} else {
		mario->kill();
	}
}

void Enemy::process_vertical_static_collision(Rect* obj) noexcept {
	stand_on(obj);
	
	if (vspeed > 0) {
		top_left.y -= vspeed;
		vspeed = 0;
	}
}