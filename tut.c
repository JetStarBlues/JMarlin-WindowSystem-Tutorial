// http://www.trackze.ro/wsbe-complicated-rectangles/

#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "olcPGE_min.h"
#include "params.h"
#include "tut.h"


#define DEBUG_ADD_CLIP_RECT    0
#define DEBUG_FINAL_CLIP_RECTS 0
#define DEBUG_DESKTOP_CLIP     0
#define DEBUG_WINDOW_CLIP      0


struct _Context* gContext;  // hmmm...
struct _Desktop* gDesktop;  // hmmm...
int16_t          gMouseX;
int16_t          gMouseY;
uint8_t          gLeftBtnState;




/* ==========================================================================================
   ...
   ========================================================================================== */

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

void* list_removeNode ( struct _List* list, int index )
{
	struct _ListNode* curNode;
	void*             value;
	int               i;

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

void* list_getNodeValue ( struct _List* list, int index )
{
	struct _ListNode* curNode;
	int               i;

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

void list_freeNodes ( struct _List* list )
{
	struct _ListNode* node;
	struct _ListNode* curNode;

	node = list->rootNode;

	while ( node )
	{
		curNode = node;  // save

		node = node->next;  // advance

		free( curNode );

		list->nItems -= 1;
	}

	list->rootNode = NULL;
}




/* ==========================================================================================
   ...
   ========================================================================================== */

struct _Rect* rect_new ( int top, int left, int bottom, int right )
{
	struct _Rect* rect;

	rect = ( struct _Rect* ) malloc( sizeof( struct _Rect ) );

	if ( rect == NULL )
	{
		return NULL;
	}

	rect->top    = top;
	rect->left   = left;
	rect->bottom = bottom;
	rect->right  = right;

	return rect;
}


// ------------------------------------------------------------------------------------------

int rect_rectsIntersect ( struct _Rect* rectA, struct _Rect* rectB )
{
	return (

		( rectA->left   <= rectB->right  ) &&
		( rectA->right  >= rectB->left   ) &&
		( rectA->top    <= rectB->bottom ) &&
		( rectA->bottom >= rectB->top    )
	);
}


// ------------------------------------------------------------------------------------------

/* Divide targetRect into a bunch of rects representing area visible
   when occluded by cuttingRect.
*/
struct _List* rect_split ( struct _Rect* _targetRect, struct _Rect* cuttingRect )
{
	struct _List* outputRects;
	struct _Rect  targetRect;
	struct _Rect* rect;

	//
	outputRects = list_new();

	if ( outputRects == NULL )
	{
		return NULL;
	}


	// Make a copy (so that don't modify original)
	targetRect.top    = _targetRect->top;
	targetRect.left   = _targetRect->left;
	targetRect.bottom = _targetRect->bottom;
	targetRect.right  = _targetRect->right;


	// Split by left edge - - - - - - - - - - - - - - - - - - - - - - -

	/*
		          |                      
		  OOOOOOOO|oooooooooooooooooooooo
		  OOOOOOOO|oooooooooooooooooooooo
		  OOOOOOOO|oooooooooooooooooooooo
		  OOOOOOOO|@@@@@@@@@@@@@@oooooooo
		  OOOOOOOO|@@@@@@@@@@@@@@oooooooo
		  OOOOOOOO|@@@@@@@@@@@@@@oooooooo
		  OOOOOOOO|@@@@@@@@@@@@@@oooooooo
		  OOOOOOOO|@@@@@@@@@@@@@@oooooooo
		  OOOOOOOO|@@@@@@@@@@@@@@oooooooo
		  OOOOOOOO|oooooooooooooooooooooo
		  OOOOOOOO|oooooooooooooooooooooo
		  OOOOOOOO|oooooooooooooooooooooo
		          |                      
	*/

	// If cuttingRect's left edge falls within targetRect
	if (

		( cuttingRect->left >= targetRect.left ) &&
		( cuttingRect->left <= targetRect.right )
	)
	{
		// Slice off portion of targetRect that falls to the left of the edge
		rect = rect_new(

			targetRect.top,
			targetRect.left,
			targetRect.bottom,
			cuttingRect->left - 1  // why -1?
		);

		if ( rect == NULL )
		{
			free( outputRects );

			return NULL;
		}


		// Add the new rectangle to the output list
		list_appendNode( outputRects, rect );

		// Shrink targetRect to exclude the sliced portion
		targetRect.left = cuttingRect->left;
	}


	// Split by top edge - - - - - - - - - - - - - - - - - - - - - - -

	/*
		          OOOOOOOOOOOOOOOOOOOOOO
		          OOOOOOOOOOOOOOOOOOOOOO
		          OOOOOOOOOOOOOOOOOOOOOO
		---------------------------------
		          @@@@@@@@@@@@@@oooooooo
		          @@@@@@@@@@@@@@oooooooo
		          @@@@@@@@@@@@@@oooooooo
		          @@@@@@@@@@@@@@oooooooo
		          @@@@@@@@@@@@@@oooooooo
		          @@@@@@@@@@@@@@oooooooo
		          oooooooooooooooooooooo
		          oooooooooooooooooooooo
		          oooooooooooooooooooooo
	*/

	// If cuttingRect's top edge falls within targetRect
	if (

		( cuttingRect->top >= targetRect.top ) &&
		( cuttingRect->top <= targetRect.bottom )
	)
	{
		// Slice off portion of targetRect that falls to the top of the edge
		rect = rect_new(

			targetRect.top,
			targetRect.left,
			cuttingRect->top - 1,  // why -1?
			targetRect.right
		);

		if ( rect == NULL )
		{
			list_freeNodes( outputRects );

			free( outputRects );

			return NULL;
		}


		// Add the new rectangle to the output list
		list_appendNode( outputRects, rect );

		// Shrink targetRect to exclude the sliced portion
		targetRect.top = cuttingRect->top;
	}


	// Split by right edge - - - - - - - - - - - - - - - - - - - - - - -

	/*
	                            |        
		                        |        
		                        |        
		                        |        
		          @@@@@@@@@@@@@@|OOOOOOOO
		          @@@@@@@@@@@@@@|OOOOOOOO
		          @@@@@@@@@@@@@@|OOOOOOOO
		          @@@@@@@@@@@@@@|OOOOOOOO
		          @@@@@@@@@@@@@@|OOOOOOOO
		          @@@@@@@@@@@@@@|OOOOOOOO
		          oooooooooooooo|OOOOOOOO
		          oooooooooooooo|OOOOOOOO
		          oooooooooooooo|OOOOOOOO
	                            |        
	*/

	// If cuttingRect's right edge falls within targetRect
	if (

		( cuttingRect->right >= targetRect.left ) &&
		( cuttingRect->right <= targetRect.right )
	)
	{
		// Slice off portion of targetRect that falls to the right of the edge
		rect = rect_new(

			targetRect.top,
			cuttingRect->right + 1,  // why +1?
			targetRect.bottom,
			targetRect.right
		);

		if ( rect == NULL )
		{
			list_freeNodes( outputRects );

			free( outputRects );

			return NULL;
		}


		// Add the new rectangle to the output list
		list_appendNode( outputRects, rect );

		// Shrink targetRect to exclude the sliced portion
		targetRect.right = cuttingRect->right;
	}


	// Split by bottom edge - - - - - - - - - - - - - - - - - - - - - - -

	/*
		                                
		                                
		                                
		          @@@@@@@@@@@@@@        
		          @@@@@@@@@@@@@@        
		          @@@@@@@@@@@@@@        
		          @@@@@@@@@@@@@@        
		          @@@@@@@@@@@@@@        
		          @@@@@@@@@@@@@@        
		---------------------------------
		          OOOOOOOOOOOOOO        
		          OOOOOOOOOOOOOO        
		          OOOOOOOOOOOOOO        
	*/

	// If cuttingRect's bottom edge falls within targetRect
	if (

		( cuttingRect->bottom >= targetRect.top ) &&
		( cuttingRect->bottom <= targetRect.bottom )
	)
	{
		// Slice off portion of targetRect that falls to the bottom of the edge
		rect = rect_new(

			cuttingRect->bottom + 1,  // why +1?
			targetRect.left,
			targetRect.bottom,
			targetRect.right
		);

		if ( rect == NULL )
		{
			list_freeNodes( outputRects );

			free( outputRects );

			return NULL;
		}


		// Add the new rectangle to the output list
		list_appendNode( outputRects, rect );

		// Shrink targetRect to exclude the sliced portion
		targetRect.bottom = cuttingRect->bottom;
	}


	// Done - - - - - - - - - - - - - - - - - - - - - - - - - -
	return outputRects;
}




/* ==========================================================================================
   ...
   ========================================================================================== */

struct _Context* context_new ( int width, int height )
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

	context->clipRects = list_new();

	if ( ( context->frameBuffer == NULL ) || ( context->clipRects == NULL ) )
	{
		free( context );

		return NULL;
	}

	return context;
}


// ------------------------------------------------------------------------------------------

// void context_subtractClipRect ( struct _Context* context, struct _Rect* newRect )
void context_subtractClipRect ( struct _Context* context, struct _Rect* newRect, uint32_t debugColor )
{
	struct _Rect*     rect;
	struct _List*     visibleSlices;
	struct _ListNode* node;
	struct _ListNode* node2;
	int               i;


	/* Check each item in the list to see if it is occluded
	   by the new rect
	*/
	i    = 0;
	node = context->clipRects->rootNode;

	while ( node != NULL )
	{
		rect = ( struct _Rect* ) node->value;


		// If the rects don't overlap, evaluate next
		if ( rect_rectsIntersect( rect, newRect ) == 0 )
		{
			node  = node->next;
			i    += 1;

			continue;
		}


		/* If the existing rect is occluded by the new rect,
		   replace the existing rect with slices that correspond
		   to its visible part
		*/
		visibleSlices = rect_split( rect, newRect );

		free( rect );


		// Remove existing rect from the list
		node = node->next;  /* retrive next node pointer before we delete current node.
		                       Since deleting a node, no need to increment 'i' */

		list_removeNode( context->clipRects, i );


		// And replace it with its visible slices
		node2 = visibleSlices->rootNode;

		while ( node2 != NULL )
		{
			rect = ( struct _Rect* ) node2->value;

			list_appendNode( context->clipRects, rect );

			// Debug - draw the intermediate clipped rects
			if ( DEBUG_ADD_CLIP_RECT )
			{
				context_strokeRect(

					context,
					rect->left,
					rect->top,
					rect->right - rect->left + 1,
					rect->bottom - rect->top + 1,
					debugColor,
					1
				);
			}

			node2 = node2->next;
		}

		list_freeNodes( visibleSlices );

		free( visibleSlices );
	}
}

void context_addClipRect ( struct _Context* context, struct _Rect* newRect, uint32_t debugColor )
{
	/* If the existing rectangles are occluded by the new rect,
	   replace them with slices that correspond to their visible parts
	*/
	context_subtractClipRect( context, newRect, debugColor );

	/* At this point we have ensured that none of the exisitng rectangles
	   overlap with the new one. As such, it can finally be added to the list
	*/
	list_appendNode( context->clipRects, newRect );

	// Debug - draw the intermediate clipped rects
	if ( DEBUG_ADD_CLIP_RECT )
	{
		context_strokeRect(

			context,
			newRect->left,
			newRect->top,
			newRect->right - newRect->left + 1,
			newRect->bottom - newRect->top + 1,
			debugColor,
			1
		);
	}	
}

void context_clearClipRects ( struct _Context* context )
{
	struct _ListNode* node;
	struct _Rect*     rect;

	// Free rects
	node = context->clipRects->rootNode;

	while ( node != NULL )
	{
		rect = ( struct _Rect* ) node->value;

		free( rect );

		node = node->next;
	}


	// Free list nodes
	list_freeNodes( context->clipRects );
}


// ------------------------------------------------------------------------------------------

void context_lineHorizontal (

	struct _Context* context,
	int              x,
	int              y,
	int              width,
	uint32_t         color,
	int              strokeWeight
)
{
	context_fillRect( context, x, y, width, strokeWeight, color );
}

void context_lineVertical (

	struct _Context* context,
	int              x,
	int              y,
	int              height,
	uint32_t         color,
	int              strokeWeight
)
{
	context_fillRect( context, x, y, strokeWeight, height, color );
}

void context_strokeRect (

	struct _Context* context,
	int              x,
	int              y,
	int              width,
	int              height,
	uint32_t         color,
	int              strokeWeight
)
{
	/* Assumes border size is included in width.
	   Similar to CSS "box-sizing" property. Eg:

	       TTTR
	       L..R  where width x height specified as 4x4
	       L..R
	       LBBB
	*/

	context_lineHorizontal(  // top

		context,
		x,
		y,
		width - strokeWeight,
		color,
		strokeWeight
	);
	context_lineVertical(  // right

		context,
		x + width - strokeWeight,
		y,
		height - strokeWeight,
		color,
		strokeWeight
	);
	context_lineHorizontal(  // bottom

		context,
		x + strokeWeight,
		y + height - strokeWeight,
		width - strokeWeight,
		color,
		strokeWeight
	);
	context_lineVertical(  // left

		context,
		x,
		y + strokeWeight,
		height - strokeWeight,
		color,
		strokeWeight
	);
}

// Draw only the portion of the rect that is within 'boundary'
void context__boundedFillRect (

	struct _Context* context,
	int              x,
	int              y,
	int              width,
	int              height,
	struct _Rect*    boundary,
	uint32_t         color
)
{
	int curX;
	int maxX;
	int maxY;
	int idx;

	maxX = x + width;
	maxY = y + height;

	// Constrain to bounds of context (screen)
	if ( maxX > context->width )
	{
		maxX = context->width;
	}

	if ( maxY > context->height )
	{
		maxY = context->height;
	}

	if ( x < 0 )
	{
		x = 0;
	}

	if ( y < 0 )
	{
		y = 0;
	}

	// Constrain to bounds of clipRect
	if ( boundary != NULL )
	{
		if ( maxX > ( boundary->right + 1 ) )
		{
			maxX = boundary->right + 1;
		}

		if ( maxY > boundary->bottom + 1 )
		{
			maxY = boundary->bottom + 1;
		}

		if ( x < boundary->left )
		{
			x = boundary->left;
		}

		if ( y < boundary->top )
		{
			y = boundary->top;
		}
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

/* Since all our drawing primitives ultimately call this function,
   they end up being appropriately clipped
*/
void context_fillRect (

	struct _Context* context,
	int              x,
	int              y,
	int              width,
	int              height,
	uint32_t         color
)
{
	struct _ListNode* node;
	struct _Rect*     clipRect;

	// If there are clipping rects, draw the rect clipped to each? of them
	if ( context->clipRects->nItems > 0 )
	{
		node = context->clipRects->rootNode;

		while ( node != NULL )
		{
			clipRect = ( struct _Rect* ) node->value;

			context__boundedFillRect( context, x, y, width, height, clipRect, color );

			node = node->next;
		}
	}

	// Otherwise, draw the rect unclipped (clipped to the screen)
	else
	{
		context__boundedFillRect( context, x, y, width, height, NULL, color );
	}
}

/* TODO: Does not clip
*/
void context_setPixel (

	struct _Context* context,
	int              x,
	int              y,
	uint32_t         color
)
{
	int idx;

	// Check bounds of context (screen)
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




/* ==========================================================================================
   ...
   ========================================================================================== */

struct _Window* window_new (

	int              x,
	int              y,
	int              width,
	int              height,
	uint32_t         debugColor,
	struct _Context* context
)
{
	struct _Window* window;

	window = ( struct _Window* ) malloc( sizeof( struct _Window ) );

	if ( window == NULL )
	{
		return NULL;
	}

	window->x          = x;
	window->y          = y;
	window->width      = width;
	window->height     = height;
	window->debugColor = debugColor;
	window->context    = context;

	return window;
}

void window_paint ( struct _Window* window )
{
	// Debug
	/*context_fillRect(

		window->context,
		window->x,
		window->y,
		window->width,
		window->height,
		window->debugColor
	);*/

	// Fill in the titlebar background
	context_fillRect(

		window->context,
		window->x,
		window->y,
		window->width,
		TITLE_BAR_HEIGHT,
		WIN_TITLECOLOR
	);

	// Fill in the window background
	context_fillRect(

		window->context,
		window->x,
		window->y + TITLE_BAR_HEIGHT,
		window->width,
		window->height - TITLE_BAR_HEIGHT,
		WIN_BGCOLOR
	);

	// Draw a 3px border around the window 
	context_strokeRect(

		window->context,
		window->x,
		window->y,
		window->width,
		window->height,
		WIN_BORDERCOLOR,
		3
	);

	// Draw a 3px border line under the titlebar
	context_lineHorizontal(

		window->context,
		window->x,
		window->y + TITLE_BAR_HEIGHT - 3,
		window->width,
		WIN_BORDERCOLOR,
		3
	);
}




/* ==========================================================================================
   ...
   ========================================================================================== */

void cursor_paint ( struct _Context* context, int x, int y )
{
	context_setPixel( context, x,     y,     0xFFFFFFFF );
	context_setPixel( context, x - 1, y,     0xFF0000FF );
	context_setPixel( context, x + 1, y,     0xFF0000FF );
	context_setPixel( context, x,     y + 1, 0xFF0000FF );
	context_setPixel( context, x,     y - 1, 0xFF0000FF );
}




/* ==========================================================================================
   ...
   ========================================================================================== */

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


// ------------------------------------------------------------------------------------------

void desktop_paint ( struct _Desktop* desktop )
{
	struct _ListNode* node;
	struct _ListNode* node2;
	struct _ListNode* node3;
	struct _Window*   window;
	struct _Window*   clipWindow;
	struct _Rect      winRect;
	struct _Rect*     rect;
	struct _Rect*     windowRect;
	struct _Rect*     desktopRect;
	struct _List*     clipWindows;

	/* Do the clipping for the desktop
	*/
	{
		context_clearClipRects( desktop->context );


		// Debug - clear desktop (without clipping)
		if ( DEBUG_DESKTOP_CLIP )
		{
			context_fillRect(

				desktop->context,
				0,
				0,
				desktop->context->width,
				desktop->context->height,
				0x00303BFF
				// 0x000000FF
			);
		}


		// Create and add a base rectangle for the desktop
		desktopRect = rect_new(

			0,
			0,
			desktop->context->height - 1,
			desktop->context->width - 1
		);

		if ( desktopRect == NULL )
		{
			return;
		}

		context_addClipRect( desktop->context, desktopRect, 0xFFFFFFFF );


		// Subtract each of the windows from the desktop
		node = desktop->children->rootNode;

		while ( node != NULL )
		{
			window = ( struct _Window* ) node->value;

			//
			winRect.top    = window->y;
			winRect.left   = window->x;
			winRect.bottom = window->y + window->height - 1;
			winRect.right  = window->x + window->width - 1;

			context_subtractClipRect( desktop->context, &winRect, window->debugColor );

			//
			node = node->next;
		}


		// Clear desktop (with clipping)
		context_fillRect(

			desktop->context,
			0,
			0,
			desktop->context->width,
			desktop->context->height,
			// 0x00303BFF
			0x000000FF
		);


		// Debug - draw the final clipped rects
		if ( DEBUG_DESKTOP_CLIP && DEBUG_FINAL_CLIP_RECTS )
		{
			node3 = desktop->context->clipRects->rootNode;

			while ( node3 != NULL )
			{
				rect = ( struct _Rect* ) node3->value;

				context_strokeRect(

					desktop->context,
					rect->left,
					rect->top,
					rect->right - rect->left + 1,
					rect->bottom - rect->top + 1,
					0xFFFF00FF,
					1
				);

				node3 = node3->next;
			}
		}
	}


	/* Do the clipping for each window
	*/
	{
		context_clearClipRects( desktop->context );


		//
		node = desktop->children->rootNode;

		while ( node != NULL )
		{
			window = ( struct _Window* ) node->value;


			// Debug - draw the window (without clipping)
			if ( DEBUG_WINDOW_CLIP )
			{
				context_fillRect(

					window->context,
					window->x,
					window->y,
					window->width,
					window->height,
					// window->debugColor
					0xFFCE96FF
				);
			}


			// Create and add a base rectangle for the current window
			windowRect = rect_new(

				window->y,
				window->x,
				window->y + window->height - 1,
				window->x + window->width - 1
			);

			if ( windowRect == NULL )
			{
				return;
			}

			context_addClipRect( desktop->context, windowRect, window->debugColor );


			// Get windows above and overlapping
			clipWindows = desktop_getWindowsAbove( desktop, window );


			// Subtract each of the clipWindows from the window
			node2 = clipWindows->rootNode;

			while ( node2 != NULL )
			{
				clipWindow = ( struct _Window* ) node2->value;

				//
				winRect.top    = clipWindow->y;
				winRect.left   = clipWindow->x;
				winRect.bottom = clipWindow->y + clipWindow->height - 1;
				winRect.right  = clipWindow->x + clipWindow->width - 1;

				context_subtractClipRect( desktop->context, &winRect, clipWindow->debugColor );

				//
				node2 = node2->next;
			}


			//
			list_freeNodes( clipWindows );
			free( clipWindows );


			// Draw the window (with clipping)
			window_paint( window );


			// Debug - draw the final clipped rects
			if ( DEBUG_WINDOW_CLIP && DEBUG_FINAL_CLIP_RECTS )
			{
				node3 = desktop->context->clipRects->rootNode;

				while ( node3 != NULL )
				{
					rect = ( struct _Rect* ) node3->value;

					context_strokeRect(

						desktop->context,
						rect->left,
						rect->top,
						rect->right - rect->left + 1,
						rect->bottom - rect->top + 1,
						0x308ACEFF,
						2
					);

					node3 = node3->next;
				}

				// Debug only visible for one window...
				break;
			}


			//
			node = node->next;
		}
	}


	// Draw mouse
	cursor_paint( desktop->context, desktop->mouseX, desktop->mouseY );
}


// ------------------------------------------------------------------------------------------

struct _Window* desktop_createWindow (

	struct _Desktop* desktop,
	int              x,
	int              y,
	int              width,
	int              height,
	uint32_t         debugColor
)
{
	struct _Window* window;
	int             rStatus;

	window = window_new( x, y, width, height, debugColor, desktop->context );

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


void desktop_raiseWindow ( struct _Desktop* desktop )
{
	struct _ListNode* child;
	struct _Window*   window;
	int               nChildren;
	int               i;

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

			( desktop->mouseX >= window->x )                      &&
			( desktop->mouseX <  ( window->x + window->width ) )  &&
			( desktop->mouseY >= window->y )                      &&
			( desktop->mouseY <  ( window->y + window->height ) )
		)
		{
			// Raise window to top of list
			list_removeNode( desktop->children, i );

			list_appendNode( desktop->children, ( void* ) window );  // new tail


			/* Windows are draggable by their titlebars.
			   Check if the mouse is inside the titlebar
			*/
			if ( desktop->mouseY < ( window->y + TITLE_BAR_HEIGHT ) )
			{
				// Update drag target and offset...
				desktop->dragTarget  = window;
				desktop->dragOffsetX = desktop->mouseX - window->x;
				desktop->dragOffsetY = desktop->mouseY - window->y;
			}


			// Done
			break;
		}


		child = child->prev;
		i -= 1;
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

struct _List* desktop_getWindowsAbove ( struct _Desktop* desktop, struct _Window* btmWindow )
{
	struct _List*     topWindows;
	struct _Window*   curWindow;
	struct _ListNode* node;
	int               foundBtmWindow;
	struct _Rect      btmWindowRect;
	struct _Rect      curWindowRect;

	//
	topWindows = list_new();

	if ( topWindows == NULL )
	{
		return NULL;
	}	

	//
	btmWindowRect.top    = btmWindow->y;
	btmWindowRect.left   = btmWindow->x;
	btmWindowRect.bottom = btmWindow->y + btmWindow->height - 1;
	btmWindowRect.right  = btmWindow->x + btmWindow->width - 1;


	//
	foundBtmWindow = 0;

	node = desktop->children->rootNode;

	while ( node != NULL )
	{
		curWindow = ( struct _Window* ) node->value;

		// Find 'btmWindow' in desktop->children
		if ( curWindow == btmWindow )
		{
			foundBtmWindow = 1;
		}

		/* Add windows to 'topWindows' that
		    1) have a higher index than 'btmWindow' (tail is topmost)
		    2) overlap with 'btmWindow'
		*/
		else if ( foundBtmWindow )  // 1)
		{
			curWindowRect.top    = curWindow->y;
			curWindowRect.left   = curWindow->x;
			curWindowRect.bottom = curWindow->y + curWindow->height - 1;
			curWindowRect.right  = curWindow->x + curWindow->width - 1;

			if ( rect_rectsIntersect( &curWindowRect, &btmWindowRect ) )  // 2)
			{
				list_appendNode( topWindows, curWindow );
			}
		}

		//
		node = node->next;
	}

	return topWindows;
}


// ------------------------------------------------------------------------------------------

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



/* ==========================================================================================
   ...
   ========================================================================================== */

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





/* =~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~
   olcPixelGameEngine
   =~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~ */

void olcGlue_renderContextBuffer ( struct _Context* context )
{
	int          i;
	int          x;
	int          y;
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


// ------------------------------------------------------------------------------------------

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

	// return false;
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
