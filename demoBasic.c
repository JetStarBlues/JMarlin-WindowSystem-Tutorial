#include <stdio.h>

#include "types.h"
#include "tut.h"
#include "widgets.h"


void crudeBtnTest ( struct _ToggleButton* btn, int relMouseX, int relMouseY )
{
	printf( "text:%s state:%d pos:( %d, %d )\n",

		btn->text,
		btn->isSet,
		relMouseX, relMouseY
	);
}

void demoBasic_init ( struct _Desktop* gDesktop )
{
	struct _Window*       win0;
	struct _Window*       win1;
	struct _Window*       win2;
	struct _Window*       win3;
	struct _ToggleButton* btn;
	struct _TextBox*      textbox;

	win0 = window_createChildWindow( ( struct _Window* ) gDesktop,  10,  10, 300, 200, 0 );
	win1 = window_createChildWindow( ( struct _Window* ) gDesktop, 100, 150, 400, 400, 0 );
	win2 = window_createChildWindow( ( struct _Window* ) gDesktop, 200, 100, 200, 400, 0 );


	win3 = window_createChildWindow(

		win0,
		( win0->width / 2 ) - ( 100 / 2 ),
		( win0->height / 2 ) - ( 100 / 2 ),
		100, 100,
		0
	);


	btn = toggleButton_new(

		( win2->width / 2 ) - ( 70 / 2 ),
		( win2->height / 2 ) - ( 30 / 2 ),
		70, 30
	);

	window_appendChildWindow( win2, ( struct _Window* ) btn );


	textbox = textBox_new(

		( win1->width / 2 ) - ( 200 / 2 ),
		( win1->height / 2 ) - ( 30 / 2 ),
		200, 30
	);

	window_appendChildWindow( win1, ( struct _Window* ) textbox );


	window_setTitle( win0, "win0" );
	window_setTitle( win1, "win1" );
	window_setTitle( win2, "win2" );
	window_setTitle( win3, "win3" );
	toggleButton_setText( btn, "btn" );
	btn->mouseReleaseEventHandler = crudeBtnTest;
	textBox_setText( textbox, "Greetings!" );
	textBox_appendText( textbox, " 10 Stars" );
}


void demoBasic_exit ( struct _Desktop* gDesktop )
{
	//
}

