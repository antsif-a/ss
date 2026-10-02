/**
	- Покажите на диаграмме иерархию наследования для класса ConsoleEnemy.
*/

#pragma once

#include "console_ui_obj_rect_adapter.hpp"
#include "walking_enemy.hpp"

namespace biv {
	class ConsoleEnemy : public WalkingEnemy, public ConsoleUIObjectRectAdapter {
		public:
			ConsoleEnemy(const Coord& top_left, const int width, const int height);

			char get_brush() const noexcept override;
	};
}