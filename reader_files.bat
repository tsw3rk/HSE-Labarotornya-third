@echo off
chcp 65001 >nul
echo 📂 Читаю тесты из input.txt...
echo --------------------------------------------------

set "EXE=cmake-build-debug\lab3.exe"
if not exist "%EXE%" set "EXE=lab3.exe"

if not exist "input.txt" (
    echo ❌ Не найден input.txt! Создай его в папке проекта.
    pause
    exit /b 1
)

"%EXE%" < input.txt

echo --------------------------------------------------
echo ✅ Тесты завершены.
pause