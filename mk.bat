@echo off
rem lcc.exe 作成バッチ.
rem
rem ※ lcc 自体のコンパイルは、下記の複数のコンパイラで行えるが、
rem 出来た lcc.exe でのコンパイルは現状 vc,bcc32 版でしか行えない。
rem
rem 他コンパイラで対応するにはアセンブラに何を使うかで lcc 側の
rem アセンブラコード出力を修正したりライブラリの扱いをどうこうする
rem 必要がある。

setlocal EnableDelayedExpansion

set ADD_LCC_COPTS=
set PROGFILES=%ProgramFiles(x86)%
if "%PROGFILES%"=="" set PROGFILES=%ProgramFiles%

set COMPI=%1
if "%COMPI%"=="" set COMPI=%COMPILER%

if /I "%2" == "simple" (
  set LIBDIR=lib
  set BINDIR=bin
  set WORKDIR=bld
) else (
  set LIBDIR=lib_%COMPI%
  set BINDIR=bin_%COMPI%
  set WORKDIR=bld_%COMPI%
)
set BINDIR1=%BINDIR%_self1
set WORKDIR1=%WORKDIR%_self1
set BINDIR2=%BINDIR%_self2
set WORKDIR2=%WORKDIR%_self2

if /I "%1" == "clean"   goto L_CLEAN
if /I "%2" == "clean"   goto L_CLEAN
if /I "%3" == "clean"   goto L_CLEAN

if /I "%COMPI%" == "vc71"     goto L_VC71
if /I "%COMPI%" == "vc8"      goto L_VC8
if /I "%COMPI%" == "vc9"      goto L_VC9
if /I "%COMPI%" == "vc10"     goto L_VC10
if /I "%COMPI%" == "vc11"     goto L_VC11
if /I "%COMPI%" == "bcc"      goto L_BCC
if /I "%COMPI%" == "bccJwasm" goto L_BCC

if /I "%COMPI%" == "ow"       goto L_OW
if /I "%COMPI%" == "dmc"      goto L_DMC
if /I "%COMPI%" == "mingw"    goto L_MINGW
if /I "%COMPI%" == "clean"    goto L_CLEAN

:non
echo usage: mk.bat COMPILER_NAME
echo  [COMPILER_NAME]
echo     vc71,vc8,vc9,vc10,vc11
echo     bcc(bcc32+tasm32), bccJwasm(bcc32+jasm)
goto END


:L_VC
rem -------------------------
rem 使用している vc のパスを設定.
:L_VC71
  set "COMPILERDIR=%PROGFILES%\Microsoft Visual Studio 2003\VC7"
  set CCLIBTYPELIB=libc.lib
  goto L_VC
:L_VC8
  set "COMPILERDIR=%PROGFILES%\Microsoft Visual Studio 8\VC"
  set CCLIBTYPELIB=libcmt.lib
  goto L_VC
:L_VC9
  set "COMPILERDIR=%PROGFILES%\Microsoft Visual Studio 9.0\VC"
  set CCLIBTYPELIB=libcmt.lib
  goto L_VC
:L_VC10
  set "COMPILERDIR=%PROGFILES%\Microsoft Visual Studio 10.0\VC"
  set CCLIBTYPELIB=libcmt.lib
  goto L_VC
:L_VC11
  set "COMPILERDIR=%PROGFILES%\Microsoft Visual Studio 11.0\VC"
  set CCLIBTYPELIB=libcmt.lib
  goto L_VC
:L_VC
  rem set COMPILERDIR=%
  rem rem CCLIBTYPE=-MD
  rem set CCLIBTYPELIB=msvcrt.lib
  rem -------------------------
  set COPTS=-DWIN32 -Isrc
  set CC=cl -c %COPTS%
  set LD=cl %COPTS%
  set AR=lib
  set FO=-Fo%WORKDIR%\
  set FE=-Fe%BINDIR%\
  set LA=%WORKDIR%
  set FL=-OUT:%LIBDIR%\liblcc.lib
  set FR=
  if not exist %BINDIR% mkdir %BINDIR%
  if /I "%COMPI%" == "vc71" (
    echo cpp=$0\cpp.exe -D__LCC_WITH_VC7__ -i$0\..\include\x86\Win32\_lcc_with_vc.h $1 $2 $3	>%BINDIR%\lcc.cfg
  ) else (
    echo cpp=$0\cpp.exe -i$0\..\include\x86\Win32\_lcc_with_vc.h $1 $2 $3	>%BINDIR%\lcc.cfg
  )
  echo	inc=-I$0\..\include\x86\win32 -I"%COMPILERDIR%\include"	>>%BINDIR%\lcc.cfg
  echo	com=$0\rcc.exe -target=x86/win32 $1 $2 $3	>>%BINDIR%\lcc.cfg
  echo	asm=ml.exe -nologo -c -Cp -coff -Fo$3 $1 $2	>>%BINDIR%\lcc.cfg
  echo	lnk=link.exe -nologo -nodefaultlib -subsystem:console -entry:mainCRTStartup $2 -OUT:$3 $1 $0\..\%LIBDIR%\liblcc.lib %CCLIBTYPELIB% oldnames.lib kernel32.lib user32.lib >>%BINDIR%\lcc.cfg
  goto L_COMPILE_COMPILER


:L_BCC
  set COMPILERDIR=C:\borland\bcc55
  set COPTS=-DWIN32 -Isrc -D_spawnvp=spawnvp
  set ADD_LCC_COPTS=-D_spawnvp=spawnvp
  set CC=bcc32 -c %COPTS% 
  set LD=bcc32 %COPTS%
  set AR=tlib
  set FO=-o%WORKDIR%\
  set FE=-e%BINDIR%\
  set LA=+%WORKDIR%
  set FL=%LIBDIR%\liblcc.lib
  set FR=
  if not exist %BINDIR% mkdir %BINDIR%
  echo	cpp=$0\cpp.exe -i$0\..\include\x86\Win32\_lcc_with_bcc.h $1 $2 $3	>%BINDIR%\lcc.cfg
  echo	inc=-I$0\..\include\x86\win32 -I"%COMPILERDIR%\include"	>>%BINDIR%\lcc.cfg
  echo	com=$0\rcc.exe -target=x86/win32 $1 $2 $3	>>%BINDIR%\lcc.cfg
  if /I "%COMPI%" == "bccJwasm" (
    echo asm=jwasm.exe $1 -Fo$3 $2	>>%BINDIR%\lcc.cfg
  ) else (
    echo asm=tasm32.exe /ml $1 $2, $3	>>%BINDIR%\lcc.cfg
  )
  echo	lnk=ilink32.exe $1 c0x32.obj $2, $3 , , $0\..\%LIBDIR%\liblcc.lib cw32.lib kernel32.lib user32.lib >>%BINDIR%\lcc.cfg
  goto L_COMPILE_COMPILER


:L_OW
  set COMPILERDIR=C:\tools\watcom1.9
  set COPTS=-DWIN32 -Isrc -D_P_WAIT=P_WAIT -D_spawnvp=spawnvp
  set CC=owcc -c %COPTS% 
  set LD=owcc %COPTS%
  set AR=wlib -c -n
  set FO=-o%WORKDIR%\
  set FE=-o%BINDIR%\
  set LA=+%WORKDIR%
  set FL=%LIBDIR%\liblcc.lib
  set FR=
  rem TODO:リンク関係('～.obj'付近にディレクティブエラーがあります")。リンクできるprintf等のあるcdeclな標準ライブラリが必要.
  if not exist %BINDIR% mkdir %BINDIR%
  echo	cpp=$0\cpp.exe -i$0\..\include\x86\Win32\_lcc_with_watcom.h $1 $2 $3	>%BINDIR%\lcc.cfg
  echo	inc=-I$0\..\include\x86\win32 -I"%COMPILERDIR%\h"	>>%BINDIR%\lcc.cfg
  echo	com=$0\rcc.exe -target=x86/win32 $1 $2 $3	>>%BINDIR%\lcc.cfg
  echo	asm=wasm.exe -Fo$3 $1 $2			>>%BINDIR%\lcc.cfg
  echo	lnk=wlink.exe F $2 N $3 L $0\..\%LIBDIR%\liblcc.lib L clib3s.lib L plibmt3s.lib L user32.lib L kernel32.lib >>%BINDIR%\lcc.cfg
  goto L_COMPILE_COMPILER


:L_DMC
  set COMPILERDIR=C:\dm
  set COPTS=-DWIN32 -Isrc
  set CC=dmc -c %COPTS% 
  set LD=dmc %COPTS%
  set AR=lib
  set FO=-o%WORKDIR%\
  set FE=-o%BINDIR%\
  set LA=%WORKDIR%
  set FL=-c %LIBDIR%\liblcc.lib
  set FR=
  rem TODO: アセンブラをどうするか→とりあえずjwasm
  rem TODO: dmcのヘッダで size_t と va_list の typedef が複数行われてしまうためエラー(どうもdmcはc++互換ぽく同一複数回のtypedefを許す模様)
  rem       link もどうするか
  if not exist %BINDIR% mkdir %BINDIR%
  echo	cpp=$0\cpp.exe -i$0\..\include\x86\Win32\_lcc_with_dmc.h $1 $2 $3	>%BINDIR%\lcc.cfg
  echo	inc=-I$0\..\include\x86\win32 -I"%COMPILERDIR%\include"	>>%BINDIR%\lcc.cfg
  echo	com=$0\rcc.exe -target=x86/win32 $1 $2 $3	>>%BINDIR%\lcc.cfg
  echo	asm=jwasm.exe -Cp -Fo$3 $1 $2			>>%BINDIR%\lcc.cfg
  echo	lnk=jwlink.exe F $2 N $3 L $0\..\%LIBDIR%\liblcc.lib LIBP %COMPILERDIR%\lib L user32.lib L kernel32.lib >>%BINDIR%\lcc.cfg
  goto L_COMPILE_COMPILER


:L_MINGW
  set COMPILERDIR=C:\MinGW
  set GCC_VER=4.6.2
  set COPTS=-DWIN32 -Isrc
  set CC=gcc -c %COPTS% 
  set LD=gcc %COPTS%
  set AR=ar
  set FO=-o%WORKDIR%\
  set FE=-o%BINDIR%\
  set LA=%WORKDIR%
  set FL=q %LIBDIR%\liblcc.lib
  set FR=
  rem TODO:アセンブラをどうするか→とりあえずjwasm
  rem TODO:ライブラリの都合があわない. mingw自体がvcのdllを流用してるので大幅な修正が必要
  if not exist %BINDIR% mkdir %BINDIR%
  echo	cpp=$0\cpp.exe -i$0\..\include\x86\Win32\_lcc_with_mingw.h $1 $2 $3	>%BINDIR%\lcc.cfg
  echo	inc=-I$0\..\include\x86\win32 -I"%COMPILERDIR%\include" -I"%COMPILERDIR%\lib\gcc\mingw32\%GCC_VER%\include" >>%BINDIR%\lcc.cfg
  echo	com=$0\rcc.exe -target=x86/win32 $1 $2 $3	>>%BINDIR%\lcc.cfg
  echo	asm=jwasm.exe -coff -Cp -Fo$3 $1 $2			>>%BINDIR%\lcc.cfg
  echo	lnk=ld.exe -L %COMPILERDIR%\lib -lmsvcrt -o$3 $1 crt2.o $2 $0\../%LIBDIR%/liblcc.lib >>%BINDIR%\lcc.cfg
  goto L_COMPILE_COMPILER


:L_COMPILE_COMPILER
  if not exist %LIBDIR%  mkdir %LIBDIR%
  if not exist %WORKDIR% mkdir %WORKDIR%
  call :L_COMILE_COMPILER_SUB %BINDIR% %WORKDIR%
  rem pause

:L_SELF_COMPILE1
  set SV_PATH=%PATH%
  set "PATH=%BINDIR%;%SV_PATH%"

  if not exist %WORKDIR1% mkdir %WORKDIR1%
  if not exist %BINDIR1% mkdir %BINDIR1%

  copy %BINDIR%\lcc.cfg %BINDIR1%\lcc.cfg

  rem set COPTS=-DWIN32 -Isrc %ADD_LCC_COPTS% -v
  set COPTS=-DWIN32 -Isrc %ADD_LCC_COPTS%
  set CC=lcc -c %COPTS%
  set LD=lcc %COPTS%
  set FO=-o %WORKDIR1%\
  set FE=-o %BINDIR1%\
  call :L_COMILE_COMPILER_SUB %BINDIR1% %WORKDIR1%
  rem pause


:L_SELF_COMPILE2
  set "PATH=%BINDIR1%;%SV_PATH%"

  if not exist %WORKDIR2% mkdir %WORKDIR2%
  if not exist %BINDIR2% mkdir %BINDIR2%

  copy %BINDIR%\lcc.cfg %BINDIR2%\lcc.cfg

  rem set COPTS=-DWIN32 -Isrc %ADD_LCC_COPTS%
  set CC=lcc -c %COPTS%
  set LD=lcc %COPTS%
  set FO=-o %WORKDIR2%\
  set FE=-o %BINDIR2%\

  call :L_COMILE_COMPILER_SUB %BINDIR2% %WORKDIR2%

:L_COMPARE_SELF
  fc /b %BINDIR1%\lcc.exe %BINDIR2%\lcc.exe

  goto L_END


:L_COMILE_COMPILER_SUB
rem
set BDIR=%1
set WK=%2

rem
%CC% %FO%bprint.obj	etc\bprint.c
%LD% %FE%bprint.exe	%WK%\bprint.obj

rem
%CC% %FO%ops.obj	etc\ops.c
%LD% %FE%ops.exe	%WK%\ops.obj

rem
%CC% %FO%cpp.obj	cpp\cpp.c
%CC% %FO%lex.obj	cpp\lex.c
%CC% %FO%nlist.obj	cpp\nlist.c
%CC% %FO%tokens.obj	cpp\tokens.c
%CC% %FO%macro.obj	cpp\macro.c
%CC% %FO%eval.obj	cpp\eval.c
%CC% %FO%include.obj	cpp\include.c
%CC% %FO%hideset.obj	cpp\hideset.c
%CC% %FO%getopt.obj	cpp\getopt.c
%CC% %FO%unix.obj	cpp\unix.c
%LD% %FE%cpp.exe	%WK%\cpp.obj %WK%\lex.obj %WK%\nlist.obj %WK%\tokens.obj %WK%\macro.obj %WK%\eval.obj %WK%\include.obj %WK%\hideset.obj %WK%\getopt.obj %WK%\unix.obj

rem
%CC% %FO%lburg.obj	lburg\lburg.c
%CC% %FO%gram.obj	lburg\gram.c
%LD% %FE%lburg.exe	%WK%\lburg.obj %WK%\gram.obj

rem
%CC% %FO%lcc.obj	etc\lcc.c
%CC% %FO%win32.obj	etc\win32.c
%LD% %FE%lcc.exe	%WK%\lcc.obj %WK%\win32.obj

rem
%BDIR%\lburg.exe src\dagcheck.md %WK%\dagcheck.c
%BDIR%\lburg.exe src\alpha.md    %WK%\alpha.c
%BDIR%\lburg.exe src\mips.md     %WK%\mips.c
%BDIR%\lburg.exe src\sparc.md    %WK%\sparc.c
%BDIR%\lburg.exe src\x86.md      %WK%\x86.c
%BDIR%\lburg.exe src\x86linux.md %WK%\x86linux.c
%CC% %FO%alloc.obj	src\alloc.c
%CC% %FO%bind.obj	src\bind.c
%CC% %FO%dag.obj	src\dag.c
%CC% %FO%decl.obj	src\decl.c
%CC% %FO%enode.obj	src\enode.c
%CC% %FO%error.obj	src\error.c
%CC% %FO%expr.obj	src\expr.c
%CC% %FO%event.obj	src\event.c
%CC% %FO%init.obj	src\init.c
%CC% %FO%inits.obj	src\inits.c
%CC% %FO%input.obj	src\input.c
%CC% %FO%lex.obj	src\lex.c
%CC% %FO%list.obj	src\list.c
%CC% %FO%main.obj	src\main.c
%CC% %FO%output.obj	src\output.c
%CC% %FO%prof.obj	src\prof.c
%CC% %FO%profio.obj	src\profio.c
%CC% %FO%simp.obj	src\simp.c
%CC% %FO%stmt.obj	src\stmt.c
%CC% %FO%string.obj	src\string.c
%CC% %FO%sym.obj	src\sym.c
%CC% %FO%trace.obj	src\trace.c
%CC% %FO%tree.obj	src\tree.c
%CC% %FO%types.obj	src\types.c
%CC% %FO%null.obj	src\null.c
%CC% %FO%symbolic.obj	src\symbolic.c
%CC% %FO%gen.obj	src\gen.c
%CC% %FO%bytecode.obj	src\bytecode.c
%CC% %FO%stab.obj	src%\stab.c
%CC% %FO%dagcheck.obj	%WK%\dagcheck.c
%CC% %FO%alpha.obj	%WK%\alpha.c
%CC% %FO%mips.obj	%WK%\mips.c
%CC% %FO%sparc.obj	%WK%\sparc.c
%CC% %FO%x86.obj	%WK%\x86.c
%CC% %FO%x86linux.obj	%WK%\x86linux.c
%LD% %FE%rcc.exe %WK%\alloc.obj %WK%\bind.obj %WK%\dag.obj %WK%\dagcheck.obj %WK%\decl.obj %WK%\enode.obj %WK%\error.obj %WK%\expr.obj %WK%\event.obj %WK%\init.obj %WK%\inits.obj %WK%\input.obj %WK%\lex.obj %WK%\list.obj %WK%\main.obj %WK%\output.obj %WK%\prof.obj %WK%\profio.obj %WK%\simp.obj %WK%\stmt.obj %WK%\string.obj %WK%\sym.obj %WK%\trace.obj %WK%\tree.obj %WK%\types.obj %WK%\null.obj %WK%\symbolic.obj %WK%\gen.obj %WK%\bytecode.obj %WK%\alpha.obj %WK%\mips.obj %WK%\sparc.obj %WK%\stab.obj %WK%\x86.obj %WK%\x86linux.obj

%CC% %FO%assert.obj	lib\assert.c
%CC% %FO%yynull.obj	lib\yynull.c
%CC% %FO%bbexit.obj	lib\bbexit.c
if exist %LIBDIR%\liblcc.lib del %LIBDIR%\liblcc.lib
%AR% %FL% %LA%\assert.obj %LA%\yynull.obj %LA%\bbexit.obj %FR%

exit /b 0

:L_CLEAN
  del /Q %WORKDIR%\*.*
  del /Q %WORKDIR1%\*.*
  del /Q %WORKDIR2%\*.*
  del /Q %BINDIR%\*.*
  del /Q %BINDIR1%\*.*
  del /Q %BINDIR2%\*.*
  rd /S /Q %WORKDIR%
  rd /S /Q %WORKDIR1%
  rd /S /Q %WORKDIR2%
  rd /S /Q %BINDIR%
  rd /S /Q %BINDIR1%
  rd /S /Q %BINDIR2%
  del lib\liblcc.lib
  goto L_END

:L_END
endlocal
