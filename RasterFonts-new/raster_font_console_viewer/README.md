

Цей код виводить заданий текст у консоль у вигляді растрових символів.

# Зібрати
make

# Список шрифтів
./build/app/application.elf --list-fonts

# Інфо про шрифт
./build/app/application.elf --font Terminus12x6 --info --color

# Один гліф (літера "А" = U+0410)
./build/app/application.elf --font Terminus12x6 --char 0x0410 --color

# Текст
./build/app/application.elf --font Terminus12x6 --text "Hello!" --scale 2 --color

# Кирилиця
./build/app/application.elf --font Terminus12x6 --text "Привіт" --color

# Всі гліфи
./build/app/application.elf --font FreePixel --all --cols 10 --color

# Без рамки
./build/app/application.elf --font Pixel --char 0x23 --no-frame --color
