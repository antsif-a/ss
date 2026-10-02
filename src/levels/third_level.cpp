#include "third_level.hpp"

using biv::ThirdLevel;

ThirdLevel::ThirdLevel(UIFactory* ui_factory) : GameLevel(ui_factory) {
	init_data();
}

bool ThirdLevel::is_final() const noexcept {
	return true;
}

biv::GameLevel* ThirdLevel::get_next() {
	return next;
}

// ----------------------------------------------------------------------------
// 									PROTECTED
// ----------------------------------------------------------------------------
void ThirdLevel::init_data() {
	// Марио создаётся первым: движущиеся платформы запоминают его как пассажира.
	ui_factory->create_mario({39, 10}, 3, 3);
	
	// Стартовый корабль.
	ui_factory->create_ship({20, 25}, 30, 2);
	ui_factory->create_full_box({28, 16}, 5, 3);
	
	// Платформа над морем, её охраняет летающий враг.
	ui_factory->create_moving_platform({50, 25}, 10, 2, 75);
	ui_factory->create_flyable_enemy({55, 18}, 3, 2, 80, 3);
	
	// Узкий столб, с которого нужно запрыгнуть на высокую платформу.
	ui_factory->create_ship({90, 22}, 6, 5);
	ui_factory->create_moving_platform({100, 18}, 8, 2, 125);
	ui_factory->create_flyable_enemy({105, 11}, 3, 2, 128, 3);
	
	// Корабль с врагом и ящиком с монетой.
	ui_factory->create_ship({140, 25}, 20, 2);
	ui_factory->create_full_box({148, 16}, 5, 3);
	ui_factory->create_enemy({150, 5}, 3, 2);
	
	// Две платформы подряд: низкая и высокая, между ними летают враги.
	ui_factory->create_moving_platform({160, 25}, 8, 2, 180);
	ui_factory->create_flyable_enemy({185, 14}, 3, 2, 200, 3);
	ui_factory->create_moving_platform({195, 19}, 8, 2, 215);
	ui_factory->create_flyable_enemy({200, 13}, 3, 2, 220, 2);
	
	ui_factory->create_ship({235, 19}, 8, 8);
	
	// Финиш: последний статический объект уровня.
	ui_factory->create_ship({255, 22}, 25, 5);
}
