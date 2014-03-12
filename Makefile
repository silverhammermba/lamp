CXXFLAGS=-std=c++11

scene: main.o
	$(CXX) $(CXXFLAGS) -o $@ $+ -lGL -lGLEW -lSDL2 -lSDL2_image
