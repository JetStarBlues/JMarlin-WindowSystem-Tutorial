
struct _List {

	unsigned int      nItems;
	struct _ListNode* rootNode;
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

	uint16_t x;
	uint16_t y;
	uint16_t width;
	uint16_t height;

	struct _Context* context;

	uint32_t color;  // temp
};

struct _Desktop {

	struct _List*    children;
	struct _Context* context;
};


