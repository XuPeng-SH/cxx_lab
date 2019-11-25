#include <stdio.h>
#include <errno.h>
#include <iostream>
#include <string.h>
#include <string>
#include <gflags/gflags.h>
#include "fiu-local.h"
#include <fiu-control.h>

using namespace std;


bool has_table(const string& name) {
    fiu_return_on("has_table", false);

    return true;
}

int main(int argc, char** argv) {
    fiu_init(0);
    fiu_enable("has_table", 1, NULL, 0);

    if (has_table("")) {
        cout << "Has table" << endl;
    } else {
        cout << "Not has table" << endl;
    }


    return 0;
}
