@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall" x64
set path=%path%;W:/RoqueLike/misc/
start /b "" "script.exe"