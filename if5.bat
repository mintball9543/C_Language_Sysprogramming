@echo off
set /a a=5
set /a b=10
set /a c=%a% + %b%
if %c% == 15 (echo The value of variable c is 15) else (echo unknown value)
if %c% == 10 (echo The value of variable c is 10) else (echo unknown value)