# psf_to_raster_generator

```markdown
# Генератор PSF Шрифтів у C-файли

Цей проєкт містить генератор шрифтів у форматі PSF, який зчитує шрифти PSF і генерує відповідні C-файли (.c та .h) із масивами гліфів, таблицею відповідності Unicode → гліф і структурою `Font` для зручного використання у C-проєктах.

---

## Особливості

- Підтримка ASCII (32–126) і кирилиці (Unicode).
- Генерація масиву структур `GlyphPointerMap` з Unicode і вказівником на гліф.
- Автоматичне створення структури `Font` із параметрами шрифту і масивом гліфів.
- Можливість використання декількох шрифтів у одному проєкті.
- Інтеграція з основною бібліотекою для малювання тексту з використанням структур Font.

---

## Використання

### Компіляція генератора

Складіть генератор командою:

```
make
```

або за допомогою вашого Makefile.

---

### Створення файлів шрифтів

Запустіть генератор, передавши шлях до PSF-файлу шрифту:

```

```sh

build/app/application.elf  fonts/Terminus12x6.psf
build/app/application.elf  fonts/Terminus18x10.psf
build/app/application.elf  fonts/Terminus20x10.psf
build/app/application.elf  fonts/Terminus22x11.psf
build/app/application.elf  fonts/Terminus24x12.psf
build/app/application.elf  fonts/Terminus28x14.psf
build/app/application.elf  fonts/Terminus32x16.psf
build/app/application.elf  fonts/TerminusBold18x10.psf
build/app/application.elf  fonts/TerminusBold20x10.psf
build/app/application.elf  fonts/TerminusBold22x11.psf
build/app/application.elf  fonts/TerminusBold24x12.psf
build/app/application.elf  fonts/TerminusBold28x14.psf
build/app/application.elf  fonts/TerminusBold32x16.psf

# build/app/application.elf  You_other_font.psf 

```

```

В результаті з'являться файли:

- `Terminus12x6.c`
- `Terminus12x6.h`

- `Terminus18x10.c`
- `Terminus18x10.h`
- `Terminus20x10.c`
- `Terminus20x10.h`
- `Terminus22x11.c`
- `Terminus22x11.h`
- `Terminus24x12.c`
- `Terminus24x12.h`
- `Terminus28x14.c`
- `Terminus28x14.h`
- `Terminus32x16.c`
- `Terminus32x16.h`

- `TerminusBold18x10.c`
- `TerminusBold18x10.h`
- `TerminusBold20x10.c`
- `TerminusBold20x10.h`
- `TerminusBold22x11.c`
- `TerminusBold22x11.h`
- `TerminusBold24x12.c`
- `TerminusBold24x12.h`
- `TerminusBold28x14.c`
- `TerminusBold28x14.h`
- `TerminusBold32x16.c`
- `TerminusBold32x16.h`

---

### Інтеграція у проект

1. Скопіюйте `.c` і `.h` файли у вашу папку проекту, наприклад, `fonts/`.
2. Додайте інклуд у ваші вихідні файли:

```
#include "TerminusBold18x10.h"
```

3. Використовуйте структуру шрифту для відображення тексту:

```
Font_DrawTextScaled(&TerminusBold18x10_font, "Привіт світе!", 10, 20, 2, 1, WHITE);
```

(передбачається, що у вас є реалізація функції `DrawPixel` і функції для малювання тексту, які працюють зі структурою `Font`)

---

## Структура файлів

- `*.c` — байти гліфів, масиви вказівників GlyphPointerMap, ініціалізація структури Font.
- `*.h` — константи розмірів шрифту, оголошення масиву GlyphPointerMap і структури Font.
- `font.h` — опис структури `Font`.
- `glyphmap.h` — опис структури GlyphPointerMap.

---

## Налаштування і розширення

- Додайте підтримку нових кодових сторінок або символів, змінюючи файл `UnicodeGlyphMap.h`.
- Можна модифікувати генератор для підтримки пропорційних шрифтів або інших форматів.

---

## Ліцензія GPL

---

## Автор

https://github.com/Alex2269/psf_font-scale

---


```
