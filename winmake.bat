@echo off

rem Windows (VS command prompt)

set src_files=^
	"olcPGE_min_x11_gdi.c" ^
	"widgets.c"            ^
	"tut.c"                ^
	"demoBasic.c"          ^
	"demoCalculator.c"

cl %src_files% /Fo"binWin/" /Fe"binWin/main.exe"
