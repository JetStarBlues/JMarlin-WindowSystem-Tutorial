#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "types.h"
#include "tut.h"
#include "widgets.h"




/* ==========================================================================================
   ...
   ========================================================================================== */

struct _ToggleButton* toggleButton_new ( int x, int y, int w, int h )
{
	struct _ToggleButton* toggleButton;
	struct _Window*       winToggleButton;
	uint16_t              winFlags;
	int                   status;

	// Allocate space for the button
	toggleButton = ( struct _ToggleButton* ) malloc( sizeof( struct _ToggleButton ) );

	if ( toggleButton == NULL )
	{
		return NULL;
	}


	// Initialize the window part of the button
	winToggleButton = ( struct _Window* ) toggleButton;

	winFlags = WIN_FLAG_NO_DECORATION;

	status = window_init(

		winToggleButton,
		x, y,
		w, h,
		winFlags,
		NULL
	);

	if ( status == FAIL )
	{
		free( toggleButton );

		return NULL;
	}


	// Initialize the toggleButton specific parts
	toggleButton->isSet = 0;

	winToggleButton->paintHandler             = toggleButton_paintHandler;
	winToggleButton->mouseReleaseEventHandler = toggleButton_mouseReleaseEventHandler;


	//
	return toggleButton;
}

void toggleButton_free ( struct _ToggleButton* toggleButton )
{
	// TODO
}


// ------------------------------------------------------------------------------------------

void toggleButton_paintHandler ( struct _Window* winToggleButton )  // uses relative positions
{
	struct _ToggleButton* toggleButton;
	uint32_t              color;

	toggleButton = ( struct _ToggleButton* ) winToggleButton;


	if ( toggleButton->isSet == 1 )
	{
		color = TOGGLEBTN_SET_COLOR;
	}
	else
	{
		color = TOGGLEBTN_COLOR;
	}

	context_fillRect(

		winToggleButton->context,
		0,
		0,
		winToggleButton->width,
		winToggleButton->height,
		color
	);
}


// ------------------------------------------------------------------------------------------

void toggleButton_mouseReleaseEventHandler ( struct _Window* winToggleButton, int relMouseX, int relMouseY )
{
	struct _ToggleButton* toggleButton;

	toggleButton = ( struct _ToggleButton* ) winToggleButton;

	// Toggle button state
	if ( toggleButton->isSet == 1 )
	{
		toggleButton->isSet = 0;
	}
	else
	{
		toggleButton->isSet = 1;
	}

	// Request a repaint
	window_invalidate(

		winToggleButton,
		0, 0,
		winToggleButton->width,
		winToggleButton->height,
		0
	);
}
