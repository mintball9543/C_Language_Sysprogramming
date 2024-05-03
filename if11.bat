@echo off
set /a a=5
set /a b=10
if %a%==6 goto :labela
if %b%==10 goto :labelb
:labela
echo The value of a is 5
:labelb
echo The value of a is 10