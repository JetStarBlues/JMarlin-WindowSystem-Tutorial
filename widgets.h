
// ------------------------------------------------------------------------------------------

struct _ToggleButton {

	struct _Window window;  // "inherit" window class

	int isSet;  // 1 - set, 0 - cleared
};

#define TOGGLEBTN_COLOR     0x084062FF;
#define TOGGLEBTN_SET_COLOR 0x002841FF;






// ------------------------------------------------------------------------------------------

// Forward declarations

struct _ToggleButton* toggleButton_new                      ( int x, int y, int w, int h );
void                  toggleButton_paintHandler             ( struct _Window* winToggleButton );
void                  toggleButton_mouseReleaseEventHandler ( struct _Window* winToggleButton, int relMouseX, int relMouseY );

