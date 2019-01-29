main.exe: main.cpp
	cl /MD /EHsc /I.\sdl\include -O2 \
		.\sdl\winlib\sdl2.lib  \
		.\sdl\winlib\sdl2main.lib \
		main.cpp \
		/link /SUBSYSTEM:console
	copy sdl\winlib\*.dll . /y
