#include <iostream>
#include <string>
#include <memory.h>
#include <vector>
#include <memory>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sstream>
#include <assert.h>

#include "consts.h"
#include "status.h"
#include "statement.h"
#include "user_schema.h"
#include "tokener.h"
#include "pager.h"
#include "table.h"


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
execute_statement(const Statement& st) {
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
                    status = store_row(user);
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

void
test_schema() {
    char buff[1000];
    memset(buff, 0, 1000);
    UserSchema user1;
    user1.id = 1;
    user1.SetUserName("XuPeng");
    user1.SetEmail("xupeng3112@163.com");
    user1.SerializeTo(buff);

    UserSchema user2;
    user2.id = 2;
    user2.SetUserName("Nana");
    user2.SetEmail("nana@163.com");
    user2.SerializeTo(buff + sizeof(UserSchema));

    UserSchema other1, other2;
    other1.DeserializeFrom(buff);
    other2.DeserializeFrom(buff + sizeof(UserSchema));

    assert(other1.id == user1.id);
    assert(other2.id == user2.id);
}

void
test_table() {
    auto table = Table::Open("/tmp/xx");
    void* page = nullptr;
    auto status = table->pager->GetPage(0, page);
    assert(status.ok());
    auto cursor = table->StartCursor();

    UserSchema user1;
    user1.id = 1;
    user1.SetUserName("XuPeng");
    user1.SetEmail("xupeng3112@163.com");

    user1.SerializeTo(cursor->Value());

    UserSchema user2;
    user2.DeserializeFrom(cursor->Value());
    assert(user1.id == user2.id);
}

int main(int argc, char** argv) {
    test_schema();
    test_table();

    /* string line = "select 1 2 3"; */
    /* Tokener::Parse(line); */

    /* while (status.ok()) { */
    /*     cout << "c->row_num=" << c->row_num << endl; */
    /*     status = c->Advance(); */
    /* } */
    /* cout << "status " << status.err_msg << endl; */

    /* size_t pos = STORE_POS; */

    /* while (true) { */
    /*     cout << PROMPT; */
    /*     string input; */
    /*     getline(cin, input); */
    /*     Statement st; */
    /*     auto status = process_input(input, st); */
    /*     if (!status.ok()) { */
    /*         if (status.type == StatusType::EXIT) { */
    /*             cout << PROMPT << status.err_msg << endl; */
    /*             break; */
    /*         } else { */
    /*             cout << PROMPT << status.err_msg << endl; */
    /*         } */
    /*         continue; */
    /*     } */
    /*     status = execute_statement(st); */
    /*     if (!status.ok()) { */
    /*         cout << PROMPT << status.err_msg << endl; */
    /*     } */
    /*     if (pos != STORE_POS) { */
    /*         UserSchema user; */
    /*         user.DeserializeFrom(STORE + pos); */
    /*         cout << "Detect new row: " << "id=" << user.id << " username=" << user.username << " email=" << user.email << endl; */
    /*         pos = STORE_POS; */
    /*     } */
    /* } */
    return 0;
}
