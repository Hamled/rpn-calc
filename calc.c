#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef double val;
typedef struct {
    size_t count;
    size_t capacity;
    val *entries;
} val_stack_t;
val_stack_t val_stack;

void val_init() {
    val_stack.count = 0;
    val_stack.capacity = 0;
    val_stack.entries = NULL;
}

void val_push(val entry) {
    if(val_stack.capacity < val_stack.count + 1) {
        size_t old_cap = val_stack.capacity;
        val_stack.capacity = old_cap < 8 ? 8 : old_cap * 2;
        size_t new_size = sizeof(val) * val_stack.capacity;
        val *new_entries = (val *)realloc(val_stack.entries, new_size);
        if(!new_entries) {
            free(val_stack.entries);
            puts("Stack overflow");
            abort();
        }
        val_stack.entries = new_entries;
    }

    val_stack.entries[val_stack.count++] = entry;
}

val val_pop() {
    if(!val_stack.count) {
        puts("Stack underflow");
        abort();
    }

    return val_stack.entries[--val_stack.count];
}


typedef enum {
    ADD,
    SUB,
    MUL,
    DIV
} op;

typedef enum {
    NONE,
    INT,
    FRAC
} state_t;

#define ctod(c) ((double)(c - 0x30))

#define DO_OP(op)                               \
    do {                                        \
        val b = val_pop();                      \
        val a = val_pop();                      \
        val_push(a op b);                       \
    } while(false)                              \

int main() {

    val_init();

    char in_buf[BUFSIZ];
    ssize_t read_sz = 0;

    state_t state = NONE;
    int frac_dig = 1;
    val cur_value = 0.0;
    while((read_sz = read(STDIN_FILENO, in_buf, BUFSIZ)) > 0) {
        for(size_t i = 0; i < read_sz; i++) {
            const char c = in_buf[i];
            if(isdigit(c)) {
                if(state == FRAC) {
                    cur_value += pow(10.0, -frac_dig) * ctod(c);
                    frac_dig++;
                } else {
                    cur_value *= 10.0;
                    cur_value += ctod(c);
                    state = INT;
                }
                continue;
            } else if(c == '.') {
                frac_dig = 1;
                state = FRAC;
                continue;
            }

            if(state != NONE) {
                val_push(cur_value);
                cur_value = 0.0;
                state = NONE;
            }

            if(!isspace(c)) {
                switch(c) {
                    case '+': DO_OP(+); break;
                    case '-': DO_OP(-); break;
                    case '*': DO_OP(*); break;
                    case '/': DO_OP(/); break;
                    default:
                        printf("Unknown operation %c\n", c);
                        abort();
                }
            }
        }
    }

    if(read_sz < 0) {
        printf("STDIN read error: %s\n", strerror(errno));
        abort();
    }

    printf("%g\n", val_pop());

    return EXIT_SUCCESS;
}
