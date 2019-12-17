#include <stdio.h>
#include <errno.h>
#include <iostream>
#include <string.h>
#include <string>
#include <gflags/gflags.h>
#include "fiu-local.h"
#include <fiu-control.h>
#include <execinfo.h>

using namespace std;


bool has_table(const string& name) {
    int nptrs;
    void *buffer[100];
    char** strings;
    nptrs = backtrace(buffer, 100);
    /* printf("backtrace() returned %d addresses\n", nptrs); */
    /* backtrace_symbols_fd(buffer, nptrs, 1); */
    strings = backtrace_symbols(buffer, nptrs);
    if (strings == NULL) {
       perror("backtrace_symbols");
       exit(EXIT_FAILURE);
    }

    /* for (int j = 0; j < nptrs; j++) */
    /*    printf("[%d]: %s\n", j, strings[j]); */

    free(strings);
    fiu_return_on("has_table", false);

    return true;
}

bool has_mem(const string& name) {
    has_table(name);
    fiu_return_on("has_mem", false);
    return true;
}

int callback(const char* name, int *failnum, void** failinfo, unsigned int* flags) {
    cout << "cb name=" << name << endl;
    cout << "cb failnum=" << *failnum << endl;
    /* return *failnum; */
    if (*failnum == 0) return 1;
    *failnum = 0;
    return 0;
}

int main(int argc, char** argv) {
    string fail_info = "this is error msg";
    fiu_init(0);
    /* fiu_enable("has_table", 1, const_cast<char*>(fail_info.c_str()), 0); */
    /* fiu_enable_external("has_table", 1, const_cast<char*>(fail_info.c_str()), 0, callback); */

    if (has_table("")) {
        cout << "Has table" << endl;
    } else {
        cout << "Not has table" << endl;
    }

    cout << "fiu_fail(has_table)=" << fiu_fail("has_table") << endl;
    cout << "fiu_failinfo(has_table)=" << (char*)fiu_failinfo() << endl;

    if (has_mem("")) {
        cout << "Has mem" << endl;
    } else {
        cout << "Not has mem" << endl;
    }

    /* fiu_disable("has_table"); */

    if (has_table("")) {
        cout << "Has table" << endl;
    } else {
        cout << "Not has table" << endl;
    }

    cout << "fiu_fail(has_table)=" << fiu_fail("has_table") << endl;
    /* cout << "fiu_failinfo(has_table)=" << (char*)fiu_failinfo() << endl; */
    return 0;
}
