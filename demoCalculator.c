#include <stdio.h>

#include "types.h"
#include "tut.h"
#include "widgets.h"


struct _Calculator {

	struct _Window window;  // "inherit" window class

	struct _TextBox* inputBox;
	struct _Button*  btn0;
	struct _Button*  btn1;
	struct _Button*  btn2;
	struct _Button*  btn3;
	struct _Button*  btn4;
	struct _Button*  btn5;
	struct _Button*  btn6;
	struct _Button*  btn7;
	struct _Button*  btn8;
	struct _Button*  btn9;
	struct _Button*  btnAdd;
	struct _Button*  btnSubtract;
	struct _Button*  btnMlutiply;
	struct _Button*  btnDivide;
	struct _Button*  btnEnter;
	struct _Button*  btnClear;
};


struct _Calculator* calculator_new ( void )
{
	//
}

struct _Calculator* calculator_buttonPressHandler ( struct _ToggleButton* btn, int relMouseX, int relMouseY )
{
	//
}




// ==========================================================================================

void demoCalculator_init ( struct _Desktop* gDesktop )
{
	//
}

void demoCalculator_exit ( struct _Desktop* gDesktop )
{
	// TODO
}

