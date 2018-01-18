/**
 * Pretty simple brainfuck interpreter
 * @author Patrick Kage
 * @date 2018-01-17
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

// constants
#define MEMORY_SIZE 30000


/**
 * Current state of the program
 */
typedef struct bf_state {
	char* exec_buf;          // instruction buffer
	unsigned int exec_size;  // size of instruction region
	char* exec_ptr;          // current execution pointer

	int* mem_buf;            // memory buffer
	unsigned int mem_size;   // size of memory
	int* mem_ptr;            // current memory pointer
} bf_state;


// prototypes because i'm too lazy to make more than one file
bf_state init_interpreter(char* filename);
bool     tick_interpreter(bf_state *state);
void     free_interpreter(bf_state *state);
#ifdef DEBUG
void     debug(bf_state *state);
#endif

// here we go!
int main(int argc, char** argv) {
	// get the filename
	if (argc != 2) {
		fprintf(stderr, "usage: %s filename\n", argv[0]);
		return 1;
	}

	// initialize bf interpreter with the file
	bf_state state = init_interpreter(argv[1]);

	while (tick_interpreter(&state)) {}

	free_interpreter(&state);
}

/**
 * Initialize interpreter
 * @param filename
 * @return a filled bf_state struct
 */
bf_state init_interpreter(char* filename) {
	bf_state state;

	// open the file
	FILE *fp = fopen(filename, "r");
	if (!fp) {
		fprintf(stderr, "file '%s' does not exist!\n", filename);
		exit(1);
	}

	// figure out how long the instruction array is
	fseek(fp, 0, SEEK_END);
	state.exec_size = (unsigned int)ftell(fp);
	rewind(fp);

	// alloc the execution buffer, and read the file into it
	state.exec_buf = (char*)malloc((size_t)state.exec_size);
	fread(state.exec_buf, state.exec_size, 1, fp);
	fclose(fp);

	// aim the execution pointer
	state.exec_ptr = state.exec_buf;

	// init the memory buffer
	state.mem_buf = (int*)calloc(sizeof(int), MEMORY_SIZE);
	state.mem_ptr = state.mem_buf;
	state.mem_size = MEMORY_SIZE;

	return state;
}


/**
 * Execute one instruction
 * @param bf_struct
 * @return false if should exit
 */
bool tick_interpreter(bf_state *state) {
#ifdef DEBUG
	debug(state);
#endif
	if (state->exec_ptr - state->exec_buf >= state->exec_size) {
		return false;
	}

	// mem buffer checker
	int mem_pos = state->mem_ptr - state->mem_buf;
	// make some ints if we need em
	int exec_pos = 1; // some non-zero
	unsigned int bracket_count;


	// execute the next command
	switch(*(state->exec_ptr)) {
		case '+':
			++*state->mem_ptr;
			break;
		case '-':
			--*state->mem_ptr;
			break;
		case '>':
			if (mem_pos + 1 >= state->mem_size) {
				fprintf(stderr, "out-of-bounds memory read (right)!");
				exit(2);
			}
			++state->mem_ptr;
			break;
		case '<':
			if (mem_pos == 0) {
				fprintf(stderr, "out-of-bounds memory read (left)!\n");
				exit(2);
			}
			--state->mem_ptr;
			break;
		case '.':
			putchar(*state->mem_ptr);
			break;
		case ',':
			*state->mem_ptr = getchar();
			break;
		case '[':
			if (*state->mem_ptr != 0) break;
			// fast forward!
			for (int bracket_count = 0; exec_pos < state->exec_size; ++state->exec_ptr) {
				if (*state->exec_ptr == '[') {
					bracket_count++;
				} else if (*state->exec_ptr == ']') {
					bracket_count--;
				}
				exec_pos = (state->exec_ptr - state->exec_buf);
				if (bracket_count == 0) break;
			}
			if (exec_pos == state->mem_size) {
				fprintf(stderr, "unmatched '['!\n");
				exit(2);
			}
			break;
		case ']':
			if (*state->mem_ptr == 0) break;
			// start rewinding!
			for (int bracket_count = 0; exec_pos >= 0; --state->exec_ptr) {
				if (*state->exec_ptr == '[') {
					bracket_count--;
				} else if (*state->exec_ptr == ']') {
					bracket_count++;
				}
				exec_pos = (state->exec_ptr - state->exec_buf);
				if (bracket_count == 0) break;
			}
			if (exec_pos < 0) {
				fprintf(stderr, "unmatched ']'!\n");
				exit(2);
			}
			break;
		default:
			break;
	}

	state->exec_ptr++;
	return true;
}

/**
 * Cleans up the bf_struct
 * @param bf_struct
 */
void free_interpreter(bf_state *state) {
	free(state->mem_buf);
	free(state->exec_buf);
}

#ifdef DEBUG
#include <unistd.h>

/**
 * debug function - prints the current state
 */
void debug(bf_state *state) {
	int exec_pos = state->exec_ptr - state->exec_buf;
	char str[] = "                              ";
	char align[] = "               ^";
	for (int c = 0; c < 30; c++) {
		if ((exec_pos + (c - 15)) < 0 || (exec_pos + (c - 15)) >= state->exec_size) {
			str[c] = ':';
			continue;
		}
		char instr = *(state->exec_ptr + (c - 15));
		switch (instr) {
			case '+':
			case '-':
			case '>':
			case '<':
			case '[':
			case ']':
			case ',':
			case '.':
				str[c] = instr;
				break;
			default:
				str[c] = ' ';
				break;
		}
	}

	int mem_pos = state->mem_ptr - state->mem_buf;
	printf("mem: [%d, %d], exec: [%d]\n{%s}\n %s\n", mem_pos, *state->mem_ptr, exec_pos, str, align);
	usleep(1000000);
}
#endif
