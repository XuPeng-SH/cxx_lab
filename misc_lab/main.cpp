#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <gflags/gflags.h>

using namespace std;

DEFINE_string(path, "", "output file path");

int main(int argc, char** argv) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    int fd = open(FLAGS_path.c_str(), O_CREAT | O_RDONLY, 0644);
    if(fd < 0){
        perror("open()");
        exit(EXIT_FAILURE);
    }
    cout << fd << endl;
    close(fd);
    return 0;
}
