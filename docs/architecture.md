# Создание и хранение объектов игры: `ConsoleUIFactory` и `Game`

Задание 2 из `README.md`:

> Изучить создание и хранение объектов игры на примере классов:
> ConsoleUIFactory, Game.

---

## 1. Краткий ответ

`Game` и `ConsoleUIFactory` хранят **одни и те же объекты дважды, но в
разных ролях**:

- **`Game`** — хранит объекты **как абстракции**, разложенные по
  контейнерам согласно их *функции в игре* (движется / сталкивается /
  является препятствием). Ему не нужно знать, что это за объект, — нужен
  лишь интерфейс, через который он дёргает поведение.
- **`ConsoleUIFactory`** — хранит объекты **как конкретные типы**, чтобы
  иметь возможность их `delete`. Это его единственная дополнительная
  роль: владение памятью.

Ни один из них не является владельцем «всего». Владение распределено так:

| Владелец | Что хранит | Зачем |
|---|---|---|
| `ConsoleUIFactory` | конкретные `Console*`-указатели | `delete` при `clear_data()` |
| `Game` | указатели на абстракции | выполнение поведения |
| `ConsoleGameMap` | `ConsoleUIObject*` | отрисовка |

---

## 2. `Game`: абстракции, разложенные по ролям

`src/controller/game.hpp`:

```cpp
class Game {
    private:
        std::vector<MapMovable*>  map_movable_objs;    // сдвигается при прокрутке
        std::vector<Rect*>        static_objs;        // неподвижная геометрия уровня
        std::vector<Collisionable*> collisionable_objs; // участвуют в столкновениях
        std::vector<Movable*>     movable_objs;       // двигаются сами каждый кадр

        Mario* mario = nullptr;   // особый случай, вынесен отдельно
        bool is_finished_  = false;
        bool is_level_end_ = false;
};
```

Четыре вектора — это **не** «четыре вида объектов». Это **четыре
категории участия в игровом цикле**. Один и тот же объект может попасть
сразу в несколько.

### Роли и то, как они используются

| Контейнер | Кто его обходит | Зачем |
|---|---|---|
| `map_movable_objs` | `move_map_left()` / `move_map_right()` | сдвинуть мир, когда игрок идёт |
| `movable_objs` | `move_objs_horizontally()` / `move_objs_vertically()` | собственное движение (гравитация, ходьба) |
| `collisionable_objs` | все три проверки столкновений | «кто вообще может в кого врезаться» |
| `static_objs` | `check_*_static_collisions()` | о что разбиваться |
| `mario` | `check_mario_collision()`, `check_static_collisions()` | особые правила: смерть, финиш, зондирование |

### Матрица регистрации

Снята непосредственно из `src/ui/console/console_ui_factory.cpp`:

| Объект | `map_movable` | `movable` | `collisionable` | `static` | `game_map` | вектор фабрики |
|---|:---:|:---:|:---:|:---:|:---:|---|
| `ConsoleBox` | ✓ | | | ✓ | ✓ | `boxes` |
| `ConsoleFullBox` | ✓ | | ✓ | ✓ | ✓ | `full_boxes` |
| `ConsoleShip` | ✓ | | | ✓ | ✓ | `ships` |
| `ConsoleEnemy` | ✓ | ✓ | ✓ | | ✓ | `enemies` |
| `ConsoleMoney` | ✓ | ✓ | ✓ | | ✓ | `moneys` |
| `ConsoleMario` | | ✓ | ✓ | | ✓ | `mario` (не вектор) |

Читается безупречно:

- **Платформы** (`Ship`, `Box`, `FullBox`) — статичны: `static` + `map_movable`.
- **Динамические существа** (`Enemy`, `Money`) — `movable` + `collisionable` + `map_movable`.
- **Марио** — `movable` + `collisionable`, но **не** `map_movable`.

### Почему Марио не `map_movable`?

Это ключевой момент. При прокрутке `Game::move_map_left()` сдвигает **мир**,
а не игрока. Марио должен остаться на экране. Компенсация делается
вручную в `main.cpp`:

```cpp
case biv::UserInput::MAP_LEFT:
    mario->move_map_left();                       // x += MAP_STEP
    if (!game.check_static_collisions(mario)) {   // чистое зондирование
        game.move_map_left();                     // сдвинуть весь мир
    }
    mario->move_map_right();                      // x -= MAP_STEP  (всегда!)
    break;
```

Обратите внимание: `mario->move_map_right()` стоит **вне** `if`. Смещение
Марио восстанавливается в ноль **каждый кадр**. Поэтому:

- `Mario` вообще не меняет свою абсолютную координату `x` от ввода;
- `Mario::process_horizontal_static_collision` — **мёртвый код**:
  он делает `hspeed = -hspeed`, а `hspeed` у Марио всегда `0`;
- стены не «отталкивают» Марио — они **блокируют прокрутку**. Если
  `check_static_collisions()` вернул `true`, не происходит вообще ничего.

Сравните с врагом, который действительно двигается сам и отскакивает:

```cpp
void Enemy::process_horizontal_static_collision(Rect* obj) noexcept {
    hspeed = -hspeed;      // развернуться
    move_horizontally();   // и отступить на шаг назад
}
```

**Вывод для защиты:** движение Марио реализовано как прокрутка мира с
проверкой на проходимость, а не как перемещение персонажа. Это сдвигает
ответственность с системы столкновений на игровой цикл.

---

## 3. `ConsoleUIFactory`: создание, владение, отвязка от представления

`UIFactory` (`src/model/ui_factory.hpp`) — абстрактный интерфейс с
чистыми виртуальными методами `create_box`, `create_enemy`,
`create_full_box`, `create_mario`, `create_money`, `create_ship`,
`get_game_map`, `get_mario`, `clear_data`. Это **паттерн Factory** в
форме абстрактной фабрики: конкретная фабрика создаёт конкретные объекты.

`ConsoleUIFactory` делает ровно три вещи при создании объекта:

```cpp
void ConsoleUIFactory::create_enemy(
    const Coord& top_left, const int width, const int height
) {
    ConsoleEnemy* enemy = new ConsoleEnemy(top_left, width, height);
    enemies.push_back(enemy);          // 1. владение (конкретный тип)
    game->add_map_movable(enemy);      // 2. роль: сдвигается с миром
    game->add_movable(enemy);          //    роль: движется сам
    game->add_collisionable(enemy);    //    роль: участвует в столкновениях
    game_map->add_obj(enemy);          // 3. отрисовка
}
```

### Три независимых списка, три разных причины

1. **Вектор фабрики** (`enemies`, `boxes`, `ships`, …) — чтобы знать
   конкретный тип для `delete` и для `clear()`.
2. **Вектора `Game`** — чтобы выполнять поведение через интерфейс.
3. **`ConsoleGameMap::objs`** — чтобы нарисовать. Тип здесь
   `ConsoleUIObject*`, то есть тоже интерфейс, а не `Rect`.

Пункт 3 — самый тонкий. `ConsoleGameMap` ничего не знает о физике: он
вызывает `get_left()`/`get_top()`/`get_brush()`. Всё это —
`ConsoleUIObjectRectAdapter`, адаптирующий `Rect` под
`ConsoleUIObject`. Именно поэтому `ConsoleEnemy` наследует **две**
иерархии: предметную (`Enemy`) и отображающую (`ConsoleUIObjectRectAdapter`).

### Пересоздание Марио — единственный не-idempotentный метод

```cpp
void ConsoleUIFactory::create_mario(...) {
    game->remove_collisionable(mario);
    game->remove_movable(mario);
    game->remove_mario();
    game_map->remove_obj(mario);
    delete mario;
    mario = nullptr;

    mario = new ConsoleMario(top_left, width, height);
    game->add_collisionable(mario);
    game->add_movable(mario);
    game->add_mario(mario);
    game_map->add_obj(mario);
}
```

Все остальные `create_*` просто добавляют объект. Этот — сначала
**убирает предыдущего Марио из всех четырёх мест и удаляет**, потому что
Марио в игре всегда один, а уровень может перезапускаться бесконечно
(смерть → `game_level->restart()`).

### Ленивая инициализация карты

```cpp
biv::GameMap* ConsoleUIFactory::get_game_map(int height, int width) {
    if (game_map == nullptr) {
        game_map = new ConsoleGameMap(height, width);
    }
    return game_map;
}
```

`main.cpp` вызывает это **до** создания уровня, поэтому к моменту
`create_*` указатель `game_map` уже валиден. Отсюда отсутствие
`nullptr`-проверок в `create_box` и остальных.

---

## 4. Кто кого удаляет: `remove_collisionable` и его пределы

`Game::remove_obj` использует идиому erase-remove:

```cpp
template<class T>
void Game::remove_obj(std::vector<T*>& container, T* obj) {
    container.erase(std::remove(container.begin(), container.end(), obj),
                   container.end());
}
```

`Game` **не удаляет объекты**. Он умеет только отвязать указатель.
Удаляет `ConsoleUIFactory::clear_data()`.

### Обнаруженный дефект: утечка памяти

```cpp
void ConsoleUIFactory::clear_data() {
    game->remove_objs();
    game_map->remove_objs();
    delete mario;            // ← только Марио
    mario = nullptr;
    boxes.clear();           // ← очистка вектора, но НЕ delete объектов
    full_boxes.clear();
    ships.clear();
    enemies.clear();
    moneys.clear();
}
```

`clear()` освобождает память **вектора**, но не память **объектов** в нём.
Все `ConsoleBox`, `ConsoleShip`, `ConsoleEnemy`, `ConsoleMoney`,
`ConsoleFullBox` остаются в куче. `clear_data()` вызывается из
`GameLevel::clear_data()`, то есть при **каждой** смене уровня и при
**каждой** перезагрузке уровня после смерти. `game_map` тоже никогда не
удаляется.

Это утечка, а не умысел: автор явно рассчитывал, что успеет удалить
объекты, и написал `delete` только для `mario` — самого важного.

### Обнаруженный дефект: «зомби»

```cpp
void Game::check_mario_collision() {
    for (int i = 0; i < collisionable_objs.size(); i++) {
        Collisionable* obj = collisionable_objs[i];
        if (obj->has_collision(mario)) {
            obj->process_mario_collision(mario);
            if (!mario->is_active()) {
                break;
            } else if (!obj->is_active()) {
                // TODO
                collisionable_objs[i] = collisionable_objs.back();
                collisionable_objs.pop_back();
                i--;
            }
        }
    }
}
```

Погибший объект убирается **только из `collisionable_objs`**. Он
остаётся в:

- `movable_objs` — продолжает двигаться;
- `map_movable_objs` — продолжает сдвигаться с миром;
- векторе фабрики — никогда не удалится;
- `ConsoleGameMap::objs` — **продолжает отрисовываться**.

Враг, на которого Марио прыгнул, формально «убит» (перестал участвовать
в столкновениях с Марио), но визуально остаётся на карте и падает вниз
бесконечно, потому что без `collisionable_objs` он больше не вызывает
`Enemy::process_vertical_static_collision` и не проверяет, не стоит ли на
краю. Строка `// TODO` в коде — автор сам отметил незавершённость.

`i--` после swap-with-back нужен, чтобы не пропустить элемент,
переехавший на позицию `i`. Это **O(1)** вместо `O(n)` сдвига.

---

## 5. Две ловушки при проектировании уровней

Это не относится к заданию напрямую, но всплывает при любой работе с
уровнями (задания 3, 4 и 7), поэтому фиксирую здесь.

### Ловушка 1: финиш уровня определяется позицией, а не смыслом

```cpp
void Game::check_vertically_static_collisions() noexcept {
    if (mario->has_collision(static_objs[static_objs.size() - 1])) {
        is_level_end_ = true;
    }
    ...
}
```

**Последний зарегистрированный статический объект — это финиш.** Не тот,
который помечен как цель, а буквально последний в векторе. В
`FirstLevel::init_data()` последним статическим объектом оказывается
`create_ship({210, 20}, 15, 7)`, потому что враги создаются после кораблей
и в `static_objs` не попадают.

Следствие: если в `init_data()` добавить платформу **после** финишного
корабля, финишем станет она, а корабль станет обычным препятствием.

### Ловушка 2: побеждает первое совпадение

```cpp
for (Collisionable* obj: collisionable_objs) {
    for (Rect* static_obj: static_objs) {
        if (obj->has_collision(static_obj)) {
            obj->process_vertical_static_collision(static_obj);
            break;              // ← остальные не рассматриваются
        }
    }
}
```

Если два статических объекта пересекаются, поведение зависит от порядка
создания. Для движущейся платформы это особенно важно: она не должна
пересекаться с кораблём в момент касания, иначе реакция будет
непредсказуемой.

---

## 6. Кто решает, что делать при столкновении

Столкновения в игре **не разрешает `Game`**. `Game` лишь находит пары и
передаёт управление объекту-участнику:

```cpp
obj->process_vertical_static_collision(static_obj);
```

Каждый объект сам решает свою реакцию. Так работает и
`Enemy::process_mario_collision`:

```cpp
void Enemy::process_mario_collision(Collisionable* mario) noexcept {
    if (mario->get_speed().v > 0 && mario->get_speed().v != V_ACCELERATION) {
        kill();          // Марио падает сверху — враг погибает
    } else {
        mario->kill();   // иначе погибает Марио
    }
}
```

Обратите внимание на аргумент: `Collisionable*`, но используется как
`Mario*` (вызывается `get_speed()` и `kill()`). Формально здесь
**небезопасное понижение типа** — работает только потому, что в этом
контейнере известно, кто есть кто. Архитектурно это самое слабое место
из всех рассмотренных: контракт `Collisionable*` не выражает, что
передают именно Марио.

`Game` при этом остаётся полностью «глупым»: он не знает ни про врагов,
ни про деньги, ни про прыжки. Он знает только четыре интерфейса. Это и
есть ответ на вопрос из `game.hpp`:

> `- Как объекты раскладываются по контейнерам?`

По контейнерам, соответствующим **их роли в игровом цикле**, а не по их
классу. Вектор `static_objs` хранит `Rect*` — и туда попадают и `Ship`,
и `Box`, и `FullBox`; вектор `collisionable_objs` хранит
`Collisionable*` — и туда попадают и `Enemy`, и `Money`, и `FullBox`.
Один объект может быть в нескольких контейнерах, потому что играет
несколько ролей.

---

## 7. Критика

Что сделано хорошо:

- разделение модели и представления выполнено последовательно
  (`Enemy` ↔ `ConsoleEnemy`, `GameMap` ↔ `ConsoleGameMap`);
- `Game` не знает ни одного конкретного типа — это и есть «логика работает
  с множествами абстракций»;
- абстрактная фабрика `UIFactory` позволяет подменить консоль на любую
  другую выводку, не трогая `GameLevel`;
- роли объектов выражены типами контейнеров, а не флагами.

Что можно улучшить:

1. **Двойное хранение.** Один объект живёт в 3–4 контейверах. Добавление
   новой роли требует правок в `Game`, `ConsoleUIFactory` и
   `ConsoleGameMap` одновременно. Естественный рефакторинг — хранить один
   список объектов с набором битовых ролей, либо интерфейс с
   `get_roles()`.
2. **Владение не выражено в типах.** Всё на `new` + `delete` вручную,
   без `std::unique_ptr`. Отсюда утечка в `clear_data()`.
3. **Удаление только из одного контейнера** (см. раздел 4). Нужен
   обратный вызов умирающего объекта, чтобы убрать его отовсюду.
4. **`Collisionable*` в `process_mario_collision`** — слишком общий тип.
   Достаточно было `Mario*` или отдельного интерфейса персонажа.
5. **Финиш уровня через `static_objs.back()`** — неявный контракт,
   полностью невидимый из `GameLevel`. Уместнее отдельный объект-цель.
