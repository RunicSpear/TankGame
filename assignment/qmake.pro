TEMPLATE = app

#Executable Name
TARGET = Assignment
CONFIG = debug

#Destination
DESTDIR = .
OBJECTS_DIR = ./build/

HEADERS	+= 	../common/Shader.h	    \	
	../common/Vector.h		        \	
	../common/Matrix.h		        \
	../common/Mesh.h		        \
	../common/Texture.h             \		
	../common/SphericalCameraManipulator.h

#Sources
SOURCES += 	main.cpp			        \
		./sources/sound.cpp				\
		../common/Shader.cpp		    \
		../common/Vector.cpp		    \
		../common/Matrix.cpp		    \
		../common/Mesh.cpp		        \
		../common/Texture.cpp           \
		../common/SphericalCameraManipulator.cpp

INCLUDEPATH += 	./ ../common/

linux-g++ {

	message("Building for Linux")

	LIBS +=	-lGLEW -lglut -lGLU -lGL -lcanberra

} else:win32-g++ {

	message("Building for Windows")

	!exists("./windows.config") {
		error("windows.config does not exist. See CROSSCOMPILING.md")
	}

	include("./windows.config")

	!exists($$PATH_TO_GLEW) || !exists("$$PATH_TO_GLEW/lib") {
		error("GLEW not found (looked in $$PATH_TO_GLEW[/lib]). See CROSSCOMPILING.md")
	}

	!exists($$PATH_TO_FREEGLUT) || !exists("$$PATH_TO_FREEGLUT/lib") {
		error("FreeGLUT not found (looked in $$PATH_TO_FREEGLUT[/lib]). See CROSSCOMPILING.md")
	}

	QMAKE_CC = /usr/bin/x86_64-w64-mingw32-gcc
	QMAKE_CXX = /usr/bin/x86_64-w64-mingw32-g++
	QMAKE_LINK = /usr/bin/x86_64-w64-mingw32-g++

	QMAKE_CFLAGS = $$replace(QMAKE_CFLAGS, "-fno-keep-inline-dllexport", "")
	QMAKE_CXXFLAGS = $$replace(QMAKE_CXXFLAGS, "-fno-keep-inline-dllexport", "")
	
	QMAKE_CXXFLAGS += -DFREEGLUT_STATIC -DGLEW_STATIC -static-libgcc -static-libstdc++
	QMAKE_LFLAGS += -static-libgcc -static-libstdc++ -Wl,--subsystem,windows

	INCLUDEPATH += $$PATH_TO_GLEW/include
	INCLUDEPATH += $$PATH_TO_FREEGLUT/freeglut/include/

	LIBPATH += $$PATH_TO_MINGW
	LIBPATH += $$PATH_TO_GLEW/lib
	LIBPATH += $$PATH_TO_FREEGLUT/freeglut/lib

	LIBS += -lglew32 -lfreeglut_static -lglu32 -lgdi32 -lwinmm -lopengl32

} else {
	error("Unknown configuration")
}
