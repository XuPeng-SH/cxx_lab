#include <stdio.h>
#include "utils.h"

// stdin/stdout/stderr to /dev/null
void redirect_std_streams() {
    stdin = fopen("/dev/null", "r");
    stdout = fopen("/dev/null", "w+");
    stderr = fopen("/dev/null", "w+");
}
