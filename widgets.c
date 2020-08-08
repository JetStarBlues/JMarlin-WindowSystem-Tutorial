#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "types.h"
#include "tut.h"
#include "widgets.h"




/* ==========================================================================================
   ...
   ========================================================================================== */

struct _ToggleButton* toggleButton_new ( int x, int y, int width, int height )
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
		width, height,
		winFlags,
		NULL
	);

	if ( status == ST_FAIL )
	{
		free( toggleButton );

		return NULL;
	}


	// Initialize the toggleButton specific parts
	toggleButton->isSet                    = 0;
	toggleButton->text                     = NULL;
	toggleButton->mouseReleaseEventHandler = NULL;  // specific to instance

	winToggleButton->paintHandler             = toggleButton_paintHandler;
	winToggleButton->mouseReleaseEventHandler = toggleButton_defaultMouseReleaseEventHandler;
	winToggleButton->freeHandler              = toggleButton_free;


	//
	return toggleButton;
}

void toggleButton_free ( struct _Window* winToggleButton )
{
	struct _ToggleButton* toggleButton;

	toggleButton = ( struct _ToggleButton* ) winToggleButton;

	free( toggleButton->text );
}


// ------------------------------------------------------------------------------------------

void toggleButton_setText ( struct _ToggleButton* toggleButton, char* text )
{
	struct _Window* winToggleButton;

	// Free existing text
	if ( toggleButton->text != NULL )
	{
		free( toggleButton->text );
	}


	// If new text is NULL, assume intent was to clear text
	if ( text == NULL )
	{
		toggleButton->text = NULL;
	}
	// Otherwise, set new text
	else
	{
		toggleButton->text = strdup( text );
	}


	// Repaint to show change
	winToggleButton = ( struct _Window* ) toggleButton;

	window_invalidate(

		winToggleButton,
		0, 0,
		winToggleButton->width,
		winToggleButton->height,
		0
	);
}


// ------------------------------------------------------------------------------------------

void toggleButton_paintHandler ( struct _Window* winToggleButton )  // uses relative positions
{
	struct _ToggleButton* toggleButton;
	uint32_t              buttonColor;
	uint32_t              buttonTextColor;
	int                   textTopPadding;
	int                   textLeftPadding;
	int                   textWidth;

	//
	toggleButton = ( struct _ToggleButton* ) winToggleButton;

	//
	if ( toggleButton->isSet == 1 )
	{
		buttonColor     = TOGGLEBTN_SET_COLOR;
		buttonTextColor = TOGGLEBTN_SET_TEXT_COLOR;
	}
	else
	{
		buttonColor     = TOGGLEBTN_COLOR;
		buttonTextColor = TOGGLEBTN_TEXT_COLOR;
	}


	// Draw background
	context_fillRect(

		winToggleButton->context,
		0,
		0,
		winToggleButton->width,
		winToggleButton->height,
		buttonColor
	);

	// Draw text (centered)
	if ( toggleButton->text != NULL )
	{
		textWidth = strlen( toggleButton->text ) * curFontWidth;

		if ( winToggleButton->width > textWidth )
		{
			textLeftPadding = ( winToggleButton->width - textWidth ) / 2;
		}
		else
		{
			textLeftPadding = 0;
		}

		if ( winToggleButton->height > curFontHeight )
		{
			textTopPadding = ( winToggleButton->height - curFontHeight ) / 2;
		}
		else
		{
			textTopPadding = 0;
		}

		context_drawString(

			winToggleButton->context,
			toggleButton->text,
			textLeftPadding,
			textTopPadding,
			buttonTextColor
		);
	}
}


// ------------------------------------------------------------------------------------------

void toggleButton_defaultMouseReleaseEventHandler ( struct _Window* winToggleButton, int relMouseX, int relMouseY )
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

	// Fire specific handler
	if ( toggleButton->mouseReleaseEventHandler != NULL )
	{
		toggleButton->mouseReleaseEventHandler( toggleButton, relMouseX, relMouseY );
	}
}




/* ==========================================================================================
   ...
   ========================================================================================== */

struct _TextBox* textBox_new ( int x, int y, int width, int height )
{
	struct _TextBox* textBox;
	struct _Window*  winTextBox;
	uint16_t         winFlags;
	int              status;

	// Allocate space for the textbox
	textBox = ( struct _TextBox* ) malloc( sizeof( struct _TextBox ) );

	if ( textBox == NULL )
	{
		return NULL;
	}


	// Initialize the window part of the textbox
	winTextBox = ( struct _Window* ) textBox;

	winFlags = WIN_FLAG_NO_DECORATION;

	status = window_init(

		winTextBox,
		x, y,
		width, height,
		winFlags,
		NULL
	);

	if ( status == ST_FAIL )
	{
		free( textBox );

		return NULL;
	}


	// Initialize the textBox specific parts
	textBox->text = NULL;

	winTextBox->paintHandler = textBox_paintHandler;
	winTextBox->freeHandler  = textBox_free;


	//
	return textBox;
}

void textBox_free ( struct _Window* winTextBox )
{
	struct _TextBox* textBox;

	textBox = ( struct _TextBox* ) winTextBox;

	free( textBox->text );
}


// ------------------------------------------------------------------------------------------

void textBox_setText ( struct _TextBox* textBox, char* text )
{
	struct _Window* winTextBox;

	// Free existing text
	if ( textBox->text != NULL )
	{
		free( textBox->text );
	}


	// If new text is NULL, assume intent was to clear text
	if ( text == NULL )
	{
		textBox->text = NULL;
	}
	// Otherwise, set new text
	else
	{
		textBox->text = strdup( text );
	}


	// Repaint to show change
	winTextBox = ( struct _Window* ) textBox;

	window_invalidate(

		winTextBox,
		0, 0,
		winTextBox->width,
		winTextBox->height,
		0
	);
}

void textBox_appendText ( struct _TextBox* textBox, char* text )
{
	struct _Window* winTextBox;
	char*           newText;

	//
	if ( textBox->text == NULL )
	{
		textBox_setText( textBox, text );

		return;
	}


	// Grow allocated space, and append text
	newText = realloc( textBox->text, strlen( textBox->text ) + strlen( text ) + 1 );

	if ( newText == NULL )
	{
		return;
	}

	strcat( newText, text );

	textBox->text = newText;


	// Repaint to show change
	winTextBox = ( struct _Window* ) textBox;

	window_invalidate(

		winTextBox,
		0, 0,
		winTextBox->width,
		winTextBox->height,
		0
	);
}


// ------------------------------------------------------------------------------------------

void textBox_paintHandler ( struct _Window* winTextBox )  // uses relative positions
{
	struct _TextBox* textBox;
	int              textTopPadding;
	int              textLeftPadding;
	int              textWidth;

	//
	textBox = ( struct _TextBox* ) winTextBox;


	// Draw background
	context_fillRect(

		winTextBox->context,
		0,
		0,
		winTextBox->width,
		winTextBox->height,
		0xFFFFFFFF
	);

	// Draw border
	context_strokeRect(

		winTextBox->context,
		0,
		0,
		winTextBox->width,
		winTextBox->height,
		0x000000FF,
		1
	);

	// Draw text
	if ( textBox->text != NULL )
	{
		textWidth = strlen( textBox->text ) * curFontWidth;

		if ( winTextBox->width > textWidth )
		{
			textLeftPadding = ( winTextBox->width - textWidth ) / 2;
		}
		else
		{
			textLeftPadding = 0;
		}

		if ( winTextBox->height > curFontHeight )
		{
			textTopPadding = ( winTextBox->height - curFontHeight ) / 2;
		}
		else
		{
			textTopPadding = 0;
		}

		context_drawString(

			winTextBox->context,
			textBox->text,
			textLeftPadding,
			textTopPadding,
			0x000000FF
		);
	}
}


