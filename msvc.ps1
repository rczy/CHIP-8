param (
    [Alias('i')]
    [string]$INC_PATH = "lib\SDL2\include",
    [Alias('l')]
    [string]$LIB_PATH = "lib\SDL2\lib\x64"
)

$EXECUTABLE = "chip-8"
$SOURCE_FILES = @("chip8.c", "input.c", "display.c", "beeper.c", "args.c", "device.c", "main.c")
$SOURCE_FILES_PATH = $SOURCE_FILES -replace "^", "src\"
$OUTPUT_DIR = "bin\windows"

if (!(Test-Path $OUTPUT_DIR)) {
    New-Item -ItemType Directory -Force -Path $OUTPUT_DIR
}
if (!(Test-Path $OUTPUT_DIR\SDL2.dll)) {
    Copy-Item lib\SDL2.dll -Destination $OUTPUT_DIR
}

cl $SOURCE_FILES_PATH /Fe"$OUTPUT_DIR\$EXECUTABLE" /Fo"$OUTPUT_DIR"\ /I $INC_PATH /link /LIBPATH:$LIB_PATH SDL2.lib SDL2main.lib shell32.lib /SUBSYSTEM:WINDOWS /DEBUG:FULL
