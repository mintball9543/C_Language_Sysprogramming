@echo off
set globalvar=5
SETLOCAL
set var=13145
set /A var=%var% + 5
ENDLOCAL
echo %var%
echo %globalvar%