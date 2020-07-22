// http://www.trackze.ro/wsbe-complicated-rectangles/

#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "olcPGE_min.h"
#include "params.h"
#include "tut.h"


/* To visualize debug properly:
    1) Comment out updates in 'tut_main' loop
    2) Change arg of 'window_paint' in 'tut_init' to desired window
    3) Comment out 'window->paintHandler( window )' in 'window_paint'
*/
#define DEBUG_WINDOW_CLIP 1

struct _Desktop* gDesktop;  // hmmm...
struct _Context* gContext;  // hmmm...
int16_t          gMouseX;
int16_t          gMouseY;
uint8_t          gLeftBtnState;

struct _Window* win0;
struct _Window* win1;
struct _Window* win2;
struct _Window* win3;




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

void list_free ( struct _List* list )
{
	list_freeNodes( list );

	free( list );
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

int rect_getWidth ( struct _Rect* rect )
{
	return rect->right - rect->left + 1;
}

int rect_getHeight ( struct _Rect* rect )
{
	return rect->bottom - rect->top + 1;
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

	// If the rectangles don't overlap, nothing to do
	if ( rect_rectsIntersect( _targetRect, cuttingRect ) == 0 )
	{
		return NULL;
	}


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
			list_free( outputRects );

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
			list_free( outputRects );

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
			list_free( outputRects );

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


// ------------------------------------------------------------------------------------------

/* Similar to rect_split, but this time we output area hidden
   when occluded by cuttingRect.

    ___a
   |  _|_b      _c
   |_|_| | --> |_| 
     |___|

*/
struct _Rect* rect_getIntersection ( struct _Rect* targetRect, struct _Rect* cuttingRect )
{
	struct _Rect* outputRect;

	// If the rectangles don't overlap, nothing to do
	if ( rect_rectsIntersect( targetRect, cuttingRect ) == 0 )
	{
		return NULL;
	}


	// outputRect starts as a copy of targetRect
	outputRect = rect_new(

		targetRect->top,
		targetRect->left,
		targetRect->bottom,
		targetRect->right
	);

	if ( outputRect == NULL )
	{
		return NULL;
	}


	// Shrink to right-most left-edge
	if (

		( cuttingRect->left >= targetRect->left ) &&
		( cuttingRect->left <= targetRect->right )
	)
	{
		outputRect->left = cuttingRect->left;
	}

	// Shrink to bottom-most top-edge
	if (

		( cuttingRect->top >= targetRect->top ) &&
		( cuttingRect->top <= targetRect->bottom )
	)
	{
		outputRect->top = cuttingRect->top;
	}

	// Shrink to left-most right-edge
	if (

		( cuttingRect->right >= targetRect->left ) &&
		( cuttingRect->right <= targetRect->right )
	)
	{
		outputRect->right = cuttingRect->right;
	}

	// Shrink to top-most bottom-edge
	if (

		( cuttingRect->bottom >= targetRect->top ) &&
		( cuttingRect->bottom <= targetRect->bottom )
	)
	{
		outputRect->bottom = cuttingRect->bottom;
	}


	//
	return outputRect;
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

	context->xOffset = 0;
	context->yOffset = 0;

	context->clipRects = list_new();

	if ( ( context->frameBuffer == NULL ) || ( context->clipRects == NULL ) )
	{
		free( context );

		return NULL;
	}

	return context;
}


// ------------------------------------------------------------------------------------------

/* Update the clipping rectangles to exclude the area inside subRect
*/
void context_subtractClipRect ( struct _Context* context, struct _Rect* subRect )
{
	struct _Rect*     oldClipRect;
	struct _Rect*     newClipRect;
	struct _List*     visibleSlices;
	struct _ListNode* node;
	struct _ListNode* node2;
	int               i;
	int               nInitialRects;  


	/* Check each rect in the "original" list to see if it is occluded
	   by subRect.
	   New rects appended to the list will (by design) not be occluded.
	*/
	i             = 0;
	node          = context->clipRects->rootNode;
	nInitialRects = context->clipRects->nItems;

	while ( nInitialRects > 0 )
	// while ( node != NULL )
	{
		oldClipRect = ( struct _Rect* ) node->value;


		/* If the existing rect is occluded by subRect,
		   replace the existing rect with slices that correspond
		   to its visible part
		*/
		visibleSlices = rect_split( oldClipRect, subRect );

		if ( visibleSlices != NULL )
		{
			// Remove existing rect from the list
			node = node->next;  /* retrive next node pointer before we delete current node.
			                       Since deleting a node, no need to increment 'i' */

			list_removeNode( context->clipRects, i );

			free( oldClipRect );


			// Replace it with its visible slices
			node2 = visibleSlices->rootNode;

			while ( node2 != NULL )
			{
				newClipRect = ( struct _Rect* ) node2->value;

				list_appendNode( context->clipRects, newClipRect );

				node2 = node2->next;
			}

			list_free( visibleSlices );
		}

		else
		{
			node = node->next;
			i += 1;
		}

		nInitialRects -= 1;
	}
}

/* Update the clipping rectangles to include the area inside newRect
*/
void context_addClipRect ( struct _Context* context, struct _Rect* newRect )
{
	/* If the existing rectangles are occluded by the new rect,
	   replace them with slices that correspond to their visible parts
	*/
	context_subtractClipRect( context, newRect );

	/* At this point we have ensured that none of the exisitng rectangles
	   overlap with the new one. As such, it can finally be added to the list
	*/
	list_appendNode( context->clipRects, newRect );
}

/* Update the clipping rectangles to include ONLY the areas inside BOTH
   the existing clipping region and newRect
*/
void context_intersectClipRect ( struct _Context* context, struct _Rect* newRect )
{
	struct _List*     unionRects;
	struct _Rect*     unionRect;
	struct _Rect*     oldClipRect;
	struct _ListNode* node;

	// Allocate space
	unionRects = list_new();

	if ( unionRects == NULL )
	{
		return;
	}

	// For each existing clipRect, get the intersection rect
	node = context->clipRects->rootNode;

	while ( node != NULL )
	{
		oldClipRect = ( struct _Rect* ) node->value;

		unionRect = rect_getIntersection( oldClipRect, newRect );

		if ( unionRect != NULL )
		{
			list_appendNode( unionRects, unionRect );
		}

		node = node->next;
	}

	// Free the original list, and replace
	list_free( context->clipRects );

	context->clipRects = unionRects;
}


// ------------------------------------------------------------------------------------------

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

	// Translate the rectangle's coordinates by the context's translation
	x += context->xOffset;
	y += context->yOffset;


	// Constrain bounds
	{
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
   they end up being appropriately clipped (and translated)
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
	uint16_t         flags,
	struct _Context* context
)
{
	struct _Window* window;
	int             status;

	// Allocate space for the window
	window = ( struct _Window* ) malloc( sizeof( struct _Window ) );

	if ( window == NULL )
	{
		return NULL;
	}

	// Initialize the window
	status = window_init( window, x, y, width, height, flags, context );

	if ( status == 0 )
	{
		free( window );

		return NULL;
	}

	//
	return window;
}

int window_init (

	struct _Window*  window,
	int              x,
	int              y,
	int              width,
	int              height,
	uint16_t         flags,
	struct _Context* context
)
{
	// Initialize fields
	window->x       = x;
	window->y       = y;
	window->width   = width;
	window->height  = height;
	window->flags   = flags;
	window->context = context;

	window->children = list_new();

	if ( window->children == NULL )
	{
		return 0;
	}

	window->parent           = NULL;
	window->prevLeftBtnState = 0;
	window->dragTarget       = NULL;
	window->dragOffsetX      = 0;
	window->dragOffsetY      = 0;
	window->paintHandler     = window_basePaintHandler;

	return 1;
}


// ------------------------------------------------------------------------------------------

int window_getAbsoluteXPosition ( struct _Window* window )
{
	if ( window->parent )
	{
		return window->x + window_getAbsoluteXPosition( window->parent );
	}
	else
	{
		return window->x;
	}
}

int window_getAbsoluteYPosition ( struct _Window* window )
{
	if ( window->parent )
	{
		return window->y + window_getAbsoluteYPosition( window->parent );
	}
	else
	{
		return window->y;
	}
}


// ------------------------------------------------------------------------------------------

struct _Window* window_createChildWindow (

	struct _Window* window,
	int             x,
	int             y,
	int             width,
	int             height,
	uint16_t        flags
)
{
	struct _Window* childWindow;
	int             status;

	//
	childWindow = window_new( x, y, width, height, flags, window->context );

	if ( childWindow == NULL )
	{
		return NULL;
	}

	//
	status = list_appendNode( window->children, ( void* ) childWindow );

	if ( status == 0 )
	{
		free( childWindow );

		return NULL;
	}

	//
	childWindow->parent = window;

	//
	return childWindow;
}

void window_appendChildWindow ( struct _Window* window, struct _Window* childWindow )
{
	int status;

	//
	status = list_appendNode( window->children, ( void* ) childWindow );

	if ( status == 0 )
	{
		return;
	}

	//
	childWindow->parent  = window;
	childWindow->context = window->context;
}


// ------------------------------------------------------------------------------------------

struct _List* window_getChildWindowsAbove ( struct _Window* window, struct _Window* btmWindow )
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

	node = window->children->rootNode;

	while ( node != NULL )
	{
		curWindow = ( struct _Window* ) node->value;

		// Find 'btmWindow' in window->children
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

void window_raiseChildWindow ( struct _Window* window, int16_t mouseX, int16_t mouseY )
{
	struct _ListNode* node;
	struct _Window*   childWindow;
	int               nChildren;
	int               i;

	nChildren = window->children->nItems;

	if ( nChildren == 0 )
	{
		return;
	}
	else if ( nChildren == 1 )
	{
		node = window->children->rootNode;
	}
	else
	{
		node = window->children->tailNode;
	}


	i = nChildren - 1;

	// Go through list backwards, because tail is topmost (drawn last)
	while ( node != NULL )
	{
		childWindow = ( struct _Window* ) node->value;

		// Mouse inside childWindow
		if (

			( mouseX >= childWindow->x )                           &&
			( mouseX <  ( childWindow->x + childWindow->width ) )  &&
			( mouseY >= childWindow->y )                           &&
			( mouseY <  ( childWindow->y + childWindow->height ) )
		)
		{
			// Raise childWindow to top of list
			list_removeNode( window->children, i );

			list_appendNode( window->children, ( void* ) childWindow );  // new tail


			/* Windows are draggable by their titlebars.
			   Check if the mouse is inside the titlebar
			*/
			if ( mouseY < ( childWindow->y + WIN_TITLEBAR_HEIGHT ) )
			{
				// Update drag target and offset...
				window->dragTarget  = childWindow;
				window->dragOffsetX = mouseX - childWindow->x;
				window->dragOffsetY = mouseY - childWindow->y;
			}


			// Done
			break;
		}

		//
		node = node->prev;
		i -= 1;
	}
}

void window_dragChildWindow ( struct _Window* window, int16_t mouseX, int16_t mouseY )
{
	/* Applying the offset makes sure that the corner of the
	   window does not suddenly snap to the mouse location
	*/
	window->dragTarget->x = mouseX - window->dragOffsetX;
	window->dragTarget->y = mouseY - window->dragOffsetY;
}


// ------------------------------------------------------------------------------------------

int getWinName ( struct _Window* window )  // temporary debug functions
{
	if      ( window == win0 ) { return 0; }
	else if ( window == win1 ) { return 1; }
	else if ( window == win2 ) { return 2; }
	else if ( window == win3 ) { return 3; }
	else                       { return 99; }  // desktop
}

uint32_t getDebugColor ( struct _Window* window )  // temporary debug functions
{
	if      ( window == win0 ) { return 0x081820FF; }
	else if ( window == win1 ) { return 0x346856FF; }
	else if ( window == win2 ) { return 0x88C070FF; }
	else if ( window == win3 ) { return 0xE0F8D0FF; }
	else if ( window == ( struct _Window* ) gDesktop ) { return 0x764462FF; }
	else                       { return 0x00FF00FF; }
}

/*
TODO: Don't quite understand tree traversal of this function.

Let's follow path of a call to 'window_paint( win0 )':

    window_paint win0
    ├── window_getClippingRegion win0
    │   ├── window_getClippingRegion win99
    │   │   └── addClipRect desktop
    │   ├── intersectClipRect ancestors of win0
    │   ├── subtractClipRect sibling win1
    │   └── subtractClipRect sibling win2
    ├── intersectClipRect decoration win0
    ├── subtractClipRect child win3
    │
    └── window_paint win3
        ├── window_getClippingRegion win3
        │   ├── window_getClippingRegion win0
        │   │   ├── window_getClippingRegion win99
        │   │   │   └── addClipRect desktop
        │   │   ├── intersectClipRect ancestors of win0
        │   │   ├── subtractClipRect sibling win1
        │   │   └── subtractClipRect sibling win2
        │   └── intersectClipRect ancestors of win3
        └── intersectClipRect decoration win3
*/

void window_getClippingRegion ( struct _Window* window, int inRecursion )
{
	int               windowX;
	int               windowY;
	struct _ListNode* node;
	struct _Rect*     rect;
	struct _Rect      rect2;
	struct _List*     siblingWindows;
	struct _Window*   siblingWindow;
	int               siblingWindowX;
	int               siblingWindowY;


	if ( DEBUG_WINDOW_CLIP )
	{
		printf( "window_getClippingRegion win%d\n", getWinName( window ) );
	}


	// Create a rectangle representing the window
	windowX = window_getAbsoluteXPosition( window );
	windowY = window_getAbsoluteYPosition( window );

	/* If the window uses a window decoration and we are recursing,
	   limit the clipping area to the area inside the decoration,
	   so that child windows don't get drawn over the decoration.
	*/
	if ( inRecursion && ( ( window->flags & WIN_FLAG_NO_DECORATION ) == 0 ) )
	{
		windowX += WIN_BORDER_WIDTH;     // exclude border
		windowY += WIN_TITLEBAR_HEIGHT;  // exclude title bar

		rect = rect_new(

			windowY,
			windowX,
			windowY + window->height - WIN_TITLEBAR_HEIGHT - WIN_BORDER_WIDTH - 1,
			windowX + window->width - ( 2 * WIN_BORDER_WIDTH ) - 1
		);
	}
	/* If the window uses a window decoration, the entire area
	   of the window is drawable.

	   If we are not recursing, then it means we are about to
	   paint a window when we return from this function. In this case,
	   we want to leave the decoration area unclipped so that
	   we can paint the decoration...
	*/
	else
	{
		rect = rect_new(

			windowY,
			windowX,
			windowY + window->height - 1,
			windowX + window->width - 1
		);
	}


	/* Add "full" window to clipping-region.
	   This is where our recursion finally halts.
	*/
	if ( window->parent == NULL )
	{
		context_addClipRect( window->context, rect );

		if ( DEBUG_WINDOW_CLIP )
		{
			printf( "addClipRect desktop\n" );
		}

		return;
	}


	// Get ancestors clipping-region?
	window_getClippingRegion( window->parent, 1 );

	/* Set the clipping-region as the union of the
	   ancestors' clipping-region and the window...
	   i.e. the part of the window that is not occluded by its ancestors...
	*/
	context_intersectClipRect( window->context, rect );

	if ( DEBUG_WINDOW_CLIP )
	{
		printf( "intersectClipRect ancestors of win%d\n", getWinName( window ) );
	}

	free( rect );


	/* Remove portions of the window which are hidden/occluded
	   by its siblings from the clipping-region
	*/
	siblingWindows = window_getChildWindowsAbove( window->parent, window );

	node = siblingWindows->rootNode;

	while ( node != NULL )
	{
		siblingWindow = ( struct _Window* ) node->value;

		//
		siblingWindowX = window_getAbsoluteXPosition( siblingWindow );
		siblingWindowY = window_getAbsoluteYPosition( siblingWindow );

		rect2.top    = siblingWindowY;
		rect2.left   = siblingWindowX;
		rect2.bottom = siblingWindowY + siblingWindow->height - 1;
		rect2.right  = siblingWindowX + siblingWindow->width - 1;

		context_subtractClipRect( window->context, &rect2 );

		//
		if ( DEBUG_WINDOW_CLIP )
		{
			printf( "subtractClipRect sibling win%d\n", getWinName( siblingWindow ) );
		}

		//
		node = node->next;
	}

	list_free( siblingWindows );
}

void window_paint ( struct _Window* window )
{
	/* Setup is the same for all windows.
	   Once finished, we call the window's unique paintHandler.
	*/
	struct _ListNode* node;
	int               windowX;
	int               windowY;
	int               childWindowX;
	int               childWindowY;
	struct _Rect      rect;
	struct _Window*   childWindow;


	if ( DEBUG_WINDOW_CLIP )
	{
		printf( "\nwindow_paint win%d\n", getWinName( window ) );
	}


	// Reset
	context_clearClipRects( window->context );
	window->context->xOffset = 0;
	window->context->yOffset = 0;


	//
	windowX = window_getAbsoluteXPosition( window );
	windowY = window_getAbsoluteYPosition( window );


	/* Create a clipping region representing the (visible) parts
	   of the window which are not hidden/occluded by its ancestors
	   or siblings
	*/
	window_getClippingRegion( window, 0 );


	/* If the window uses a window decoration, paint it.
	   Then, remove the decoration from the clipping-region.
	*/
	if ( ( window->flags & WIN_FLAG_NO_DECORATION ) == 0 )
	{
		// Draw the window decoration
		window_paintDecoration( window );


		/* Limit client drawable area
		
		   ooooooooooo <-
		   o+++++++++o   | titlebar
		   ooooooooooo <-
		   o.........o <-
		   o.........o   | drawable
		   o.........o   | area
		   o.........o <-
		   ooooooooooo
		*/
		windowX += WIN_BORDER_WIDTH;     // exclude border
		windowY += WIN_TITLEBAR_HEIGHT;  // exclude title bar

		rect.top    = windowY;
		rect.left   = windowX;
		rect.bottom = windowY + window->height - WIN_TITLEBAR_HEIGHT - WIN_BORDER_WIDTH - 1;
		rect.right  = windowX + window->width - ( 2 * WIN_BORDER_WIDTH ) - 1;

		context_intersectClipRect( window->context, &rect );


		if ( DEBUG_WINDOW_CLIP )
		{
			printf( "intersectClipRect decoration win%d\n", getWinName( window ) );
		}
	}


	/* Remove portions of the window which are hidden/occluded
	   by its children from the clipping-region
	*/
	node = window->children->rootNode;

	while ( node != NULL )
	{
		childWindow = ( struct _Window* ) node->value;

		//
		childWindowX = window_getAbsoluteXPosition( childWindow );
		childWindowY = window_getAbsoluteYPosition( childWindow );

		rect.top    = childWindowY;
		rect.left   = childWindowX;
		rect.bottom = childWindowY + childWindow->height - 1;
		rect.right  = childWindowX + childWindow->width - 1;

		context_subtractClipRect( window->context, &rect );

		//
		if ( DEBUG_WINDOW_CLIP )
		{
			printf( "subtractClipRect child win%d\n", getWinName( childWindow ) );
		}

		//
		node = node->next;
	}


	if ( DEBUG_WINDOW_CLIP )
	{
		debug_drawClipRects( window->context, 0xFFFF00FF, 6, getDebugColor( window ) );
	}


	/* paintHandler uses relative positions.
	   We thus translate the context to accomodate.
	*/
	window->context->xOffset = windowX;
	window->context->yOffset = windowY;
//	window->paintHandler( window );


	// Paint children
	node = window->children->rootNode;

	while ( node != NULL )
	{
		childWindow = ( struct _Window* ) node->value;

		window_paint( childWindow );

		node = node->next;
	}
}


// ------------------------------------------------------------------------------------------

void window_basePaintHandler ( struct _Window* window )  // uses relative positions
{
	// Fill in the drawable area
	context_fillRect(

		window->context,
		0,
		0,
		window->width,
		window->height,
		WIN_BACKGROUND_COLOR
	);
}

void window_paintDecoration ( struct _Window* window )  // uses absolute positions
{
	int windowX;
	int windowY;

	//
	windowX = window_getAbsoluteXPosition( window );
	windowY = window_getAbsoluteYPosition( window );


	// Debug
	/*context_fillRect(

		window->context,
		windowX,
		windowY,
		window->width,
		window->height,
		window->debugColor
	);*/

	// Fill in the titlebar background
	context_fillRect(

		window->context,
		windowX,
		windowY,
		window->width,
		WIN_TITLEBAR_HEIGHT,
		WIN_TITLEBAR_COLOR
	);

	// Fill in the window background
	/*context_fillRect(

		window->context,
		windowX,
		windowY + WIN_TITLEBAR_HEIGHT,
		window->width,
		window->height - WIN_TITLEBAR_HEIGHT,
		WIN_BACKGROUND_COLOR
	);*/

	// Draw a border around the window 
	context_strokeRect(

		window->context,
		windowX,
		windowY,
		window->width,
		window->height,
		WIN_BORDER_COLOR,
		WIN_BORDER_WIDTH
	);

	// Draw a line under the titlebar
	context_lineHorizontal(

		window->context,
		windowX,
		windowY + WIN_TITLEBAR_HEIGHT - WIN_BORDER_WIDTH,
		window->width,
		WIN_BORDER_COLOR,
		WIN_BORDER_WIDTH
	);
}


// ------------------------------------------------------------------------------------------

/* TODO - Why does each window track leftBtnState?
*/
void window_processMouse (

	struct _Window* window,
	int16_t         mouseX,
	int16_t         mouseY,
	uint8_t         leftBtnState
)
{
	// Button currently pressed
	if ( leftBtnState )
	{
		// Button was previously released
		if ( window->prevLeftBtnState == 0 )
		{
			// Raise pressed window
			window_raiseChildWindow( window, mouseX, mouseY );
		}
	}

	// Button currently released
	else
	{
		window->dragTarget = NULL;
	}


	//
	if ( window->dragTarget != NULL )
	{
		// Drag pressed window
		window_dragChildWindow( window, mouseX, mouseY );
	}

	// Updated saved state
	window->prevLeftBtnState = leftBtnState;
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
	struct _Window*  winDesktop;
	uint16_t         winFlags;
	int              status;

	// Allocate space for the desktop
	desktop = ( struct _Desktop* ) malloc( sizeof( struct _Desktop ) );

	if ( desktop == NULL )
	{
		return NULL;
	}


	// Initialize the window part of the desktop
	winDesktop = ( struct _Window* ) desktop;

	winFlags = WIN_FLAG_NO_DECORATION;

	status = window_init(

		winDesktop,
		0,
		0,
		context->width,
		context->height,
		winFlags,
		context
	);

	if ( status == 0 )
	{
		free( desktop );

		return NULL;
	}


	// Initialize the desktop specific parts
	desktop->mouseX          = winDesktop->width / 2;
	desktop->mouseY          = winDesktop->height / 2;
	winDesktop->paintHandler = desktop_paintHandler;  // override the default paintHandler


	//
	return desktop;
}


// ------------------------------------------------------------------------------------------

void desktop_paintHandler ( struct _Window* desktop )
{
	context_fillRect(

		desktop->context,
		0,
		0,
		desktop->context->width,
		desktop->context->height,
		DESKTOP_COLOR
	);
}


// ------------------------------------------------------------------------------------------

void desktop_processMouse (

	struct _Desktop* desktop,
	int16_t          mouseX,
	int16_t          mouseY,
	uint8_t          leftBtnState
)
{
	struct _Window* winDesktop;

	winDesktop = ( struct _Window* ) desktop;

	// Save mouse location (why?)
	desktop->mouseX = mouseX;
	desktop->mouseY = mouseY;

	// Handle mouse
	window_processMouse( winDesktop, mouseX, mouseY, leftBtnState );

	// Update screen to reflect changes mouse-event may have caused
	window_paint( winDesktop );

	// Draw the mouse
	cursor_paint( winDesktop->context, mouseX, mouseY );
}




/* ==========================================================================================
   ...
   ========================================================================================== */

void debug_strokeRect ( struct _Context* context, struct _Rect* rect, uint32_t color, int strokeWeight )
{
	context_strokeRect(

		context,
		rect->left,
		rect->top,
		rect_getWidth( rect ),
		rect_getHeight( rect ),
		color,
		strokeWeight
	);
}

void debug_fillRect ( struct _Context* context, struct _Rect* rect, uint32_t color )
{
	context_fillRect(

		context,
		rect->left,
		rect->top,
		rect_getWidth( rect ),
		rect_getHeight( rect ),
		color
	);
}

void debug_drawClipRects ( struct _Context* context, uint32_t strokeColor, int strokeWeight, uint32_t fillColor )
{
	struct _ListNode* node;
	struct _Rect*     rect;

	node = context->clipRects->rootNode;

	while ( node != NULL )
	{
		rect = ( struct _Rect* ) node->value;

		if ( fillColor != 0 )
		{
			context_fillRect(

				context,
				rect->left,
				rect->top,
				rect_getWidth( rect ),
				rect_getHeight( rect ),
				fillColor
			);
		}

		context_strokeRect(

			context,
			rect->left,
			rect->top,
			rect_getWidth( rect ),
			rect_getHeight( rect ),
			strokeColor,
			strokeWeight
		);

		/*printf(

			"%d %d %d %d\n",
			rect->left,
			rect->top,
			rect_getWidth( rect ),
			rect_getHeight( rect )
		);*/

		node = node->next;
	}
}




/* ==========================================================================================
   ...
   ========================================================================================== */

void tut_init ( void )
{
	// struct _Window* win0;
	// struct _Window* win1;
	// struct _Window* win2;
	// struct _Window* win3;

	gContext = context_new( SCREEN_WIDTH, SCREEN_HEIGHT );

	gDesktop = desktop_new( gContext );

	win0 = window_createChildWindow( ( struct _Window* ) gDesktop,  10,  10, 300, 200, 0 );
	win1 = window_createChildWindow( ( struct _Window* ) gDesktop, 100, 150, 400, 400, 0 );
	win2 = window_createChildWindow( ( struct _Window* ) gDesktop, 200, 100, 200, 400, 0 );

	win3 = window_createChildWindow(

		win0,
		win0->x + win0->width - 50,
		15,
		// win0->x + win0->width / 2 - 50,
		// win0->y + win0->height / 2 - 50,
		100, 100,
		0
	);


//	window_paint( ( struct _Window* ) gDesktop );
window_paint( win0 );
}

void tut_main ( void )
{
	// Poll mouse status...
//	desktop_processMouse( gDesktop, gMouseX, gMouseY, gLeftBtnState );
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
