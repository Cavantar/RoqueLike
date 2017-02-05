@echo off

if not exist ..\build mkdir ..\build
pushd ..\build

set UnityBuild=False

set LibsDirectory=E:/Libs

set IncludeDirectories= ^
	-I%LibsDirectory%/glm

set LibraryDirectories=
REM -LIBPATH:%LibsDirectory%/freeglut/lib/x64/ ^

set Libs=

set FilesToCompile= ^
../jpb/Noise.cpp ^
../jpb/SimpleParser.cpp ^
../jpb/Profiler.cpp

set FilesToLink= ^
Noise.obj ^
SimpleParser.obj ^
Profiler.obj

REM Zi(Generate Debug information), FC(Full Path To Source), O2(Fast Code)

set CompilerOptions=%Defines% /FC /EHsc /MD /MP /wd4503 /nologo %IncludeDirectories%
set LinkerOptions=/link %LibraryDirectories%

REM /SUBSYSTEM:windows

set CompileStatic=false
set TestLib=jpb.lib

if %CompileStatic%==true (
set TestLib=jpb_s.lib
goto static
)

echo.
echo compillingDll
echo -----------------------------------------------
echo.

cl %CompilerOptions% /Zi /O2x /DJPB_DLL_BUILD %FilesToCompile% %Libs% /link /dll /OUT:..\lib\jpb.dll /implib:..\lib\jpb.lib

copy ..\lib\jpb.dll e:\Projekty\ProcGen\build\jpb.dll
copy ..\lib\jpb.dll e:\Projekty\xWorkingProjects\RoqueLike\build\jpb.dll
copy ..\lib\jpb.dll ..\build\jpb.dll

echo.
echo -----------------------------------------------

goto test
:static

echo.
echo Compilling Static lib
echo -----------------------------------------------
echo.

cl %CompilerOptions% /c /O2x /Zi %FilesToCompile% %Libs%
lib /nologo %FilesToLink% /OUT:..\lib\jpb_s.lib

echo.
echo -----------------------------------------------

:test

echo.
echo compilingTest
echo -----------------------------------------------
echo.

cl %CompilerOptions% /Zi ..\jpb\test.cpp ..\lib\%TestLib%

echo.
echo -----------------------------------------------

popd
