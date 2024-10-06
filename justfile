flags := "-Wall -ggdb -Wextra -Wpedantic -lm"
raylibFlags := "-I./include/raylib/ -L./include/raylib/ -lraylib"
box2dFlags := "-I./include/ -L./include/box2d/ -lbox2d"
default: build
	./build/2d

build: create_build_dir game colled

game: create_build_dir
	gcc -o ./build/2d ./src/*.c {{flags}} {{raylibFlags}} {{box2dFlags}}

colled: create_build_dir
	gcc -o ./build/colled ./src/colled/*.c ./src/colled/*.h {{flags}} {{raylibFlags}} && ./build/colled

create_build_dir:
	mkdir -p ./build
