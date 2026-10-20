#include "raylib.h"
#include<stddef.h>


void* glfwGetX11Display(void);
unsigned long glfwGetX11Window(void* window);

typedef unsigned long XID;
typedef XID Window;
typedef XID Atom;
typedef void Display;

#define False 0
#define True 1
#define XA_ATOM 4
#define PropModeReplace 0

Display* XOpenDisplay(const char*);
Atom XInternAtom(Display* display, const char* atom_name, int only_if_exists);
int XChangeProperty(Display* display, Window w, Atom property, Atom type, 
                    int format, int mode, const unsigned char* data, int nelements);


void make_window_dock(void){
  void* rawWindow = GetWindowHandle();
  Display* display = XOpenDisplay(NULL);
  Window window = (Window)glfwGetX11Window(rawWindow);

  if (display && window) {
	Atom wmType = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
	Atom typeDock = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DOCK", False);

	XChangeProperty(
		display, window,
		wmType, XA_ATOM, 32,
		PropModeReplace,
		(unsigned char*)&typeDock, 1
		);
  }

}
