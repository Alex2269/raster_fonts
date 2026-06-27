#!/bin/bash
# svg2font.sh - швидка конвертація SVG у RasterFont

if [ $# -lt 2 ]; then
    echo "Usage: $0 <output_name> <svg_file_or_dir> [width] [height]"
    echo "Example: $0 myicons ./icons 24 24"
    exit 1
fi

OUTPUT_NAME="$1"
INPUT_PATH="$2"
WIDTH="${3:-32}"
HEIGHT="${4:-32}"

echo "Converting SVG to RasterFont..."
echo "Output: ${OUTPUT_NAME}.c / ${OUTPUT_NAME}.h"
echo "Glyph size: ${WIDTH}x${HEIGHT}"
echo ""

# Запускаємо конвертер
./build/app/application.elf -w $WIDTH -h $HEIGHT -v "$OUTPUT_NAME" "$INPUT_PATH"

if [ $? -eq 0 ]; then
    echo ""
    echo "✓ Generated files:"
    ls -lh ${OUTPUT_NAME}.c ${OUTPUT_NAME}.h 2>/dev/null
else
    echo ""
    echo "✗ Conversion failed!"
    exit 1
fi
