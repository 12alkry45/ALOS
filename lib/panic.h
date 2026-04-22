#ifndef LIB_PANIC_H
#define LIB_PANIC_H

#define NORETURN __attribute__((noreturn))

NORETURN void panic(const char *msg);

#define PANIC(msg) \
    do { \
        kernel_print("KERNEL PANIC\n"); \
        kernel_print("FILE: " __FILE__ "\n"); \
        kernel_print("LINE: "); \
        kernel_print(int_to_ascii(__LINE__)); \
        kernel_print("\nMSG: " msg "\n"); \
        panic(msg); \
    } while(0)

#endif
