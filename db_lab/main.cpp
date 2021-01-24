#include <iostream>
#include <string>

using namespace std;
constexpr const char* PROMPT = "db > ";
constexpr const char* CMD_PREFIX = ".";
constexpr const char* CMD_EXIT = ".exit";
constexpr const char* ST_SELECT = "select";
constexpr const char* ST_DELETE = "delete";

enum StatusType {
    INVALID_CMD,
    INVALID_ST,
    OK,
    EMPTY,
    EXIT
};

struct Status {
    StatusType type = StatusType::OK;
    string err_msg = "";
    bool
    ok() const { return type == StatusType::OK; }
};

enum StatementType {
    INVALID,
    SELECT,
    DELETE,
    INSERT
};

struct Statement {
    StatementType type = StatementType::INVALID;
    string st;
};

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
prepare_statement(const string& st, Statement& out) {
    Status status;
    if (st.rfind(ST_SELECT, 0) == 0) {
        status = prepare_select_st(st, out);
    } else if (st.rfind(ST_DELETE, 0) == 0) {
        status = prepare_delete_st(st, out);
    } else {
        status.type = StatusType::INVALID_ST;
        status.err_msg = string("INVALID_ST: \"") + st + "\"";
    }

    return status;
}

Status
execute_statement(const Statement& st) {
    Status status;
    cout << PROMPT << "executing statement \"" << st.st << "\"" << endl;
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

int main(int argc, char** argv) {
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
        status = execute_statement(st);
    }
    return 0;
}
