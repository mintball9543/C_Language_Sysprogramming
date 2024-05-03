@echo off
set /a index=1
set /a count=5
:while
if %index% leq %count% (echo The value of index is %index%
set /a index=index+1
goto :while)