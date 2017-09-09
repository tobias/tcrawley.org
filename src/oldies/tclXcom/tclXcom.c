/* TclXcom.c - A package of tcl commands for manipulating windows in the 
   X Windows System. See the README. 
   copyright 1998, Tobias O. Crawley
   tocrawle@eos.ncsu.edu
   Fully distributable and modifiable as long as you send me a copy and 
   keep my acknowledgment 
*/

#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <tcl.h>

/* keeps track of all windows and what screen each is on
   populated when getXwins is called */

struct WindowStruct {
	Window w;
	int screen;
};

struct {
	struct WindowStruct list[256];
	int last;
} gWindowList;

int errorHandler(Display *display, XErrorEvent *err) {
	/* empty function that ignores all errors */

}

int isValidWindowID(Window w) {
	/* checks for the window in gWindowList in an effort to avoid
	   a BAD_WINDOW error */
	int i;
	
	for (i=0;i <= gWindowList.last; i++) 
		if (gWindowList.list[i].w == w) 
			return 1;
	
	return 0;
}

	
Window do_find(Display *display,int screen, Window w, char *res_name, 
	char *res_class, char *class_list) {                   
	/* goes through the entire screen hierarchy finding all windows */
    	XClassHint chint;
    	Window win, root, parent, *children;
 	unsigned int i, n_children;
	char temp_str[256];

	if (XGetClassHint(display, w, &chint)) {
		if (class_list != NULL) {
			sprintf(temp_str,"{%s %s %u} ",
				chint.res_name,chint.res_class,w);
			strcat(class_list,temp_str);
			
			gWindowList.list[gWindowList.last].screen = screen;
			gWindowList.list[gWindowList.last++].w = w;
		} else if (!strcmp(chint.res_name,res_name) && 
			!strcmp(chint.res_class,res_class)) {
        		XFree(chint.res_name);
        		XFree(chint.res_class);
        		return w;
        	}	
	        XFree(chint.res_name);
	        XFree(chint.res_class);
    	}

	if (!XQueryTree(display, w, &root, &parent, &children, &n_children))
	        return 0;
 	for (i = 0; i < n_children; i++) {
        	if (win = do_find(display, screen, children[i],res_name,
			res_class,class_list)) {
            		XFree(children);
            		return win;
        	}
    	}
    	XFree(children);
    	return 0;
} 

Window start_find(Display *display, int screen,char *res_name, char *res_class, 
	char *class_list) {
	/* starts up do_find */
	static Window found_window = None;

	return found_window ? found_window :
		do_find(display,screen, RootWindow(display, screen),
			res_name,res_class,class_list);
}

int getXWinClasses(ClientData client_data, Tcl_Interp *interp, int objc,
	Tcl_Obj * CONST objv[]) {
	/* retuns the res_name res_class and winId for every window on the
	   specified screen */
	
	Tcl_Obj *result_ptr;
	Display *display;
	char class_list[1024];
	int screen;
	char err[80];
	
	err[0] = '\0';
	
	if (objc != 2) {
		Tcl_WrongNumArgs(interp,1,objv,"screen");
		return TCL_ERROR;
	}
	
	Tcl_GetIntFromObj(interp,objv[1],&screen);
	class_list[0] = '\0';
	
	if ((display = XOpenDisplay(NULL)) == NULL) {
		strcpy(err,"can't connect to display.");
	} else {
		gWindowList.last = 0;
		start_find(display,screen,NULL,NULL,class_list);
	}
	
	XCloseDisplay(display);
	
	result_ptr = Tcl_GetObjResult(interp);
	if (err[0] != '\0') {
		Tcl_SetStringObj(result_ptr,err,-1);
		return TCL_ERROR;
	}
	
	Tcl_SetStringObj(result_ptr,class_list,-1);
	return TCL_OK;
	
}	
		
int getXCursorLoc(ClientData client_data, Tcl_Interp *interp, int objc,
	Tcl_Obj * CONST objv[]) {
	/* returns the xy coordinates of the cursor relative to the 
	   specifed window and relative to root */	
	Tcl_Obj *result_ptr;
	Display *display;
	Window w;
	Window dummyW;
	unsigned int dummyUI;
	int root_x_return, root_y_return;
        int win_x_return, win_y_return;
	char loc[12];
	char err[80];
	
	err[0] = '\0';
	
	if (objc != 2) {
		Tcl_WrongNumArgs(interp,1,objv,"windowId");
		return TCL_ERROR;
	}
		
	if ((display = XOpenDisplay(NULL)) == NULL) {
		strcpy(err,"can't connect to display.");
	} else {
		Tcl_GetLongFromObj(interp,objv[1],(long *)&w);
		
		if ((isValidWindowID(w)) == 0) {
			sprintf(err,"unable to find window with id %u.",w);	
		} else {
			XQueryPointer(display, w, &dummyW, &dummyW,
                	      	&root_x_return, &root_y_return, &win_x_return,
                      		&win_y_return, &dummyUI);
			sprintf(loc,"%d %d %d %d",win_x_return,win_y_return,
			root_x_return,root_y_return);
		}
	}
	
	XCloseDisplay(display);

	result_ptr = Tcl_GetObjResult(interp);
	
	if (err[0] == '\0') {
		Tcl_SetStringObj(result_ptr,loc,-1);
		return TCL_OK;
	}
	
	Tcl_SetStringObj(result_ptr,err,-1);
	return TCL_ERROR;
	
}	

int getXWinGeometry(ClientData client_data, Tcl_Interp *interp, int objc,
	Tcl_Obj * CONST objv[]) {
        /* returns the width and height of the specified window */		
	Tcl_Obj *result_ptr;
	Display *display;
	Window w;
	Window dummyW;
	unsigned int dummyUI;
	int dummyI;
        int win_width, win_height;
	char loc[12];
	char err[80];
	
	err[0] = '\0';
	
	if (objc != 2) {
		Tcl_WrongNumArgs(interp,1,objv,"windowId");
		return TCL_ERROR;
	}
		
	if ((display = XOpenDisplay(NULL)) == NULL) {
		strcpy(err,"can't connect to display.");
	} else {
		Tcl_GetLongFromObj(interp,objv[1],(long *)&w);
		
		if ((isValidWindowID(w)) == 0) {
			sprintf(err,"unable to find window with id %u.",w);	
		} else {
			XGetGeometry(display, w, &dummyW, &dummyI,
                	      	&dummyI, &win_width, &win_height,
                      		&dummyUI, &dummyUI);
			sprintf(loc,"%d %d",win_width,win_height);
		}
	}
	
	XCloseDisplay(display);

	result_ptr = Tcl_GetObjResult(interp);
	
	if (err[0] == '\0') {
		Tcl_SetStringObj(result_ptr,loc,-1);
		return TCL_OK;
	}
	
	Tcl_SetStringObj(result_ptr,err,-1);
	return TCL_ERROR;
	
}

int maximizeXWin(ClientData client_data, Tcl_Interp *interp, int objc,
	Tcl_Obj * CONST objv[]) {
	/* a crude maximize function. only works if the window to be resized
	   has a max_width and max_height set in NORMAL_HINTS. It moves the
	   window to 10 20 in an effort to keep the WM decorations on the screen
        */  
	
	Tcl_Obj *result_ptr;
	Display *display;
	Window w;
	XSizeHints *size_hint;
	XSizeHints *user_size_hint;
	unsigned int dummyUI;
	int new_height, new_width;
	int i;
	int screen;
	char err[80];
	
	err[0] = '\0';
	
	if (objc != 2) {
		Tcl_WrongNumArgs(interp,1,objv,"windowId");
		return TCL_ERROR;
	}
		
	if ((display = XOpenDisplay(NULL)) == NULL) {
		strcpy(err,"can't connect to display.");
	} else {
		Tcl_GetLongFromObj(interp,objv[1],(long *)&w);
		
		if ((isValidWindowID(w)) == 0) {
			sprintf(err,"unable to find window with id %u.",w);	
		} else {
			/* determine which screen the window is on */
			while (gWindowList.list[++i].w != w);
			screen = gWindowList.list[i].screen;
			
			size_hint = XAllocSizeHints();
			user_size_hint = XAllocSizeHints();
			
			XGetWMNormalHints(display,w,size_hint,user_size_hint);
			if ((size_hint->flags & PMaxSize) == PMaxSize) {
				new_width = size_hint->max_width;
				new_height = size_hint->max_height;
			} 
			else {
            XFree(size_hint);
            XFree(user_size_hint);
				return TCL_ERROR;
			}
			
			XFree(size_hint);
			XFree(user_size_hint);
					
			/* move the window to 10,20 and set size
		      to screen size */
			XMoveResizeWindow(display,w,10,20,new_width,
				new_height);
			
			XFlush(display);
		}
	}
	
	XCloseDisplay(display);
	
	result_ptr = Tcl_GetObjResult(interp);
	
	if (err[0] != '\0') {
		Tcl_SetStringObj(result_ptr,err,-1);
		return TCL_ERROR;
	}
	
	return TCL_OK;
	
}

int setXWinClass(ClientData client_data, Tcl_Interp *interp, int objc,
	Tcl_Obj * CONST objv[]) {
	/* sets the res_class and res_name */
	
	Tcl_Obj *result_ptr;
	Display *display;
	Window w;
	XClassHint *hint;
	char *nwin_name;
	char *nwin_class;
	char loc[12];
	char err[80];
	
	err[0] = '\0';
	
	if (objc != 4) {
		Tcl_WrongNumArgs(interp,1,objv,
			"windowId newWindowName newWindowClass");
		return TCL_ERROR;
	}
		
	if ((display = XOpenDisplay(NULL)) == NULL) {
		strcpy(err,"can't connect to display.");
		result_ptr = Tcl_GetObjResult(interp);
		Tcl_SetStringObj(result_ptr, err, -1);
		return TCL_ERROR;
	}
	
	Tcl_GetLongFromObj(interp,objv[1],(long *)&w);
	nwin_name = Tcl_GetStringFromObj(objv[2],NULL);
	nwin_class = Tcl_GetStringFromObj(objv[3],NULL); 
	
	if (!(isValidWindowID(w))) {
		XCloseDisplay(display);
		sprintf(err,"can't find window with id %u.",w);
		result_ptr = Tcl_GetObjResult(interp);
		Tcl_SetStringObj(result_ptr, err, -1);
		return TCL_ERROR;
	} else { 
		if ((hint = XAllocClassHint()) == NULL) {
                	strcpy(err,"can't allocate X hint.");
			result_ptr = Tcl_GetObjResult(interp);
			Tcl_SetStringObj(result_ptr, err, -1);
			return TCL_ERROR;
        	}
		
		hint->res_name = nwin_name;
		hint->res_class = nwin_class;
		
		XSetClassHint(display,w,hint);
		XFree(hint);
		
		XFlush(display);
		XCloseDisplay(display);

		return TCL_OK;
		
	}
		
	
}

int iconifyXWin(ClientData client_data, Tcl_Interp *interp, int objc,
	Tcl_Obj * CONST objv[]) {
	/* iconifies the window */
	
	Tcl_Obj *result_ptr;
	Display *display;
	Window w;
	char err[80];
	int screen;
	int i=0;
	
	if (objc != 2) {
		Tcl_WrongNumArgs(interp,1,objv,"windowId");
		return TCL_ERROR;
	}
				
	if ((display = XOpenDisplay(NULL)) == NULL) {
		strcpy(err,"can't connect to display.");
		result_ptr = Tcl_GetObjResult(interp);
		Tcl_SetStringObj(result_ptr, err, -1);
		return TCL_ERROR;
	}
	
	Tcl_GetLongFromObj(interp,objv[1],(long *)&w);
	
	/* determine which screen the window is on */
	while (gWindowList.list[++i].w != w);
		screen = gWindowList.list[i].screen;
		
	if (!(isValidWindowID(w))) {
		XCloseDisplay(display);
		sprintf(err,"can't find window with id %u.",w);
		result_ptr = Tcl_GetObjResult(interp);
		Tcl_SetStringObj(result_ptr, err, -1);
		return TCL_ERROR;
	}  
	
	XIconifyWindow(display,w,screen);
	XFlush(display);
	XCloseDisplay(display);
	
	return TCL_OK;
	
}

int sendXButtonClick(ClientData client_data, Tcl_Interp *interp, int objc,
	Tcl_Obj * CONST objv[]) {
	/* sends a buttonpress and buttonrelease event to the specified
	   window at the specified xy */
	
	Tcl_Obj *result_ptr;
	Display *display;
	Window w;
	Window root_win;
	XEvent ev;
	char *win_name;
	char *win_class;
	int win_x;
	int win_y;
	int root_x;
	int root_y;
	int dummy_int;
	unsigned int dummy_uint;
	char err[80];
	
	if (objc != 4) {
		Tcl_WrongNumArgs(interp,1,objv,
			"windowId x y");
		return TCL_ERROR;
	}
		
	if ((display = XOpenDisplay(NULL)) == NULL) {
		strcpy(err,"can't connect to display.");
		result_ptr = Tcl_GetObjResult(interp);
		Tcl_SetStringObj(result_ptr, err, -1);
		return TCL_ERROR;
	}
	
	Tcl_GetLongFromObj(interp,objv[1],(long *)&w);
	Tcl_GetIntFromObj(interp,objv[2],&win_x);
	Tcl_GetIntFromObj(interp,objv[3],&win_y);
	
	if (!(isValidWindowID(w))) {
		XCloseDisplay(display);
		sprintf(err,"can't find window with id %u.",w);
		result_ptr = Tcl_GetObjResult(interp);
		Tcl_SetStringObj(result_ptr, err, -1);
		return TCL_ERROR;
	} 
	/* get the root win of w */
	XGetGeometry(display,w,&root_win,&dummy_int,&dummy_int,&dummy_uint, 
		&dummy_uint, &dummy_uint, &dummy_uint);
	
	/* get the geometry of the root win */
	XGetGeometry(display,root_win, &root_win, &root_x, &root_y, 
		&dummy_uint, &dummy_uint, &dummy_uint, &dummy_uint);
	
	/*press the first mouse button */
	ev.type = ButtonPress;
	ev.xbutton.type = ButtonPress;
	ev.xbutton.display = display;
	ev.xbutton.root = root_win;
	ev.xbutton.time = CurrentTime;
	ev.xbutton.subwindow = None;
	ev.xbutton.window = w;
	ev.xbutton.x = win_x;
	ev.xbutton.y = win_y;
	ev.xbutton.x_root = root_x + win_x;
	ev.xbutton.y_root = root_y + win_y;
	ev.xbutton.button = 1;
        ev.xbutton.same_screen = True;
	ev.xbutton.state = 0x100;

	XSendEvent(display,w,True,ButtonPressMask,&ev);
        XFlush(display);
		
	/*release the mouse button */
	ev.type = ButtonRelease;
       	ev.xbutton.time = CurrentTime;
 	ev.xbutton.type = ButtonRelease;
       	ev.xbutton.state = 0x100;
	
       	XSendEvent(display,w,True,ButtonReleaseMask,&ev);
       	XFlush(display);
	XCloseDisplay(display);
	
	return TCL_OK;
	
}

unsigned int getKeyCodeFromChar(char c,int *shift, Display *display) {
	/* used to convert characters to keycodes for sendXKeyPress */
	char cs[20];
	
	if (((c>='a') && (c<='z')) || ((c>='A') && (c<='Z')) ||
		((c>='0') && (c<='9'))) {
		cs[0] = c;
		cs[1] = '\0';
		if ((c>='A') && (c<='Z')) {
			*shift = 1;
		} else {
			*shift = 0;
		}
		return XKeysymToKeycode(display,XStringToKeysym(cs));
	}
	
	*shift = 0;
	
	switch (c) {
		case ' ':
			strcpy(cs,"space");
			break;
		case '!':
			*shift = 1;
			strcpy(cs,"exclam");
			break;
		case '@':
			*shift = 1;
			strcpy(cs,"at");
			break;
		case '#':
			*shift = 1;
			strcpy(cs,"numbersign");
			break;
		case '$':
			*shift = 1;
			strcpy(cs,"dollar");
			break;
		case '%':
			*shift = 1;
			strcpy(cs,"percent");
			break;
		case '^':
			*shift = 1;
			strcpy(cs,"asciicircum");
			break;
		case '&':
			*shift = 1;
			strcpy(cs,"ampersand");
			break;
		case '*':
			*shift = 1;
			strcpy(cs,"asterisk");
			break;
		case '(':
			*shift = 1;
			strcpy(cs,"parenleft");
			break;
		case ')':
			*shift = 1;
			strcpy(cs,"parenright");
			break;
		case '-':
			strcpy(cs,"minus");
			break;
		case '_':
			*shift = 1;
			strcpy(cs,"underscore");
			break;
		case '=':
			strcpy(cs,"equal");
			break;
		case '+':
			*shift = 1;
			strcpy(cs,"plus");
			break;
		case '\\':
			strcpy(cs,"backslash");
			break;
		case '|':
			*shift = 1;
			strcpy(cs,"bar");
			break;
		case '`':
			strcpy(cs,"grave");	
			break;
		case '~':
			*shift = 1;
			strcpy(cs,"asciitilde");
			break;
		case '[':
			strcpy(cs,"bracketleft");
			break;
		case '{':
			*shift = 1;
			strcpy(cs,"braceleft");
			break;
		case ']':
			strcpy(cs,"bracketright");
			break;
		case '}':
			*shift = 1;
			strcpy(cs,"braceright");
			break;
		case ';':
			strcpy(cs,"semicolon");
			break;
		case ':':
			*shift = 1;
			strcpy(cs,"colon");
			break;
		case '\'':
			strcpy(cs,"apostrophe");
			break;
		case '"':
			*shift = 1;
			strcpy(cs,"quotedbl");
			break;
		case ',':
			strcpy(cs,"comma");
			break;
		case '<':
			*shift = 1;
			strcpy(cs,"less");
			break;
		case '.':
			strcpy(cs,"period");
			break;
		case '>':
			*shift = 1;
			strcpy(cs,"greater");
			break;
		case '/':
			strcpy(cs,"slash");
			break;
		case '?':
			*shift = 1;
			strcpy(cs,"question");
			break;
		case '\t':
			strcpy(cs,"Tab");
			break;
		case '\n':
			strcpy(cs,"Return");
			break;
		case '\b':
			strcpy(cs,"BackSpace");
			break;
		default:
			*shift = 1;
			strcpy(cs,"underscore");
			break;
	}
	
	return XKeysymToKeycode(display,XStringToKeysym(cs));

}
int sendXKeyPress(ClientData client_data, Tcl_Interp *interp, int objc,
	Tcl_Obj * CONST objv[]) {
	/* sends a string to a window at a specified xy */
	
	Tcl_Obj *result_ptr;
	Display *display;
	Window w;
	Window root_win;
	XEvent ev_press;
	XEvent ev_release;
	int win_x;
	int win_y;
	char *send_string;
	int root_x;
	int root_y;
	unsigned int dummy_uint;
	int dummy_int;
	int i;
	int shift;
	char err[80];
	
	if (objc != 5) {
		Tcl_WrongNumArgs(interp,1,objv,
			"windowId x y string");
		return TCL_ERROR;
	}
		
	if ((display = XOpenDisplay(NULL)) == NULL) {
		strcpy(err,"can't connect to display.");
		result_ptr = Tcl_GetObjResult(interp);
		Tcl_SetStringObj(result_ptr, err, -1);
		return TCL_ERROR;
	}
	
	Tcl_GetLongFromObj(interp,objv[1],(long *)&w);
	Tcl_GetIntFromObj(interp,objv[2],&win_x);
	Tcl_GetIntFromObj(interp,objv[3],&win_y);
	send_string = Tcl_GetStringFromObj(objv[4],NULL); 
	
	if (!(isValidWindowID(w))) {
		XCloseDisplay(display);
		sprintf(err,"can't find window with id %u.",w);
		result_ptr = Tcl_GetObjResult(interp);
		Tcl_SetStringObj(result_ptr, err, -1);
		return TCL_ERROR;
	} 
	/* get the root win of w */
	XGetGeometry(display,w,&root_win,&dummy_int,&dummy_int,&dummy_uint, 
		&dummy_uint, &dummy_uint, &dummy_uint);
		
	/* get the geometry of the root win */
	XGetGeometry(display,root_win, &root_win, &root_x, &root_y, 
		&dummy_uint, &dummy_uint, &dummy_uint, &dummy_uint);

	/*set up the keypress event */
	ev_press.type = KeyPress;
	ev_press.xkey.type = KeyPress;
	ev_press.xkey.display = display;
	ev_press.xkey.root = root_win;
	ev_press.xkey.time = CurrentTime;
	ev_press.xkey.subwindow = None;
	ev_press.xkey.window = w;
	ev_press.xkey.x = win_x;
	ev_press.xkey.y = win_y;
	ev_press.xkey.x_root = root_x + win_x;
	ev_press.xkey.y_root = root_y + win_y;
        ev_press.xkey.same_screen = True;
	
	/*set up the keyrelease event */
	ev_release = ev_press;
	ev_release.type = KeyRelease;
        ev_release.xkey.time = CurrentTime;
       	 ev_release.xkey.type = KeyRelease;
		
	for (i=0;send_string[i]!='\0';i++){
		ev_press.xkey.keycode = getKeyCodeFromChar(
			send_string[i],&shift,display);
		if (shift) {
			ev_press.xkey.state = ShiftMask;
		} else {
			ev_press.xkey.state = 0x100;
		}
		XSendEvent(display,w,True,KeyPressMask,&ev_press);
		XSendEvent(display,w,True,KeyReleaseMask,&ev_release);
	       	XFlush(display);
	}
	
	XCloseDisplay(display);
		
	return TCL_OK;
	
}

int getXScreenCount(ClientData client_data, Tcl_Interp *interp, int objc,
	Tcl_Obj * CONST objv[]) {
	/* returns the number of screens in the display */
	
	Tcl_Obj *result_ptr;
	Display *display;
	char class_list[1024];
	int screen;
	int count;
	char err[80];
	
	if (objc > 1	) {
		Tcl_WrongNumArgs(interp,1,objv,NULL);
		return TCL_ERROR;
	}
	
	
	if ((display = XOpenDisplay(NULL)) == NULL) {
		strcpy(err,"can't connect to display.");
		result_ptr = Tcl_GetObjResult(interp);
		Tcl_SetStringObj(result_ptr, err, -1);
		return TCL_ERROR;
	}
	
	count = XScreenCount(display);
	
	XCloseDisplay(display);

	result_ptr = Tcl_GetObjResult(interp);
	Tcl_SetIntObj(result_ptr,count);
	
	return TCL_OK;
	
}

int Tclxcom_Init(Tcl_Interp *interp) {

	XSetErrorHandler(errorHandler);

	Tcl_CreateObjCommand(interp,"getXwins", getXWinClasses, 
		(ClientData)NULL,(Tcl_CmdDeleteProc *)NULL);
	Tcl_CreateObjCommand(interp,"getXcursorloc", getXCursorLoc, 
		(ClientData)NULL,(Tcl_CmdDeleteProc *)NULL);
	Tcl_CreateObjCommand(interp,"setXwinclass", setXWinClass, 
		(ClientData)NULL,(Tcl_CmdDeleteProc *)NULL);
	Tcl_CreateObjCommand(interp,"iconifyXwin", iconifyXWin, 
		(ClientData)NULL,(Tcl_CmdDeleteProc *)NULL);
	Tcl_CreateObjCommand(interp,"sendXbutton", sendXButtonClick, 
		(ClientData)NULL,(Tcl_CmdDeleteProc *)NULL);
	Tcl_CreateObjCommand(interp,"sendXstring", sendXKeyPress, 
		(ClientData)NULL,(Tcl_CmdDeleteProc *)NULL);
	Tcl_CreateObjCommand(interp,"getXscreencount", getXScreenCount, 
		(ClientData)NULL,(Tcl_CmdDeleteProc *)NULL);
	Tcl_CreateObjCommand(interp,"getXwingeometry", getXWinGeometry, 
		(ClientData)NULL,(Tcl_CmdDeleteProc *)NULL);
	Tcl_CreateObjCommand(interp,"maximizeXwin", maximizeXWin, 
		(ClientData)NULL,(Tcl_CmdDeleteProc *)NULL);
	Tcl_PkgProvide(interp, "TclXCom", "0.3");
	
	return TCL_OK;
}


