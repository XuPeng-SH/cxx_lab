#include <iostream>
#include <string>
#include <memory.h>
#include <vector>

using namespace std;
constexpr const char* PROMPT = "db > ";
constexpr const char* CMD_PREFIX = ".";
constexpr const char* CMD_EXIT = ".exit";
constexpr const char* ST_SELECT = "select";
constexpr const char* ST_DELETE = "delete";
constexpr const char* ST_INSERT = "insert";

constexpr const size_t MAX_STORE_SIZE = 1024 * 1024 * 1024;

static char STORE[MAX_STORE_SIZE];
static size_t STORE_POS = 0;

enum StatusType {
    INVALID_CMD,
    INVALID_ST,
    ILLIGLE_ST,
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

class Tokener {
 public:
    static vector<string>
    Parse(const string& line) {
        vector<string> out;
        if (line.length() == 0) {
            return std::move(out);
        }
        size_t pos = 0;
        size_t pre_pos = 0;
        static const char* DELIMITER = " ";
        while ((pos = line.find(DELIMITER, pre_pos)) != string::npos) {
            auto l = line.substr(pre_pos, pos - pre_pos);
            /* cout << "xxx " << l << " pre_pos = " << pre_pos << " pos =" << pos << endl; */
            out.emplace_back(std::move(l));
            pre_pos = pos + 1;
        }
        auto l = line.substr(pre_pos, pos);
        if (l.length() != 0) {
            out.emplace_back(std::move(l));
        }
        return std::move(out);
    }
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

struct UserSchema {
    constexpr static const uint16_t NameSize = 32;
    constexpr static const uint16_t EMailSize = 255;
    uint32_t id;
    char username[NameSize];
    char email[EMailSize];

    void
    SetUserName(const char* un) {
        memset(username, 0, NameSize);
        if (!un) return;
        strncpy(username, un, sizeof(username));
    }
    void
    SetEmail(const char* e) {
        memset(email, 0, EMailSize);
        if (!e) return;
        strncpy(email, e, sizeof(email));
    }

    void
    SerializeTo(void* destination) const {
        memcpy(destination, this, sizeof(UserSchema));
    }
    void
    DeserializeFrom(void* source) {
        memcpy((void*)this, source, sizeof(UserSchema));
    }
};

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

int main(int argc, char** argv) {
    /* cout << "size of UserSchema is: " << sizeof(UserSchema) << endl; */
    /* char buff[1000]; */
    /* memset(buff, 0, 1000); */

    /* UserSchema user1; */
    /* user1.id = 1; */
    /* user1.SetUserName("XuPeng"); */
    /* user1.SetEmail("xupeng3112@163.com"); */
    /* user1.SerializeTo(buff); */

    /* UserSchema user2; */
    /* user2.id = 2; */
    /* user2.SetUserName("Nana"); */
    /* user2.SetEmail("nana@163.com"); */
    /* user2.SerializeTo(buff + sizeof(UserSchema)); */

    /* UserSchema other1, other2; */
    /* other1.DeserializeFrom(buff); */
    /* other2.DeserializeFrom(buff + sizeof(UserSchema)); */
    /* cout << "user1.username=" << user1.username << endl; */
    /* cout << "user1.email=" << user1.email << endl; */
    /* cout << "user2.username=" << user2.username << endl; */
    /* cout << "user2.email=" << user2.email << endl; */

    /* cout << "other1.username=" << other1.username << endl; */
    /* cout << "other1.email=" << other1.email << endl; */
    /* cout << "other2.username=" << other2.username << endl; */
    /* cout << "other2.email=" << other2.email << endl; */

    /* string line = "select 1 2 3"; */
    /* Tokener::Parse(line); */

    size_t pos = STORE_POS;

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
        if (!status.ok()) {
            cout << PROMPT << status.err_msg << endl;
        }
        if (pos != STORE_POS) {
            UserSchema user;
            user.DeserializeFrom(STORE + pos);
            cout << "Detect new row: " << "id=" << user.id << " username=" << user.username << " email=" << user.email << endl;
            pos = STORE_POS;
        }
    }
    return 0;
}
