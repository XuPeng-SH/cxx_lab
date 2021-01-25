#include <iostream>
#include <string>
#include <memory.h>
#include <unistd.h>
#include <gflags/gflags.h>

#include "consts.h"
#include "status.h"
#include "statement.h"
#include "user_schema.h"
#include "tokener.h"
#include "pager.h"
#include "table.h"
#include "test.h"


using namespace std;

static char STORE[MAX_STORE_SIZE];
static size_t STORE_POS = 0;


Status
process_cmd(const string& cmd) {
    Status status;
    if (cmd == CMD_EXIT) {
        status.type = StatusType::EXIT;
        status.err_msg = "Exit!";
    } else {
        status.type = StatusType::INVALID_CMD;
        status.err_msg = string("INVALID_CMD: \"") + cmd + "\"";
    }
    return status;
}

Status
store_row(const UserSchema& user) {
    Status status;
    user.SerializeTo(STORE + STORE_POS);
    STORE_POS += sizeof(user);
    return status;
}


Status
prepare_select_st(const string& st, Statement& out) {
    Status status;
    out.type = StatementType::SELECT;
    out.st = st;
    return status;
}

Status
prepare_delete_st(const string& st, Statement& out) {
    Status status;
    out.type = StatementType::DELETE;
    out.st = st;
    return status;
}

Status
prepare_insert_st(const string& st, Statement& out) {
    Status status;
    out.type = StatementType::INSERT;
    out.st = st;
    return status;
}

Status
prepare_statement(const string& st, Statement& out) {
    Status status;
    if (st.rfind(ST_SELECT, 0) == 0) {
        status = prepare_select_st(st, out);
    } else if (st.rfind(ST_DELETE, 0) == 0) {
        status = prepare_delete_st(st, out);
    } else if (st.rfind(ST_INSERT, 0) == 0) {
        status = prepare_insert_st(st, out);
    } else {
        status.type = StatusType::INVALID_ST;
        status.err_msg = string("INVALID_ST: \"") + st + "\"";
    }

    return status;
}

Status
execute_statement(const Statement& st, shared_ptr<Table> table) {
    Status status;
    auto statements = Tokener::Parse(st.st);
    if (statements[0] == ST_SELECT) {

    } else if (statements[0] == ST_DELETE) {

    } else if (statements[0] == ST_INSERT) {
        if (statements.size() != 4) {
            status.type = StatusType::ILLIGLE_ST;
            status.err_msg = string("illigle statement \"") + st.st + "\"";
        } else {
            try {
                size_t pos;
                UserSchema user;
                user.id = static_cast<uint32_t>(stoul(statements[1], &pos));
                if (statements[1].size() - pos != 0) {
                    status.type = StatusType::ILLIGLE_ST;
                    status.err_msg = string("illigle statement \"") + st.st + "\"";
                } else {
                    user.SetUserName(statements[2].c_str());
                    user.SetEmail(statements[3].c_str());
                    /* status = store_row(user); */
                    /* auto cursor = table->LastCursor(); */
                    /* user.SerializeTo(cursor->Value()); */
                    /* table->Advance(); */
                    table->Append(user);
                }
            } catch (std::exception& exp) {
                status.type = StatusType::ILLIGLE_ST;
                status.err_msg = string("illigle statement \"") + st.st + "\"";
                /* cout << exp.what() << endl; */
            }
        }

    } else {
        status.type = StatusType::INVALID_ST;
        status.err_msg = string("unkown statement \"") + st.st + "\"";
    }

    return status;
}

Status
process_input(const string& input, Statement& out) {
    Status status;
    if (input.length() == 0) {
        status.type = StatusType::EMPTY;
        status.err_msg = "Empty input!";
        return status;
    }

    if (input.rfind(CMD_PREFIX, 0) == 0) {
        return process_cmd(input);
    }

    status = prepare_statement(input, out);

    return status;
}

DEFINE_string(db_file, "/tmp/xyz", "db file");


int main(int argc, char** argv) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    test_schema();
    test_table();

    auto table = Table::Open(FLAGS_db_file);

    /* while (status.ok()) { */
    /*     cout << "c->row_num=" << c->row_num << endl; */
    /*     status = c->Advance(); */
    /* } */
    /* cout << "status " << status.err_msg << endl; */

    size_t num_rows = table->num_rows;

    while (true) {
        cout << PROMPT;
        string input;
        getline(cin, input);
        Statement st;
        auto status = process_input(input, st);
        if (!status.ok()) {
            if (status.type == StatusType::EXIT) {
                cout << PROMPT << status.err_msg << endl;
                break;
            } else {
                cout << PROMPT << status.err_msg << endl;
            }
            continue;
        }
        status = execute_statement(st, table);
        if (!status.ok()) {
            cout << PROMPT << status.err_msg << endl;
        }
        if (num_rows != table->num_rows) {
            UserSchema user;
            auto c = table->LastCursor();
            user.DeserializeFrom(c->Value() - sizeof(UserSchema));
            cout << "Detect new row: " << "id=" << user.id << " username=" << user.username << " email=" << user.email << endl;
            num_rows = table->num_rows;
        }
    }
    return 0;
}
