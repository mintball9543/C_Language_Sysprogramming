@echo off
set str1=string1
set str2=string2
if defined str1 echo variable str1 is defined
if defined str3 (echo variable str3 is defined) else (echo variable str3 is not defined)
