CXXFLAGS=-std=c++11

scene: main.o
	$(CXX) $(CXXFLAGS) -o $@ $+ -lGL -lGLEW -lSDL2 -lSDL2_image

ref: c3_basic.o
	$(CXX) $(CXX FLAGS) -o $@ $+ -lGL -lGLEW -lSDL2 -lSDL2_image
