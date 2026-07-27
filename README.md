<div align="center">

# 🥊 Punchy2D

### Cartoon 2D Physics Engine / Мультяшный 2D физический движок

[![C++17](https://img.shields.io/badge/C++-17-blue?logo=c%2B%2B)](https://en.cppreference.com/w/cpp/17)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Size](https://img.shields.io/badge/code-~600_lines-orange)]()

**Fun-first physics for punches, slaps and spectacular ragdolls.**
**Физика ради веселья: удары, пощёчины и зрелищные рэгдоллы.**

[Quick Start / Быстрый старт](#-quick-start--быстрый-старт) • [API](#-api-reference--справочник-api) • [Build / Сборка](#️-build--сборка)

</div>

---

## ✨ Features / Особенности

| 🇬🇧 English | 🇷🇺 Русский |
| :--- | :--- |
| 🎯 **Cartoon over realism** — exaggerated reactions, spectacular flights and bounces | 🎯 **Мультяшность важнее реализма** — преувеличенные реакции, зрелищные полёты и отскоки |
| 🤖 **Soft ragdoll** — 10 body parts, soft joints, force propagation | 🤖 **Мягкий рэгдолл** — 10 частей тела, эластичные суставы, передача силы |
| ⚡ **Juicy impulses** — off-center hits create spin, chain reactions through body | ⚡ **Сочные импульсы** — удары не в центр крутят, цепные реакции через тело |
| 🎲 **Built-in chaos** — random micro-impulses for unpredictable fun | 🎲 **Встроенный хаос** — случайные микроимпульсы для непредсказуемости |
| 🕐 **Fixed timestep** — configurable TPS (default 35), spiral-of-death protection | 🕐 **Фиксированный шаг** — настраиваемый TPS (по умолчанию 35), защита от спирали смерти |
| 🧩 **Tiny & dependency-free** — 5 files, ~600 lines, pure C++17 | 🧩 **Крошечный и без зависимостей** — 5 файлов, ~600 строк, чистый C++17 |

---

## 🚀 Quick Start / Быстрый старт

```cpp
#include "PhysicsEngine.h"
#include "Ragdoll.h"

using namespace Punchy2D;

int main() {
    PhysicsEngine engine;

    // Tune the cartoon feel / Настройка мультяшности
    engine.config().impulseMultiplier = 3.5f;  // Punch strength / Сила удара
    engine.config().spinMultiplier    = 6.0f;  // Spin exaggeration / Преувеличение вращения
    engine.config().gravity           = {0.f, 250.f};

    // Create ragdoll / Создаём рэгдолла
    Ragdoll rag = createRagdoll(engine, {400.f, 300.f});

    // Create heavy ball / Создаём тяжёлый мяч
    BodyId ball = engine.createObject();
    Body* b = engine.getBody(ball);
    b->position = {-100.f, 200.f};
    b->setMass(15.f);

    // PUNCH! / УДАР!
    engine.applyImpulse(ball, {10000.f, 300.f}, b->position);

    // Simulate 1 second / Симуляция 1 секунду
    for (int i = 0; i < 60; ++i)
        engine.update(1.f / 60.f);

    return 0;
}
```

---

## 🎛 Configuration / Конфигурация

All tuning lives in `PhysicsConfig` / Все настройки в `PhysicsConfig`:

| Parameter / Параметр | Default / По умолч. | Description / Описание |
| :--- | :--- | :--- |
| `gravity` | `{0, 300}` | Cartoon gravity / Мультяшная гравитация |
| `ticksPerSecond` | `35` | Physics tick rate / Частота физических тиков |
| `impulseMultiplier` | `3.0` | Punch strength multiplier / Множитель силы удара |
| `spinMultiplier` | `5.0` | Rotation exaggeration / Преувеличение вращения |
| `chainReactionScale` | `0.8` | Force propagation through joints / Передача силы через суставы |
| `chaosMultiplier` | `0.3` | Random impulse intensity / Интенсивность случайных импульсов |
| `maxVelocity` | `8000` | Speed cap / Ограничение скорости |
| `constraintIterations` | `5` | Solver accuracy / Точность решателя связей |
| `substeps` | `2` | Substeps per tick / Подшаги на тик |

### TPS Guide / Рекомендации по TPS

| TPS | Best for / Лучше для | Feel / Ощущение |
| :--- | :--- | :--- |
| 20 | Mobile, background physics / Мобильные, фоновая физика | Choppy but fast / Дёрганая, но быстрая |
| **35** | **Default balance / Баланс по умолчанию** | **Good enough / Достаточно хорошо** |
| 60 | Desktop, smooth gameplay / Десктоп, плавный геймплей | Smooth and precise / Плавная и точная |
| 120 | High-precision simulation / Высокоточная симуляция | Maximum accuracy / Максимальная точность |

```cpp
engine.setTicksPerSecond(60.f);   // Change at runtime / Изменение на лету
engine.update(deltaTime);         // Accumulator handles everything / Аккумулятор делает всё сам
```

---

## 📖 API Reference / Справочник API

### Bodies / Тела

| Method / Метод | Description / Описание |
| :--- | :--- |
| `createObject()` | Create dynamic body / Создать динамическое тело |
| `createStaticBody(pos, r)` | Create static body (floor/wall) / Создать статику (пол/стена) |
| `removeObject(id)` | Remove body + its constraints / Удалить тело и его связи |
| `getBody(id)` | Get body pointer by ID / Получить указатель на тело по ID |

### Forces / Силы

| Method / Метод | Description / Описание |
| :--- | :--- |
| `applyForce(id, force)` | Continuous force / Непрерывная сила |
| `applyImpulse(id, impulse, point)` | Instant hit at world point / Мгновенный удар в точку |
| `applyAngularImpulse(id, impulse)` | Pure spin impulse / Импульс вращения |

### Simulation / Симуляция

| Method / Метод | Description / Описание |
| :--- | :--- |
| `update(dt)` | Advance with fixed timestep / Шаг с фиксированным dt |
| `reset()` | Reset accumulator & tick counter / Сброс аккумулятора и счётчика |
| `setTicksPerSecond(tps)` | Change TPS (rescales accumulator) / Сменить TPS |
| `getTotalTicks()` | Total ticks since start / Всего тиков с начала |

### Ragdoll / Рэгдолл

```cpp
// Create / Создать
Ragdoll rag = createRagdoll(engine, origin, scale);

// Hit specific part / Ударить конкретную часть
hitRagdollPart(engine, rag, HEAD, impulse, point);
```

**Parts / Части:** `HEAD` · `TORSO` · `UPPER_ARM_L/R` · `LOWER_ARM_L/R` · `UPPER_LEG_L/R` · `LOWER_LEG_L/R`

---

## 🛠️ Build / Сборка

**Requirements / Требования:** C++17 compiler / компилятор C++17. No dependencies / Без зависимостей.

### One-liner / Одной командой

```bash
# Linux / macOS / MinGW
g++ -std=c++17 -O2 -Wall Example.cpp PhysicsEngine.cpp Ragdoll.cpp -o punchy2d && ./punchy2d

# Windows MSVC
cl /std:c++17 /O2 /W4 Example.cpp PhysicsEngine.cpp Ragdoll.cpp && punchy2d.exe
```

### CMake (optional / опционально)

```cmake
cmake_minimum_required(VERSION 3.10)
project(Punchy2D CXX)
set(CMAKE_CXX_STANDARD 17)

add_library(punchy2d STATIC PhysicsEngine.cpp Ragdoll.cpp)
target_include_directories(punchy2d PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(example Example.cpp)
target_link_libraries(example punchy2d)
```

### Integration / Интеграция

Copy 5 files into your project / Скопируйте 5 файлов в проект:

```text
your_project/src/physics/
├── PhysicsEngine.h
├── PhysicsEngine.cpp
├── Ragdoll.h
├── Ragdoll.cpp
└── Example.cpp  ← optional / опционально
```

---

## 🎮 Use Cases / Применение

| 🇬🇧 | 🇷🇺 |
| :--- | :--- |
| 🥊 Kick the Buddy–style games | 🥊 Игры в стиле Kick the Buddy |
| 🎯 Punch / slap simulators | 🎯 Симуляторы ударов и пощёчин |
| 💥 Chain-reaction destruction | 💥 Разрушения с цепными реакциями |
| 🎨 Cartoon physics animations | 🎨 Мультяшные физические анимации |
| 📱 Mobile games (low TPS mode) | 📱 Мобильные игры (режим низкого TPS) |

---

## 🤝 Contributing / Вклад

Issues and PRs are welcome! / Issues и PR приветствуются!

**Wishlist / Список идей:** Raycasting · Verlet cloth · Impact particles · Spatial partitioning · More ragdoll presets · Polygon shapes · Breakable joints API

---

## 📄 License / Лицензия

**MIT** — use freely in personal and commercial projects. Attribution appreciated but not required.
**MIT** — свободное использование в личных и коммерческих проектах. Указание авторства приветствуется, но не обязательно.

See [LICENSE](LICENSE) for full text / Полный текст в файле [LICENSE](LICENSE).

---

<div align="center">

**Made with ❤️ for devs who love cartoon physics / Сделано с ❤️ для тех, кто любит мультяшную физику**

⭐ Star if useful! / Поставьте звезду, если полезно! ⭐

</div>
