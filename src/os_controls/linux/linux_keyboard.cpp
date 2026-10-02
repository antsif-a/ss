#include "linux_keyboard.hpp"

#include <cstdlib>
#include <filesystem>

#include <fcntl.h>
#include <linux/input.h>
#include <ncurses.h>
#include <unistd.h>

using biv::LinuxKeyboard;

LinuxKeyboard::LinuxKeyboard() : device_path(find_device_path()) {}

LinuxKeyboard::~LinuxKeyboard() {
	if (input_fd >= 0) {
		close(input_fd);
	}
}

biv::UserInput LinuxKeyboard::get_user_input() {
	if (input_fd < 0) {
		return get_terminal_input();
	}
	
	poll_events();
	// Символы нажатых клавиш всё равно попадают в буфер терминала.
	flushinp();
	
	if (is_pressed(KEY_Q)) {
		return UserInput::EXIT;
	} else if (is_pressed(KEY_SPACE)) {
		return UserInput::MARIO_JUMP;
	} else if (is_pressed_or_repeated(KEY_A)) {
		return UserInput::MAP_RIGHT;
	} else if (is_pressed_or_repeated(KEY_D)) {
		return UserInput::MAP_LEFT;
	}
	return UserInput::NO_INPUT;
}

void LinuxKeyboard::on() {
	if (input_fd < 0) {
		input_fd = open(device_path.c_str(), O_RDONLY | O_NONBLOCK);
	}
}

void LinuxKeyboard::off() {
	key_states.clear();
	prev_input = UserInput::NO_INPUT;
}

// ----------------------------------------------------------------------------
// 									PRIVATE
// ----------------------------------------------------------------------------
std::string LinuxKeyboard::find_device_path() {
	if (const char* path = std::getenv("MARIO_KEYBOARD")) {
		return path;
	}
	
	// udev создаёт для клавиатур ссылки вида /dev/input/by-path/*-event-kbd.
	std::error_code error;
	for (
		const auto& entry: 
		std::filesystem::directory_iterator("/dev/input/by-path", error)
	) {
		const std::string name = entry.path().filename().string();
		const std::string suffix = "-event-kbd";
		if (
			name.size() >= suffix.size() && 
			name.compare(name.size() - suffix.size(), suffix.size(), suffix) == 0
		) {
			return entry.path().string();
		}
	}
	return "/dev/input/event2";
}

biv::UserInput LinuxKeyboard::get_terminal_input() {
	int c = getch();
	
	switch (c) {
		case 'd':
			prev_input = UserInput::MAP_LEFT;
			return UserInput::MAP_LEFT;
		case 'a':
			prev_input = UserInput::MAP_RIGHT;
			return UserInput::MAP_RIGHT;
		case 's':
			prev_input = UserInput::NO_INPUT;
			return UserInput::NO_INPUT;
		case ' ':
			return UserInput::MARIO_JUMP;
		case 'q':
			return UserInput::EXIT;
	}
    
	switch (prev_input) {
		case UserInput::MAP_LEFT:
			return UserInput::MAP_LEFT;
		case UserInput::MAP_RIGHT:
			return UserInput::MAP_RIGHT;
		default:
			return UserInput::NO_INPUT;
	}
}

bool LinuxKeyboard::is_pressed(const unsigned short key) {
	return key_states[key] == KeyState::PRESS;
}

bool LinuxKeyboard::is_pressed_or_repeated(const unsigned short key) {
	return (
		key_states[key] == KeyState::PRESS || 
		key_states[key] == KeyState::REPEAT
	);
}

void LinuxKeyboard::poll_events() {
	input_event ev = {};
	while (read(input_fd, &ev, sizeof ev) == sizeof ev) {
		if (ev.type == EV_KEY) {
			key_states[ev.code] = static_cast<KeyState>(ev.value);
		}
	}
}
