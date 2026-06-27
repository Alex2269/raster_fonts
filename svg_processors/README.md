
```markdown
# 🖼️ Raster Font Tools

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![C Standard](https://img.shields.io/badge/C-GNU17-blue.svg)](https://www.gnu.org/software/gnu-c-manual/)
[![Platform](https://img.shields.io/badge/Platform-Linux%20x86__64-green.svg)]()
[![Embedded](https://img.shields.io/badge/Target-Embedded%20Systems-orange.svg)]()

> 🇺🇦 Набір інструментів для генерації та перегляду растрових шрифтів (RasterFont) з різних джерел: **SVG**, **TTF**, **PSF**. Оптимізовано для вбудованих систем.

---

## 📋 Зміст

- [Огляд](#-огляд)
- [Особливості](#-особливості)
- [Архітектура](#-архітектура)
- [Вимоги](#-вимоги)
- [Встановлення](#-встановлення)
- [Використання](#-використання)
  - [SVG → RasterFont](#1-svg--rasterfont)
  - [TTF → RasterFont](#2-ttf--rasterfont)
  - [PSF → RasterFont](#3-psf--rasterfont)
  - [Консольний переглядач](#4-консольний-переглядач)
- [Формат даних](#-формат-даних)
- [Діагностика](#-діагностика)
- [Ліцензія](#-ліцензія)

---

## 🔍 Огляд

Проект складається з **трьох генераторів** та **двох консольних переглядачів**, які працюють з уніфікованою структурою `RasterFont`. Це дозволяє легко інтегрувати шрифти у вбудовані системи з мінімальним споживанням пам'яті.

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│  SVG Icons  │     │  TTF Fonts  │     │  PSF Fonts  │
└──────┬──────┘     └──────┬──────┘     └──────┬──────┘
       │                   │                   │
       ▼                   ▼                   ▼
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│svg_to_raster│     │ttf_generator│     │psf_generator│
└──────┬──────┘     └──────┬──────┘     └──────┬──────┘
       │                   │                   │
       └───────────┬───────┴───────────┬───────┘
                   ▼                   ▼
          ┌────────────────────────────────┐
          │   Unified RasterFont Format    │
          │   (C arrays + metadata)        │
          └──────────────┬─────────────────┘
                         ▼
              ┌─────────────────────┐
              │  Console Viewers    │
              │  (verification)     │
              └─────────────────────┘
```

---

## ⭐ Особливості

- 🎯 **Уніфікований формат** — усі генератори створюють однакову структуру `RasterFont`
- 📦 **Компактне зберігання** — 1 біт на піксель, щільне пакування
- 🔍 **Автоматична обрізка** — видалення порожніх полів для економії пам'яті
- 🎨 **Підтримка SVG** — конвертація іконок (FontAwesome тощо) у растровий формат
- 🔤 **Підтримка TTF** — генерація з TrueType шрифтів з обробкою кирилиці
- 🖥️ **Підтримка PSF** — імпорт консольних шрифтів Linux
- 👁️ **Консольний перегляд** — візуальна перевірка без GUI
- ⚡ **Embedded-ready** — мінімальні залежності, оптимізовано для MCU

---

## 🏗️ Архітектура

```
raster_fonts/
├── svg_processors/
│   ├── svg_to_raster/          # SVG → RasterFont
│   │   ├── svg_generator/
│   │   │   └── svg_to_raster.c
│   │   ├── Makefile
│   │   └── svg2font.sh
│   │
│   └── raster_console_viewer/  # Переглядач RasterFont
│       ├── raster_viewer/
│       │   ├── raster_viewer.c
│       │   ├── glyphmap.h
│       │   └── glyphs.h
│       └── Makefile
│
├── ttf_to_raster_generator/    # TTF → RasterFont
│   ├── ttf_generator/
│   │   └── ttf_generator.c
│   └── Makefile
│
├── psf_to_raster_generator/    # PSF → RasterFont
│   ├── psf_generator/
│   │   ├── psf_generator.c
│   │   ├── psf_font.c/h
│   │   └── UnicodeGlyphMap.h
│   └── Makefile
│
├── ttf-console-viewer/         # Переглядач TTF у консолі
│   └── ttf-console-viewer.c
│
└── psf_console_viewer/         # Переглядач PSF у консолі
    └── psf_console_viewer.c
```

---

## 📦 Вимоги

### Системні залежності (Ubuntu/Debian)

```bash
# Базові інструменти
sudo apt install build-essential gcc make

# Для SVG генератора
sudo apt install librsvg2-dev libcairo2-dev libglib2.0-dev

# Для TTF генератора та переглядачів
sudo apt install libfreetype-dev

# Опціонально: для GUI-переглядачів
sudo apt install libraylib-dev libglfw3-dev libgl1-mesa-dev
```

---

## 🚀 Встановлення

### 1. Клонувати репозиторій

```bash
git clone https://github.com/Alex2269/raster_fonts.git

```

### 2. Зібрати всі інструменти

```bash
# SVG генератор
cd svg_processors/svg_to_raster && make clean && make

# TTF генератор
cd ../../ttf_to_raster_generator && make clean && make

# PSF генератор
cd ../psf_to_raster_generator && make clean && make

# Консольний переглядач
cd ../svg_processors/raster_console_viewer && make clean && make
```

---

## 🛠️ Використання

### 1. SVG → RasterFont

Конвертація SVG іконок (наприклад, FontAwesome) у формат RasterFont.

#### Синтаксис

```bash
./build/app/application.elf [опції] <ім'я_шрифту> <шлях_до_svg>
```

#### Опції

| Опція | Опис | За замовчуванням |
|-------|------|------------------|
| `-w <width>` | Ширина гліфа в пікселях | 16 |
| `-h <height>` | Висота гліфа в пікселях | 16 |
| `-v` | Детальний вивід (verbose) | вимкнено |

#### Приклади

```bash
# Одиночний SVG файл
./build/app/application.elf myfont icon.svg

# Пакетна обробка директорії
./build/app/application.elf -w 16 -h 16 myicons ./svg_icons

# З детальним виводом
./build/app/application.elf -w 24 -h 24 -v awesome ./fontawesome-svg
```

#### Швидка конвертація через скрипт

```bash
chmod +x svg2font.sh
./svg2font.sh myicons ./svg_icons 16 16
```

#### Формати імен файлів

| Ім'я файлу | Unicode код |
|------------|-------------|
| `a.svg` | U+0061 |
| `5.svg` | U+0035 |
| `0041.svg` | U+0041 |
| `U+2665.svg` | U+2665 (♥) |

#### Сортування

Файли обробляються у порядку:
1. **Цифри**: `0.svg` → `9.svg`
2. **Малі літери**: `a.svg` → `z.svg`
3. **Великі літери**: `A.svg` → `Z.svg`
4. **Інші імена** — за алфавітом

---

### 2. TTF → RasterFont

Генерація растрових гліфів з TrueType шрифтів.

```bash
./build/app/application.elf <шрифт.ttf> <ширина> <висота>

# Приклад
./build/app/application.elf fonts/DejaVuSans.ttf 12 16
```

**Особливості:**
- Автоматична генерація ASCII (32-126) та кирилиці (0x0400-0x04FF)
- Обрізка порожніх рядків
- Обчислення вертикальних зсувів для правильного рендеру

---

### 3. PSF → RasterFont

Імпорт консольних шрифтів Linux (формат PSF1/PSF2).

```bash
./build/app/application.elf <шрифт.psf>

# Приклад
./build/app/application.elf fonts/TerminusBold32x16.psf
```

**Підтримувані шрифти Terminus:**
- `Terminus12x6.psf` ... `Terminus32x16.psf`
- `TerminusBold18x10.psf` ... `TerminusBold32x16.psf`

---

### 4. Консольний переглядач

Візуальна перевірка згенерованих шрифтів прямо у терміналі.

#### Синтаксис

```bash
./application.elf [опції]
```

#### Опції

| Опція | Опис | За замовчуванням |
|-------|------|------------------|
| `-a [scale]` | Показати всі гліфи | scale=2 |
| `-u <code> [scale]` | Один гліф за Unicode (hex) | scale=2 |
| `-g [per_row] [scale]` | Сітка іконок | per_row=5, scale=2 |
| `-h` | Довідка | — |

#### Приклади

```bash
# Всі гліфи з масштабом 2x
./application.elf -a 2

# Конкретний гліф U+0021 з масштабом 3x
./application.elf -u 0021 3

# Сітка 5 іконок в ряд, масштаб 2x
./application.elf -g 5 2
```

#### Приклад виводу

```
=== Glyph U+0021 (index 1) ===
Unicode: U+0021
Size: 13x16 pixels
Buffer size: 32 bytes
Offsets: vertical=0, horizontal=0
---
          ###
          ###
##        ###
####      ###
#####     ###
#######   ###
#############
#############
#############
#############
#############
#######   ###
#####     ###
####      ###
##        ###
          ###
```

---

## 📐 Формат даних

### Структура `RasterFont`

```c
typedef struct {
    const char* name;                    // Назва шрифту
    int glyph_width;                     // Базова ширина
    int glyph_height;                    // Базова висота
    int glyph_bytes;                     // Розмір буфера на гліф
    const GlyphPointerMap* glyph_map;    // Мапа Unicode → гліф
    int glyph_count;                     // Кількість гліфів
    const int* glyph_widths;             // Фактичні ширини
    const int* glyph_heights;            // Фактичні висоти
    const int* glyph_vertical_offsets;   // Вертикальні зсуви
    const int* glyph_horizontal_offsets; // Горизонтальні зсуви
} RasterFont;
```

### Бітове пакування

Кожен гліф зберігається як масив байтів:
- **1 біт = 1 піксель**
- **Біти читаються зліва направо** (MSB first)
- **Рядки йдуть зверху вниз**

```
Приклад: байт 0b11000011 → пікселі: ##    ##
```

### Формула розміру

```c
bytes_per_row = (width + 7) / 8;
total_bytes   = bytes_per_row * height;
```

---

## 🔬 Діагностика

### Проблема: Гліфи виглядають дзеркальними

**Симптом:** Іконки відображаються перевернутими.

**Рішення:** Перевірте порядок бітів у генераторі:
```c
int bit_idx = 7 - (x % 8);  // MSB first (зліва направо)
```

### Проблема: Гліфи обрізані або зміщені

**Симптом:** Частина іконки відсутня.

**Рішення:**
1. Перевірте `glyph_width` та `glyph_height`
2. Перевірте `vertical_offset` та `horizontal_offset`
3. Переконайтеся, що генератор правильно обрізає порожні поля

### Проблема: Порожні гліфи

**Симптом:** Гліф відображається повністю порожнім.

**Рішення:**
1. Перевірте, чи достатньо `glyph_bytes`
2. Перевірте формулу: `bytes_per_row = (width + 7) / 8`
3. Використовуйте переглядач з різними масштабами: `-u <code> 1` та `-u <code> 3`

### Проблема: SVG не рендериться правильно

**Симптом:** Деформації або відсутні деталі.

**Рішення:**
1. Перевірте `viewBox` у SVG — деякі мають негативні координати
2. Використовуйте параметр `-v` для детального виводу bounding box
3. Спробуйте різні розміри: `-w 24 -h 24`

---

## 📊 Порівняння генераторів

| Функція | SVG | TTF | PSF |
|---------|-----|-----|-----|
| Вхідний формат | SVG файли | TTF шрифти | PSF1/PSF2 |
| Підтримка кирилиці | ✅ (через імена) | ✅ (авто) | ✅ (через мапу) |
| Автоматична обрізка | ✅ | ✅ | ⚠️ (часткова) |
| Вертикальні зсуви | ❌ | ✅ | ❌ |
| Залежності | Cairo, RSVG | FreeType | — |

---

## 🤝 Внесок

Вітаються pull requests та issues! Для великих змін спочатку відкрийте issue для обговорення.

---

## 📄 Ліцензія

Цей проект ліцензовано під **GNU General Public License v3.0** — див. файл [LICENSE](LICENSE) для деталей.

```
Copyright (C) 2026

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
```

---

## 🙏 Подяки

- [FreeType](https://freetype.org/) — бібліотека для роботи зі шрифтами
- [Cairo](https://www.cairographics.org/) — 2D графічна бібліотека
- [librsvg](https://wiki.gnome.org/Projects/LibRsvg) — SVG рендерер
- [FontAwesome](https://fontawesome.com/) — джерело SVG іконок

---

<p align="center">
  Зроблено з ❤️ для embedded-спільноти
</p>
```

---

## 📌 Додаткові файли, які варто додати до репозиторію

### `.gitignore`

```gitignore
# Build artifacts
build/
*.elf
*.hex
*.bin
*.o
*.d
*.lst

# Generated fonts
*.c
*.h
!glyphmap.h
!glyphs.h
!UnicodeGlyphMap.h
!psf_font.h

# Editor files
.vscode/
.idea/
*.swp
*~

# OS files
.DS_Store
Thumbs.db

# Temporary
__tmp/
dest_dir/
```

### `CONTRIBUTING.md`

```markdown
# Внесок у проект

## Як допомогти

1. Форкніть репозиторій
2. Створіть гілку для своїх змін: `git checkout -b feature/amazing-feature`
3. Зробіть коміт: `git commit -m 'Add amazing feature'`
4. Запушіть: `git push origin feature/amazing-feature`
5. Відкрийте Pull Request

## Стиль коду

- Використовуйте **C17 / GNU17** стандарт
- Коментарі — **українською** або англійською
- Іменування змінних — `snake_case`
- Відступи — 4 пробіли

## Тестування

Перед PR обов'язково:
- [ ] Збірка без помилок (`make clean && make`)
- [ ] Перевірка через консольний переглядач
- [ ] Порівняння з оригінальними SVG/TTF
```

---

Цей README готовий до публікації на GitHub! 🚀 Він містить усе необхідне: опис, приклади, діаграми, таблиці та інструкції з діагностики. Якщо хочеш додати щось конкретне (наприклад, скріншоти виводу, відео-демо або більше прикладів) — скажи, доповнимо!
