#include <string>
#include <iostream>
#include <rocksdb/db.h>
#include <rocksdb/status.h>
#include <map>
#include <gflags/gflags.h>

#include "server.h"
#include "client.h"

using namespace std;
using namespace rocksdb;

DEFINE_string(port, "5555", "port");
DEFINE_string(mode, "server", "server or client");
DEFINE_string(host, "127.0.0.1", "host to connect");
DEFINE_string(table_name, "default", "table name");
DEFINE_string(db_path, "/tmp/rs_lab", "db path");


int main(int argc, char** argv) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    cout << "\n======== Config Block Start =================\n";
    cout << "MODE: " << FLAGS_mode << "\n";
    if (FLAGS_mode == "server") {
        cout << "SERVER_PORT: " << FLAGS_port << "\n";
    } else {
        cout << "CONNECT_HOST: " << FLAGS_host << "\n";
        cout << "CONNECT_PORT: " << FLAGS_port << "\n";
    }
    cout << "======== Config Block End   =================\n" << endl;

    if (FLAGS_mode == "server") {
        Server server(FLAGS_port, FLAGS_db_path);
        server.run();
    } else {
        auto client = Client::Build(FLAGS_host, FLAGS_port);
        auto response = client->CreateTable(FLAGS_table_name);
        cout << "RESPONSE id=" << response->table().id() << endl;
        cout << "RESPONSE name=" << response->table().name() << endl;
    }
    return 0;
}
