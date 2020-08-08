#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "tut.h"
#include "widgets.h"


struct _Calculator {

	struct _Window window;  // "inherit" window class

	struct _TextBox*      display;
	struct _ToggleButton* btn0;
	struct _ToggleButton* btn1;
	struct _ToggleButton* btn2;
	struct _ToggleButton* btn3;
	struct _ToggleButton* btn4;
	struct _ToggleButton* btn5;
	struct _ToggleButton* btn6;
	struct _ToggleButton* btn7;
	struct _ToggleButton* btn8;
	struct _ToggleButton* btn9;
	struct _ToggleButton* btnAdd;
	struct _ToggleButton* btnSubtract;
	struct _ToggleButton* btnMlutiply;
	struct _ToggleButton* btnDivide;
	struct _ToggleButton* btnEqual;
	struct _ToggleButton* btnClear;
};

void calculator_buttonPressHandler ( struct _ToggleButton* btn, int relMouseX, int relMouseY );


// ------------------------------------------------------------------------------------------

static int buttonWidth       = 20;
static int buttonHeight      = 20;
static int buttonTopPadding  = 5;
static int buttonLeftPadding = 5;


int calculator_layoutGetAbsX ( int gridX )
{
	return (

		( gridX * buttonWidth ) +
		( buttonLeftPadding * ( gridX + 1 ) )
	);
}

int calculator_layoutGetAbsY ( int gridY )
{
	int absY;

	absY = (

		( gridY * buttonWidth ) +
		( buttonLeftPadding * ( gridY + 1 ) )
	);

	absY += WIN_TITLEBAR_HEIGHT;  // TODO, we shouldn't have to consider window decoration...

	return absY;
}

int calculator_layoutGetAbsWidth ( int gridWidth )
{
	return (

		( gridWidth * buttonWidth ) +
		( buttonLeftPadding * ( gridWidth - 1 ) )
	);
}

int calculator_layoutGetAbsHeight ( int gridHeight )
{
	return (

		( gridHeight * buttonHeight ) +
		( buttonTopPadding * ( gridHeight - 1 ) )
	);
}

void calculator_layoutButton ( struct _Window* winBtn, int gridX, int gridY, int gridWidth, int gridHeight )
{
	winBtn->x      = calculator_layoutGetAbsX( gridX );
	winBtn->y      = calculator_layoutGetAbsY( gridY );
	winBtn->width  = calculator_layoutGetAbsWidth( gridWidth );
	winBtn->height = calculator_layoutGetAbsHeight( gridHeight );
}

void calculator_layout ( struct _Calculator* calculator )
{
	/*
	     -------------------
	    |      display      |
	     -------------------
	     --- --- --- -------
	    | 7 | 8 | 9 |  DEL  |
	     --- --- --- -------
	    | 4 | 5 | 6 | * | / |
	     --- --- --- --- ---
	    | 1 | 2 | 3 | + | - |
	     --- --- --- -------
	    |   | 0 |   |   =   |
	     --- --- --- -------
	*/

	//
	struct _Window* winCalculator;
	int             nButtonsPerRow;
	int             nButtonsPerCol;
	int             calculatorWidth;
	int             calculatorHeight;


	//
	nButtonsPerRow   = 5;
	nButtonsPerCol   = 5;
	calculatorWidth  = ( ( buttonWidth + buttonLeftPadding ) * nButtonsPerRow ) + buttonLeftPadding;
	calculatorHeight = ( ( buttonHeight + buttonTopPadding ) * nButtonsPerCol ) + buttonTopPadding;
	calculatorHeight += WIN_TITLEBAR_HEIGHT;  // TODO, we shouldn't have to consider window decoration...


	// Set window size
	winCalculator = ( struct _Window* ) calculator;

	winCalculator->width  = calculatorWidth;
	winCalculator->height = calculatorHeight;


	// Layout buttons
	calculator_layoutButton(

		( struct _Window* ) calculator->display,
		0, 0,
		nButtonsPerRow, 1
	);

	calculator_layoutButton(

		( struct _Window* ) calculator->btn7,
		0, 1,
		1, 1
	);
	calculator_layoutButton(

		( struct _Window* ) calculator->btn8,
		1, 1,
		1, 1
	);
	calculator_layoutButton(

		( struct _Window* ) calculator->btn9,
		2, 1,
		1, 1
	);
	calculator_layoutButton(

		( struct _Window* ) calculator->btnClear,
		3, 1,
		2, 1
	);

	calculator_layoutButton(

		( struct _Window* ) calculator->btn4,
		0, 2,
		1, 1
	);
	calculator_layoutButton(

		( struct _Window* ) calculator->btn5,
		1, 2,
		1, 1
	);
	calculator_layoutButton(

		( struct _Window* ) calculator->btn6,
		2, 2,
		1, 1
	);
	calculator_layoutButton(

		( struct _Window* ) calculator->btnMlutiply,
		3, 2,
		1, 1
	);
	calculator_layoutButton(

		( struct _Window* ) calculator->btnDivide,
		4, 2,
		1, 1
	);

	calculator_layoutButton(

		( struct _Window* ) calculator->btn1,
		0, 3,
		1, 1
	);
	calculator_layoutButton(

		( struct _Window* ) calculator->btn2,
		1, 3,
		1, 1
	);
	calculator_layoutButton(

		( struct _Window* ) calculator->btn3,
		2, 3,
		1, 1
	);
	calculator_layoutButton(

		( struct _Window* ) calculator->btnAdd,
		3, 3,
		1, 1
	);
	calculator_layoutButton(

		( struct _Window* ) calculator->btnSubtract,
		4, 3,
		1, 1
	);

	calculator_layoutButton(

		( struct _Window* ) calculator->btn0,
		1, 4,
		1, 1
	);
	calculator_layoutButton(

		( struct _Window* ) calculator->btnEqual,
		3, 4,
		2, 1
	);
}


// ------------------------------------------------------------------------------------------

struct _ToggleButton* calculator_createButton ( struct _Calculator* calculator, char* label )
{
	struct _ToggleButton* btn;

	btn = toggleButton_new( 0, 0, 0, 0 );

	if ( btn == NULL )
	{
		return NULL;
	}

	window_appendChildWindow( ( struct _Window* ) calculator, ( struct _Window* ) btn );

	toggleButton_setText( btn, label );

	btn->mouseReleaseEventHandler = calculator_buttonPressHandler;

	return btn;
}

struct _TextBox* calculator_createDisplay ( struct _Calculator* calculator )
{
	struct _TextBox* display;

	display = textBox_new( 0, 0, 0, 0 );

	if ( display == NULL )
	{
		return NULL;
	}

	window_appendChildWindow( ( struct _Window* ) calculator, ( struct _Window* ) display );

	return display;
}

struct _Calculator* calculator_new ( void )
{
	struct _Calculator* calculator;
	struct _Window*     winCalculator;
	int                 status;


	// Allocate space
	calculator = ( struct _Calculator* ) malloc( sizeof( struct _Calculator ) );

	if ( calculator == NULL )
	{
		return NULL;
	}


	// Initialize the window part of the calculator
	winCalculator = ( struct _Window* ) calculator;

	status = window_init(

		winCalculator,
		0,              // x
		0,              // y
		0,              // width
		0,              // height
		0,              // flags
		NULL            // context
	);

	if ( status == ST_FAIL )
	{
		free( calculator );

		return NULL;
	}

	window_setTitle( winCalculator, "calculator" );


	// Initialize the calculator specific parts
	{
		// Create the display
		calculator->display = calculator_createDisplay( calculator );

		// Create the buttons
		calculator->btn0        = calculator_createButton( calculator, "0" );
		calculator->btn1        = calculator_createButton( calculator, "1" );
		calculator->btn2        = calculator_createButton( calculator, "2" );
		calculator->btn3        = calculator_createButton( calculator, "3" );
		calculator->btn4        = calculator_createButton( calculator, "4" );
		calculator->btn5        = calculator_createButton( calculator, "5" );
		calculator->btn6        = calculator_createButton( calculator, "6" );
		calculator->btn7        = calculator_createButton( calculator, "7" );
		calculator->btn8        = calculator_createButton( calculator, "8" );
		calculator->btn9        = calculator_createButton( calculator, "9" );
		calculator->btnAdd      = calculator_createButton( calculator, "+" );
		calculator->btnSubtract = calculator_createButton( calculator, "_" );
		calculator->btnMlutiply = calculator_createButton( calculator, "*" );
		calculator->btnDivide   = calculator_createButton( calculator, "/" );
		calculator->btnClear    = calculator_createButton( calculator, "DEL" );
		calculator->btnEqual    = calculator_createButton( calculator, "=" );

		// Layout the calculator
		calculator_layout( calculator );
	}


	//
	return calculator;
}

/*void calculator_free ( struct _Window* winCalculator )
{
	struct _Calculator* calculator;

	calculator = ( struct _Calculator* ) winCalculator;
}*/


// ------------------------------------------------------------------------------------------

void calculator_buttonPressHandler ( struct _ToggleButton* btn, int relMouseX, int relMouseY )
{
	struct _Calculator* calculator;
	struct _Window*     winBtn;

	winBtn     = ( struct _Window* ) btn;
	calculator = ( struct _Calculator* ) winBtn->parent;


	if ( btn == calculator->btn0 )
	{
		textBox_appendText( calculator->display, "0" );
	}
	if ( btn == calculator->btn1 )
	{
		textBox_appendText( calculator->display, "1" );
	}
	if ( btn == calculator->btn2 )
	{
		textBox_appendText( calculator->display, "2" );
	}
	if ( btn == calculator->btn3 )
	{
		textBox_appendText( calculator->display, "3" );
	}
	if ( btn == calculator->btn4 )
	{
		textBox_appendText( calculator->display, "4" );
	}
	if ( btn == calculator->btn5 )
	{
		textBox_appendText( calculator->display, "5" );
	}
	if ( btn == calculator->btn6 )
	{
		textBox_appendText( calculator->display, "6" );
	}
	if ( btn == calculator->btn7 )
	{
		textBox_appendText( calculator->display, "7" );
	}
	if ( btn == calculator->btn8 )
	{
		textBox_appendText( calculator->display, "8" );
	}
	if ( btn == calculator->btn9 )
	{
		textBox_appendText( calculator->display, "9" );
	}


	if ( btn == calculator->btnClear )
	{
		textBox_setText( calculator->display, NULL );
	}
}




// ==========================================================================================

static struct _Window* parentWindow;  // ugly...


void demoCalculator_spawnCalculator ( struct _ToggleButton* btn, int relMouseX, int relMouseY )
{
	struct _Calculator* calculator;

	calculator = calculator_new();

	window_appendChildWindow( parentWindow, ( struct _Window* ) calculator );

	// window_moveWindow( ( struct _Window* ) calculator, 0, 0 );  // also forces a redraw?
	window_raiseWindow( ( struct _Window* ) calculator );  // force redraw...
}

// ------------------------------------------------------------------------------------------

void demoCalculator_init ( struct _Desktop* gDesktop )
{
	struct _ToggleButton* launchBtn;

	parentWindow = ( struct _Window* ) gDesktop;

	{
		launchBtn = toggleButton_new(

			20, 20,
			150, 30
		);

		window_appendChildWindow( parentWindow, ( struct _Window* ) launchBtn );

		toggleButton_setText( launchBtn, "New Calculator" );

		launchBtn->mouseReleaseEventHandler = demoCalculator_spawnCalculator;
	}

demoCalculator_spawnCalculator( NULL, 0, 0 );
}

void demoCalculator_exit ( struct _Desktop* gDesktop )
{
	//
}

