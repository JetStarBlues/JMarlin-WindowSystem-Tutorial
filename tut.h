
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

	/* Using offsets here allows paintHandlers to be oblivious
	   of their window's absolute position. Instead they can
	   assume that the window's top-corner is at (0, 0) and use
	   relative positioning accordingly.
	*/
	int xOffset;
	int yOffset;

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

struct _Window;

struct _Window {

	int16_t  x;
	int16_t  y;
	uint16_t width;
	uint16_t height;

	uint16_t flags;

	struct _Context* context;

	struct _Window* parent;
	struct _List*   children;

	// mouse
	// uint8_t prevLeftBtnState;  // Why is this tracked separately by each window?

	// drag
	struct _Window* dragTarget;
	uint16_t        dragOffsetX;
	uint16_t        dragOffsetY;

	//
	void ( *paintHandler )             ( struct _Window* );
	void ( *mousePressEventHandler )   ( struct _Window*, int, int );
	void ( *mouseReleaseEventHandler ) ( struct _Window*, int, int );
	void ( *mouseIsPressedHandler )    ( struct _Window*, int, int );
};


// Window flags
#define WIN_FLAG_NO_DECORATION 0x1

// Theme
#define WIN_BACKGROUND_COLOR 0xBBBBBBFF
#define WIN_TITLEBAR_COLOR   0x7092BEFF
// #define WIN_BORDER_COLOR     0x000000FF
#define WIN_BORDER_COLOR     0xFF0000FF
#define DESKTOP_COLOR        0x2C2137FF

//
#define WIN_TITLEBAR_HEIGHT 25  // px
#define WIN_BORDER_WIDTH    3   // px


// ------------------------------------------------------------------------------------------

struct _Desktop {

	struct _Window window;

	// mouse
	// int mouseX;
	// int mouseY;
};


// ------------------------------------------------------------------------------------------

struct _MouseState {

	int mouseX;
	int mouseY;

	int mousePressEvent;    // set once, when button transitions from released to pressed
	int mouseReleaseEvent;  // set once, when button transitions from pressed to released
	int mouseIsPressed;     // set for all frames between press (inclusive) and release (exclusive)
};







// ------------------------------------------------------------------------------------------

// Forward declarations
struct _ListNode* listNode_new      ( void* value );
struct _List*     list_new          ( void );
int               list_appendNode   ( struct _List* list, void* value );
void*             list_removeNode   ( struct _List* list, int index );
void*             list_getNodeValue ( struct _List* list, int index );
void              list_freeNodes    ( struct _List* list );
void              list_free         ( struct _List* list );


//
struct _Rect* rect_new             ( int top, int left, int bottom, int right );
int           rect_getWidth        ( struct _Rect* rect );
int           rect_getHeight       ( struct _Rect* rect );
int           rect_rectsIntersect  ( struct _Rect* rectA, struct _Rect* rectB );
struct _List* rect_split           ( struct _Rect* _targetRect, struct _Rect* cuttingRect );
struct _Rect* rect_getIntersection ( struct _Rect* _targetRect, struct _Rect* cuttingRect );


//
struct _Context* context_new               ( int width, int height );
void             context_subtractClipRect  ( struct _Context* context, struct _Rect* subRect );
void             context_addClipRect       ( struct _Context* context, struct _Rect* newRect );
void             context_intersectClipRect ( struct _Context* context, struct _Rect* newRect );
void             context_clearClipRects    ( struct _Context* context );
void             context_lineHorizontal    ( struct _Context* context, int x, int y, int width, uint32_t color, int strokeWeight );
void             context_lineVertical      ( struct _Context* context, int x, int y, int height, uint32_t color, int strokeWeight );
void             context_strokeRect        ( struct _Context* context, int x, int y, int width, int height, uint32_t color, int strokeWeight );
void             context_fillRect          ( struct _Context* context, int x, int y, int width, int height, uint32_t color );
void             context_setPixel          ( struct _Context* context, int x, int y, uint32_t color );


//
struct _Window* window_new                             ( int x, int y, int width, int height, uint16_t flags, struct _Context* context );
int             window_init                            ( struct _Window* window, int x, int y, int width, int height, uint16_t flags, struct _Context* context );
int             window_getAbsoluteXPosition            ( struct _Window* window );
int             window_getAbsoluteYPosition            ( struct _Window* window );
struct _Window* window_createChildWindow               ( struct _Window* window, int x, int y, int width, int height, uint16_t flags );
void            window_appendChildWindow               ( struct _Window* window, struct _Window* childWindow );
struct _List*   window_getChildWindowsAbove            ( struct _Window* window, struct _Window* btmWindow );
void            window_raiseChildWindow                ( struct _Window* window, int mouseX, int mouseY );
void            window_dragChildWindow                 ( struct _Window* window, int mouseX, int mouseY );
void            window_paint                           ( struct _Window* window );
void            window_defaultPaintHandler             ( struct _Window* window );
void            window_paintDecoration                 ( struct _Window* window );
void            window_handleMouseEvent                ( struct _Window* window, struct _MouseState* mouseState, int relMouseX, int relMouseY );
void            window_defaultMousePressEventHandler   ( struct _Window* window, int relMouseX, int relMouseY );
void            window_defaultMouseReleaseEventHandler ( struct _Window* window, int relMouseX, int relMouseY );
void            window_defaultMouseIsPressedHandler    ( struct _Window* window, int relMouseX, int relMouseY );


//
void cursor_paint ( struct _Context* context, int x, int y );


//
struct _Desktop* desktop_new              ( struct _Context* context );
void             desktop_paintHandler     ( struct _Window* desktop );
void             desktop_handleMouseEvent ( struct _Desktop* desktop, struct _MouseState* mouseState );


//
void debug_strokeRect    ( struct _Context* context, struct _Rect* rect, uint32_t color, int strokeWeight );
void debug_fillRect      ( struct _Context* context, struct _Rect* rect, uint32_t color );
void debug_drawClipRects ( struct _Context* context, uint32_t strokeColor, int strokeWeight, uint32_t fillColor );





