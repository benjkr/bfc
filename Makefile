all: bin/bf-compiler bin/bf-interpreter bin/bf-jit

bin/bf-compiler: src/bf_compiler.c src/bf.c
	cc -Wall -o ./bin/bf-compiler -O3 src/bf_compiler.c

bin/bf-interpreter: src/bf_interpreter.c src/bf.c
	cc -Wall -o ./bin/bf-interpreter -O3 src/bf_interpreter.c

bin/bf-jit: src/bf_jit.c src/bf.c src/jit/jit.c src/jit/.jit.c
	cc -Wall -ggdb -o ./bin/bf-jit -O3 src/bf_jit.c

src/jit/.jit.c: jit_generator/generate_x86-64.py
	python3 jit_generator/generate_x86-64.py > src/jit/.jit.c