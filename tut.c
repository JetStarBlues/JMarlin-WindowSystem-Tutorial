#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "olcPGE_min.h"
#include "params.h"
#include "tut.h"


struct _Context* gContext;  // hmmm...





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

	return list;
}

int list_addValue ( struct _List* list, void* value )
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
	}


	//
	list->nItems += 1;

	return 1;
}

void* list_getNodeValue ( struct _List* list, unsigned int index )
{
	struct _ListNode* curNode;
	unsigned int      i;

	if ( ( list->nItems == 0 ) || ( index >= list->nItems ) )
	{
		return NULL;
	}


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
	unsigned int     x,
	unsigned int     y,
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






struct _Window* window_new (

	unsigned int     x,
	unsigned int     y,
	unsigned int     width,
	unsigned int     height,
	struct _Context* context,
	uint32_t         color  // temp
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
	window->context = context;
	window->color   = color;  // temp

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


	desktop->context = context;

	return desktop;
}

struct _Window* desktop_createWindow (

	struct _Desktop* desktop,
	unsigned int     x,
	unsigned int     y,
	unsigned int     width,
	unsigned int     height,
	uint32_t         color  // temp
)
{
	struct _Window* window;
	int             rStatus;

	window = window_new( x, y, width, height, desktop->context, color );

	if ( window == NULL )
	{
		return NULL;
	}


	rStatus = list_addValue( desktop->children, ( void* ) window );

	if ( rStatus == 0 )
	{
		free( window );

		return NULL;
	}


	return window;
}

void desktop_paint ( struct _Desktop* desktop )
{
	unsigned int    i;
	// struct _Window* window;
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
}










void tut_init ( void )
{
	gContext = context_new( SCREEN_WIDTH, SCREEN_HEIGHT );
}

void tut_main ( void )
{
	struct _Desktop* desktop;

	desktop = desktop_new( gContext );

	desktop_createWindow( desktop,  10,  10, 300, 200, 0xFF0000FF );
	desktop_createWindow( desktop, 100, 150, 400, 400, 0x00FF00FF );
	desktop_createWindow( desktop, 200, 100, 200, 600, 0x0000FFFF );

	desktop_paint( desktop );
}






void renderContextBuffer ( struct _Context* context )
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




bool UI_onUserCreate ( void )
{
	tut_init();

	tut_main();

	return true;
}

bool UI_onUserUpdate ( void )
{
	// tut_main();

	renderContextBuffer( gContext );

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
