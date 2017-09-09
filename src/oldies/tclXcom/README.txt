TclXCom is a loadable module for Tcl8.x which allows certain control
over windows in the X Windows System. Its basically just a set of Xlib calls
with wrappers that allow them to be called from Tcl. 

THE TCL INTERFACE
getXwins screen
Returns a list of all the screens on the specified screen. Each list 
element is of the form {winName winClass winId}
This function should be called before calling any of the other functions that
require a winId as it sets up an internal list of windows that is used in
the other functions. You should also re-call this function if new windows
have appeared on the screen since the last call, and you need to access 
those windows from this package. This really isn't something you have to
worry about though, because you call getXwins to get the needed winId before
accessing the window.

getXcursorloc winId
Returns the xy location of the mouse pointer in relation to the specified
window. The return is of the form 
x_relative_to_win y_relative_to_win x_relative_to_root y_relative_to_root

setXwinclass winId windowName windowClass
Sets the res_name and res_class for the specified window. 

iconifyXwin winId
Iconifies the specified window.

sendXbutton winId x y
Simulates a button press and a button release event at the specified xy 
location in the specified window. It only simulates the first mouse button.

sendXstring windId x y string
Sends a string through the simulation of keyPress events to the specified 
location in the specified window. It recognizes all of the basic keyboard
symbols (upper and lower case) as well as:
\t	tab
\n	newline
\b	backspace (handy for clearing a text box)
it replaces any characters it doesn't recognize with an underscore.

getXscreencount
Returns the number of screens in the current display.

getXwingeometry winId
Returns the width and height of the specified window in the form 
width height 

maximizeXwin winId  
Pseudo maximizes the specified window. It only works if the window has 
its max_height and max_width set in its WM_NORMAL_HINTS. It works
by moving the window to 10 20 to retain the WM decorations and sets the 
window size to its max_width and max_height. 


BUILDING
The included makefile works for me under Solaris. You'll have to set all 
of the lib paths by hand. If you feel like setting this up with a configure
script, feel free. 


CONTACT INFO
If you have questions, suggestions, or improvements, please email me.
tocrawle@eos.ncsu.edu


Copyright 1998, Tobias O. Crawley.
Fully distributable and modifiable as long as you send me a copy and
keep my acknowledgment.

