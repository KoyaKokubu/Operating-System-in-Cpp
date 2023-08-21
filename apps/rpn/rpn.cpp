#include <cstring>
#include <cstdlib>
#include <cstdio>
#include "../syscall.h"
// #include "../../kernel/graphics.hpp"

// auto printk = reinterpret_cast<int (*)(const char*, ...)>(0x000000000010b000);
// auto fill_rect = reinterpret_cast<decltype(FillRectangle)*>(0x000000000010c270);
// auto scrn_writer = reinterpret_cast<PixelWriter**>(0x000000000024e078);

// // auto& printk = *reinterpret_cast<int (*)(const char*, ...)>(0x000000000010b000);
// // auto& fill_rect = *reinterpret_cast<decltype(FillRectangle)*>(0x000000000010c270);
// // auto& scrn_writer = *reinterpret_cast<decltype(screen_writer)*>(0x000000000024e078);
// // #@@range_end(kernelfunc_decl)

int stack_ptr;
long stack[100];

long Pop() {
    long value = stack[stack_ptr];
    --stack_ptr;
    return value;
}

void Push(long value) {
    ++stack_ptr;
    stack[stack_ptr] = value;
}

extern "C" void main (int argc, char** argv) {
    stack_ptr = -1;

    for (int i=1; i < argc; ++i) {
        if (strcmp(argv[i], "+") == 0) {
            long b = Pop();
            long a = Pop();
            Push(a + b);
						// printk("[%d] <- %ld\n", stack_ptr, a + b);
        } else if (strcmp(argv[i], "-") == 0) {
            long b = Pop();
            long a = Pop();
            Push(a - b);
						// printk("[%d] <- %ld\n", stack_ptr, a - b);
        } else if (strcmp(argv[i], "*") == 0) {
            long b = Pop();
            long a = Pop();
            Push(a * b);
						// printk("[%d] <- %ld\n", stack_ptr, a * b);
        } else if (strcmp(argv[i], "/") == 0) {
            long b = Pop();
            long a = Pop();
            Push(a / b);
						// printk("[%d] <- %ld\n", stack_ptr, a / b);
        } else if (strcmp(argv[i], "%") == 0) {
            long b = Pop();
            long a = Pop();
            Push(a % b);
						// printk("[%d] <- %ld\n", stack_ptr, a % b);
        } else {
            long a = atol(argv[i]);
            Push(a);
        }
    }

		// fill_rect(**scrn_writer, Vector2D<int>{100, 10}, Vector2D<int>{200, 200}, ToColor(0x00ff00));
		// // fill_rect(*scrn_writer, Vector2D<int>{100, 10}, Vector2D<int>{200, 200}, ToColor(0x00ff00));

    long result = 0;
		if (stack_ptr >= 0) {
        result = Pop();
    }

		printf("%ld\n", result);
		exit(static_cast<int>(result));
}