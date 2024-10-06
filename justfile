flags := "-Wall -ggdb -Wextra -Wpedantic -lm"
raylibFlags := "-I./include/raylib/ -L./include/raylib/ -lraylib"
box2dFlags := "-I./include/ -L./include/box2d/ -lbox2d"
default: build
	./build/2d

build: create_build_dir game colled

game: create_build_dir
	gcc -o ./build/2d -I./src/common ./src/common/*.h ./src/common/*.c ./src/*.c {{flags}} {{raylibFlags}} {{box2dFlags}}

colled: create_build_dir
	gcc -o ./build/colled -I./src/common ./src/common/*.h ./src/common/*.c ./src/colled/*.c ./src/colled/*.h {{flags}} {{raylibFlags}} 

create_build_dir:
	mkdir -p ./build
