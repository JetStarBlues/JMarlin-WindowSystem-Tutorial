
// ------------------------------------------------------------------------------------------

struct _ToggleButton {

	struct _Window window;  // "inherit" window class

	int   isSet;  // 1 - set, 0 - cleared
	char* text;

	//
	void ( *mouseReleaseEventHandler ) ( struct _ToggleButton*, int, int );
};

#define TOGGLEBTN_COLOR          0x3596BAFF
#define TOGGLEBTN_SET_COLOR      0x426291FF
#define TOGGLEBTN_TEXT_COLOR     0x000000FF
#define TOGGLEBTN_SET_TEXT_COLOR 0x333333FF


// ------------------------------------------------------------------------------------------

struct _TextBox {

	struct _Window window;  // "inherit" window class

	char* text;
};







// ------------------------------------------------------------------------------------------

// Forward declarations
struct _ToggleButton* toggleButton_new                             ( int x, int y, int width, int height );
void                  toggleButton_free                            ( struct _Window* winToggleButton );
void                  toggleButton_setText                         ( struct _ToggleButton* toggleButton, char* text );
void                  toggleButton_paintHandler                    ( struct _Window* winToggleButton );
void                  toggleButton_defaultMouseReleaseEventHandler ( struct _Window* winToggleButton, int relMouseX, int relMouseY );


//
struct _TextBox* textBox_new          ( int x, int y, int width, int height );
void             textBox_free         ( struct _Window* winTextBox );
void             textBox_setText      ( struct _TextBox* textBox, char* text );
void             textBox_appendText   ( struct _TextBox* textBox, char* text );
void             textBox_paintHandler ( struct _Window* winTextBox );



