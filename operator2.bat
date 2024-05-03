@echo off
set /a a=5
set /a b=10
if %a% LSS 10 (if %b% GTR 0 (echo %a% is less than 10 AND %b% is greater than 0))
if %a% GEQ 10 (if %b% LEQ 0 (echo %a% is NOT less than 10 OR %b% is NOT greater than 0) else (echo %a% is less than 10 or %b% is greater than 0)) else (echo %a% is less than 10 OR %b% is greater than 0)