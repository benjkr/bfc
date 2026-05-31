all: bin/bfc

bin/bfc: src/bfc.c src/bf.h src/bf_lex.c src/bf_jit.c src/jit/jit.h src/jit/.jit.h src/bf_interpreter.c src/bf_compiler.c src/bf_compiler_asm.c
	gcc -Wall -o ./bin/bfc -O3 src/bf_jit.c src/bf_interpreter.c src/bf_compiler.c src/bf_lex.c src/bf_compiler_asm.c src/bfc.c

src/jit/.jit.h: jit_generator/generate_x86-64.py
	python3 jit_generator/generate_x86-64.py > src/jit/.jit.h