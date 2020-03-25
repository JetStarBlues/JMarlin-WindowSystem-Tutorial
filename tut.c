// http://www.trackze.ro/wsbe-complicated-rectangles/

#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "olcPGE_min.h"
#include "params.h"
#include "tut.h"


struct _Context* gContext;  // hmmm...
struct _Desktop* gDesktop;  // hmmm...
int16_t          gMouseX;
int16_t          gMouseY;
uint8_t          gLeftBtnState;





struct _ListNode* listNode_new ( void* value )
{
	struct _ListNode* listNode;

	listNode = ( struct _ListNode* ) malloc( sizeof( struct _ListNode ) );

	if ( listNode == NULL )
	{
		return NULL;
	}

	listNode->value = value;
	listNode->prev  = NULL;
	listNode->next  = NULL;

	return listNode;
}

struct _List* list_new ( void )
{
	struct _List* list;

	list = ( struct _List* ) malloc( sizeof( struct _List ) );

	if ( list == NULL )
	{
		return NULL;
	}

	list->nItems   = 0;
	list->rootNode = NULL;
	list->tailNode = NULL;

	return list;
}

/*
   List is not circular. Ex:

       a
       .next = b
       .prev = NULL
           b
           .next = c
           .prev = a
               c
               .next = NULL
               .prev = b

       list.rootNode = a
       list.tailNode = c
*/
int list_appendNode ( struct _List* list, void* value )
{
	struct _ListNode* newNode;
	struct _ListNode* curNode;

	newNode = listNode_new( value );

	if ( newNode == NULL )
	{
		return 0;
	}


	// If the list is empty, the node becomes the root node
	if ( list->rootNode == NULL )
	{
		list->rootNode = newNode;
	}

	// Otherwise, insert the node at the tail of the list
	else
	{
		curNode = list->rootNode;

		while ( curNode->next )
		{
			curNode = curNode->next;
		}

		curNode->next = newNode;
		newNode->prev = curNode;

		// Mark as new tail
		list->tailNode = newNode;
	}


	//
	list->nItems += 1;

	return 1;
}

void* list_removeNode ( struct _List* list, unsigned int index )
{
	struct _ListNode* curNode;
	void*             value;
	unsigned int      i;

	// Early exit
	if ( ( list->nItems == 0 ) || ( index >= list->nItems ) )
	{
		return NULL;
	}


	// Find node
	curNode = list->rootNode;
	i       = 0;

	while ( i < index )
	{
		curNode = curNode->next;

		i += 1;
	}

	// Save value
	value = curNode->value;


	// Point neighbours to each other
	if ( curNode->prev )
	{
		curNode->prev->next = curNode->next;
	}

	if ( curNode->next )
	{
		curNode->next->prev = curNode->prev;
	}

	// If node is root, make next node new root
	if ( i == 0 )
	{
		list->rootNode = curNode->next;
	}

	// If node is tail, make prev node new tail
	if ( i == 0 )
	{
		list->tailNode = curNode->prev;
	}


	//
	free( curNode );

	list->nItems -= 1;


	//
	return value;
}

void* list_getNodeValue ( struct _List* list, unsigned int index )
{
	struct _ListNode* curNode;
	unsigned int      i;

	// Early exit
	if ( ( list->nItems == 0 ) || ( index >= list->nItems ) )
	{
		return NULL;
	}


	// Find node
	curNode = list->rootNode;
	i       = 0;

	while ( i < index )
	{
		curNode = curNode->next;

		i += 1;
	}

	return curNode->value;
}






struct _Context* context_new ( unsigned int width, unsigned int height )
{
	struct _Context* context;

	context = ( struct _Context* ) malloc( sizeof( struct _Context ) );

	if ( context == NULL )
	{
		return NULL;
	}

	context->width   = width;
	context->height  = height;
	context->nPixels = width * height;

	context->frameBuffer = ( uint32_t* ) malloc( sizeof( uint32_t ) * context->nPixels );

	return context;
}


void context_fillRect (

	struct _Context* context,
	int              x,
	int              y,
	unsigned int     width,
	unsigned int     height,
	uint32_t         color
)
{
	unsigned int curX;
	unsigned int maxX;
	unsigned int maxY;
	unsigned int idx;

	maxX = x + width;
	maxY = y + height;

	// Check bounds
	if ( maxX > context->width )
	{
		maxX = context->width;
	}

	if ( maxY > context->height )
	{
		maxY = context->height;
	}

	if ( x < 0 )  // JK...
	{
		x = 0;
	}

	if ( y < 0 )
	{
		y = 0;
	}


	// Draw the rectangle
	while ( y < maxY )
	{
		for ( curX = x; curX < maxX; curX += 1 )
		{
			idx = ( y * context->width ) + curX;

			context->frameBuffer[ idx ] = color;
		}

		y += 1;
	}
}

void context_setPixel (

	struct _Context* context,
	int              x,
	int              y,
	uint32_t         color
)
{
	unsigned int idx;

	// Check bounds
	if (

		( x < 0 )                ||
		( x >= context->width )  ||
		( y < 0 )                ||
		( y >= context->height )
	)
	{
		return;
	}

	idx = ( y * context->width ) + x;

	context->frameBuffer[ idx ] = color;
}






struct _Window* window_new (

	int              x,
	int              y,
	unsigned int     width,
	unsigned int     height,
	uint32_t         color,
	struct _Context* context
)
{
	struct _Window* window;

	window = ( struct _Window* ) malloc( sizeof( struct _Window ) );

	if ( window == NULL )
	{
		return NULL;
	}

	window->x       = x;
	window->y       = y;
	window->width   = width;
	window->height  = height;
	window->color   = color;
	window->context = context;

	return window;
}

void window_paint ( struct _Window* window )
{
	context_fillRect(

		window->context,
		window->x,
		window->y,
		window->width,
		window->height,
		window->color
	);
}






void cursor_paint ( struct _Context* context, int x, int y )
{
	context_setPixel( context, x,     y,     0xFFFFFFFF );
	context_setPixel( context, x - 1, y,     0x000000FF );
	context_setPixel( context, x + 1, y,     0x000000FF );
	context_setPixel( context, x,     y + 1, 0x000000FF );
	context_setPixel( context, x,     y - 1, 0x000000FF );
}





struct _Desktop* desktop_new ( struct _Context* context )
{
	struct _Desktop* desktop;

	desktop = ( struct _Desktop* ) malloc( sizeof( struct _Desktop ) );

	if ( desktop == NULL )
	{
		return NULL;
	}


	desktop->children = list_new();

	if ( desktop->children == NULL )
	{
		free( desktop );

		return NULL;
	}


	desktop->context    = context;
	desktop->mouseX     = 0;
	desktop->mouseY     = 0;
	desktop->dragTarget = NULL;

	return desktop;
}

struct _Window* desktop_createWindow (

	struct _Desktop* desktop,
	int              x,
	int              y,
	unsigned int     width,
	unsigned int     height,
	uint32_t         color
)
{
	struct _Window* window;
	int             rStatus;

	window = window_new( x, y, width, height, color, desktop->context );

	if ( window == NULL )
	{
		return NULL;
	}


	rStatus = list_appendNode( desktop->children, ( void* ) window );

	if ( rStatus == 0 )
	{
		free( window );

		return NULL;
	}


	return window;
}

void desktop_paint ( struct _Desktop* desktop )
{
	struct _ListNode* child;

	// Clear desktop
	context_fillRect(

		desktop->context,
		0,
		0,
		desktop->context->width,
		desktop->context->height,
		// 0xABCDEFFF
		0x000000FF
	);


	// Draw windows
	child = desktop->children->rootNode;

	while ( child != NULL )
	{
		window_paint( ( struct _Window* ) child->value );

		child = child->next;
	}


	// Draw mouse
	cursor_paint( desktop->context, desktop->mouseX, desktop->mouseY );
}

void desktop_raiseWindow ( struct _Desktop* desktop )
{
	struct _ListNode* child;
	struct _Window*   window;
	unsigned int      nChildren;
	unsigned int      i;

	nChildren = desktop->children->nItems;

	if ( nChildren == 0 )
	{
		return;
	}
	else if ( nChildren == 1 )
	{
		child = desktop->children->rootNode;
	}
	else
	{
		child = desktop->children->tailNode;
	}


	i = nChildren - 1;

	// Go through list backwards, because tail is topmost (drawn last)
	while ( child != NULL )
	{
		window = ( struct _Window* ) child->value;

		// Mouse inside window
		if (

			( desktop->mouseX >= window->x )                     &&
			( desktop->mouseX < ( window->x + window->width ) )  &&
			( desktop->mouseY >= window->y )                     &&
			( desktop->mouseY < ( window->y + window->height ) )
		)
		{
			// Raise window to top of list
			list_removeNode( desktop->children, i );

			list_appendNode( desktop->children, ( void* ) window );  // new tail


			// Update drag target and offset...
			desktop->dragTarget  = window;
			desktop->dragOffsetX = desktop->mouseX - window->x;
			desktop->dragOffsetY = desktop->mouseY - window->y;


			// Done
			break;
		}


		child  = child->prev;
		i     -= 1;
	}
}

void desktop_dragWindow ( struct _Desktop* desktop )
{
	/* Applying the offset makes sure that the corner of the
	   window does not suddenly snap to the mouse location
	*/
	desktop->dragTarget->x = desktop->mouseX - desktop->dragOffsetX;
	desktop->dragTarget->y = desktop->mouseY - desktop->dragOffsetY;
}

void desktop_processMouse (

	struct _Desktop* desktop,
	int16_t          mouseX,
	int16_t          mouseY,
	uint8_t          leftBtnState
)
{
	// Save
	desktop->mouseX = mouseX;
	desktop->mouseY = mouseY;


	// Button currently pressed
	if ( leftBtnState )
	{
		// Button was previously released
		if ( desktop->prevLeftBtnState == 0 )
		{
			// Raise pressed window
			desktop_raiseWindow( desktop );
		}
	}

	// Button currently released
	else
	{
		desktop->dragTarget = NULL;
	}


	//
	if ( desktop->dragTarget != NULL )
	{
		// Drag pressed window
		desktop_dragWindow( desktop );
	}


	// Update screen to reflect changes
	desktop_paint( desktop );

	// Save state
	desktop->prevLeftBtnState = leftBtnState;
}










void tut_init ( void )
{
	gContext = context_new( SCREEN_WIDTH, SCREEN_HEIGHT );

	gDesktop = desktop_new( gContext );

	desktop_createWindow( gDesktop,  10,  10, 300, 200, 0xFF0000FF );
	desktop_createWindow( gDesktop, 100, 150, 400, 400, 0x00FF00FF );
	desktop_createWindow( gDesktop, 200, 100, 200, 400, 0x0000FFFF );

	desktop_paint( gDesktop );
}

void tut_main ( void )
{
	// Poll mouse status...
	desktop_processMouse( gDesktop, gMouseX, gMouseY, gLeftBtnState );

	//
}






void olcGlue_renderContextBuffer ( struct _Context* context )
{
	unsigned int i;
	unsigned int x;
	unsigned int y;
	uint32_t     color;
	uint8_t      r;
	uint8_t      g;
	uint8_t      b;

	i = 0;
	x = 0;
	y = 0;

	while ( i < context->nPixels )
	{
		color = context->frameBuffer[ i ];
		r = ( color >> 24 ) & 0xFF;
		g = ( color >> 16 ) & 0xFF;
		b = ( color >>  8 ) & 0xFF;

		PGE_drawRGB( x, y, r, g, b );

		i += 1;
		x += 1;

		if ( x == context->width )
		{
			x  = 0;
			y += 1;
		}
	}
}

void olcGlue_getMouseStatus ( void )
{
	gMouseX = PGE_getMouseX();
	gMouseY = PGE_getMouseY();

	gLeftBtnState = PGE_getMouse( MOUSE_LEFT ).bHeld;
}





bool UI_onUserCreate ( void )
{
	tut_init();

	return true;
}

bool UI_onUserUpdate ( void )
{
	olcGlue_getMouseStatus();

	tut_main();

	olcGlue_renderContextBuffer( gContext );

	return true;
}

bool UI_onUserDestroy ( void )
{
	return true;
}


int main ( void )
{
	if ( PGE_construct( SCREEN_WIDTH, SCREEN_HEIGHT, 1, 1, "WSBE Tutorial" ) )
	{
		PGE_start();
	}

	return 0;
}
