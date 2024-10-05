flags := "-Wall -ggdb -Wextra -Wpedantic -lraylib -lbox2d -I./include/ -lm"
default: build
	./build/2d

build: create_build_dir
	gcc -o ./build/2d ./src/*.c {{flags}} 

colled: create_build_dir
	gcc -o ./build/colled ./src/colled/*.c ./src/colled/*.h {{flags}} && ./build/colled

create_build_dir:
	mkdir -p ./build