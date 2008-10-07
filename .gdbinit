set env DYLD_INSERT_LIBRARIES /usr/lib/libMallocDebug.A.dylib
set env DISPLAY :0.0
set env MallocGuardEdges 1
set env MallocStackLogging 1
set env MallocPreScribble 1
set env MallocScribble 1
set env MallocBadFreeAbort 1

cd debug/cocoa
set env DYLD_LIBRARY_PATH .
set env LD_LIBRARY_PATH .:/usr/X11R6/lib
file multi
