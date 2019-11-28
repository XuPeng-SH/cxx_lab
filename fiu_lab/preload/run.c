/* #include <iostream> */
#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <dlfcn.h>
#include <unistd.h>

static void __attribute__((constructor)) run_init(void) {
    /* std::cout << "Hello run_init" << std::endl; */
    /* printf("Hello run_init\n"); */
}

static __thread void* (*malloc_orig) (size_t size) = NULL;
/* static __thread int malloc_count = 0; */

static void __attribute__((constructor(201))) init_malloc(void) {
    malloc_orig = (void*(*)(size_t))dlsym(RTLD_NEXT, "malloc");
}

void* malloc(size_t size) {
    if (malloc_orig == NULL) {
        init_malloc();
    }
    /* malloc_count += 1; */
    /* if (malloc_count>=2500) { */
    /*     return NULL; */
    /* } */
    return (*malloc_orig)(size);
}
