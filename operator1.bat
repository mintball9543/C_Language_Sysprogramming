@echo off
set /a a=5
set /a b=10
if %a% EQU %b% echo A is equal to than B
if %a% NEQ %b% echo A is not equal to than B
if %a% LSS %b% echo A is less than B
if %a% LEQ %b% echo A is less than or equal B
if %a% GTR %b% echo A is greater than B
if %a% GEQ %b% echo A is greater than B
