viewer: viewer.cpp minecraftmap.cpp
	gcc -o viewer viewer.cpp -lz -lX11 -lGL -lGLU -g
