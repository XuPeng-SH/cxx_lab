#include <iostream>
#include <gflags/gflags.h>

#include "server.h"
#include "async_client.h"

using namespace std;

DEFINE_string(mode, "server", "server or client");
DEFINE_string(host, "127.0.0.1", "host");
DEFINE_string(port, "8000", "port");
DEFINE_int32(index, 1, "index");

int main(int argc, char** argv) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    if (FLAGS_mode == "server") {
        ServerImpl server(FLAGS_host, FLAGS_port);
        server.Run();
    } else if (FLAGS_mode == "client") {
        auto client = AsyncClient::Build(FLAGS_host, FLAGS_port);
        auto resp = client->GetIndex(FLAGS_index);
        if (not resp) {
            cout << "[Request][Index]: " << FLAGS_index << " --> " << " [No Response]" << endl;
        } else {
            cout << "[Request][Index]: " << FLAGS_index << " --> " << " [";
            cout << resp->name << "][";
            cout << resp->index << "]" << endl;
        }
    } else {
        cerr << "FLAGS_mode: " << FLAGS_mode << " is invalid!" << endl;
        assert(false);
    }

    return 0;
}
