set env DISPLAY :0.0
cd debug/cocoa
set env DYLD_LIBRARY_PATH .
set env LD_LIBRARY_PATH .:/usr/X11R6/lib
file awtest
