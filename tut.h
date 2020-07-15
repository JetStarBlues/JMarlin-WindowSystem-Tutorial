
struct _List {

	int               nItems;
	struct _ListNode* rootNode;
	struct _ListNode* tailNode;
};

struct _ListNode
{
	void*             value;
	struct _ListNode* prev;
	struct _ListNode* next;
};


// ------------------------------------------------------------------------------------------

struct _Context {

	uint32_t* frameBuffer;
	int       width;
	int       height;
	int       nPixels;

	struct _List* clipRects;
};


// ------------------------------------------------------------------------------------------

struct _Rect {

	int top;
	int left;
	int bottom;
	int right;
};


// ------------------------------------------------------------------------------------------

struct _Window {

	int16_t  x;
	int16_t  y;
	uint16_t width;
	uint16_t height;
	uint32_t color;

	struct _Context* context;

};


// ------------------------------------------------------------------------------------------

struct _Desktop {

	struct _Context* context;
	struct _List*    children;

	// mouse
	uint8_t  prevLeftBtnState;
	int16_t  mouseX;
	int16_t  mouseY;

	// drag
	struct _Window* dragTarget;
	uint16_t        dragOffsetX;
	uint16_t        dragOffsetY;
};






// ------------------------------------------------------------------------------------------

// Forward declarations
struct _ListNode* listNode_new      ( void* value );
struct _List*     list_new          ( void );
int               list_appendNode   ( struct _List* list, void* value );
void*             list_removeNode   ( struct _List* list, int index );
void*             list_getNodeValue ( struct _List* list, int index );
void              list_freeNodes    ( struct _List* list );

struct _Rect* rect_new            ( int top, int left, int bottom, int right );
struct _List* rect_split          ( struct _Rect* _targetRect, struct _Rect* cuttingRect );
int           rect_rectsIntersect ( struct _Rect* rectA, struct _Rect* rectB );

struct _Context* context_new              ( int width, int height );
void             context_subtractClipRect ( struct _Context* context, struct _Rect* newRect, uint32_t debugColor );
void             context_addClipRect      ( struct _Context* context, struct _Rect* newRect, uint32_t debugColor );
void             context_clearClipRects   ( struct _Context* context );
void context_lineHorizontal (

	struct _Context* context,
	int              x,
	int              y,
	int              width,
	uint32_t         color,
	int              strokeWeight
);
void context_lineVertical (

	struct _Context* context,
	int              x,
	int              y,
	int              height,
	uint32_t         color,
	int              strokeWeight
);
void context_strokeRect (

	struct _Context* context,
	int              x,
	int              y,
	int              width,
	int              height,
	uint32_t         color,
	int              strokeWeight
);
void context_fillRect (

	struct _Context* context,
	int              x,
	int              y,
	int              width,
	int              height,
	uint32_t         color
);
void context_setPixel (

	struct _Context* context,
	int              x,
	int              y,
	uint32_t         color
);

struct _Window* window_new (

	int              x,
	int              y,
	int              width,
	int              height,
	uint32_t         color,
	struct _Context* context
);
void window_paint ( struct _Window* window );

void cursor_paint ( struct _Context* context, int x, int y );

struct _Desktop* desktop_new ( struct _Context* context );
struct _Window* desktop_createWindow (

	struct _Desktop* desktop,
	int              x,
	int              y,
	int              width,
	int              height,
	uint32_t         color
);
void          desktop_paint           ( struct _Desktop* desktop );
void          desktop_raiseWindow     ( struct _Desktop* desktop );
void          desktop_dragWindow      ( struct _Desktop* desktop );
struct _List* desktop_getWindowsAbove ( struct _Desktop* desktop, struct _Window* btmWindow );
void desktop_processMouse (

	struct _Desktop* desktop,
	int16_t          mouseX,
	int16_t          mouseY,
	uint8_t          leftBtnState
);








