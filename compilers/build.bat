@echo off
call "D:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64     
set compilerflags=/Od /Zi /EHsc
mkdir %2\out
set linkerflags=/OUT:%2\out\%~n1.exe
cl.exe %compilerflags% %1 /link %linkerflags%
