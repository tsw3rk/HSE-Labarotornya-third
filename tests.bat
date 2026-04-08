@echo off
chcp 65001 >nul
cls
echo ==================================================
echo   Lab 3: Калькулятор выражений
echo ==================================================
echo.
echo Вводи тесты построчно. Пример:
echo   evaluate 2 x y
echo   3 4
echo   x*x+y*2
echo.
echo Чтобы завершить ввод: Ctrl+Z, затем Enter
echo ==================================================
echo.

set "EXE=cmake-build-debug\lab3.exe"
if not exist "%EXE%" set "EXE=lab3.exe"

if not exist "%EXE%" (
    echo ERROR: lab3.exe not found! Build project first.
    pause
    exit /b 1
)

echo [Запуск программы...]
echo --------------------------------------------------
"%EXE%"

echo.
echo --------------------------------------------------
echo [Программа завершилась]
echo.
echo Нажми любую клавишу для выхода...
pause >nul