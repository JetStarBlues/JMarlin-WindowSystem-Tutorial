
struct _List {

	unsigned int      nItems;
	struct _ListNode* rootNode;
	struct _ListNode* tailNode;
};

struct _ListNode
{
	void*             value;
	struct _ListNode* prev;
	struct _ListNode* next;
};



struct _Context {

	uint32_t*    frameBuffer;
	unsigned int width;
	unsigned int height;
	unsigned int nPixels;
};

struct _Window {

	int16_t  x;
	int16_t  y;
	uint16_t width;
	uint16_t height;
	uint32_t color;

	struct _Context* context;

};

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


