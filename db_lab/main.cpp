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

    PAGE_NUM_OVERFLOW,
    PAGE_LOAD_ERR,
    PAGE_FLUSH_ERR,

    CURSOR_END_OF_FILE,

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

    string
    ToString() const {
        stringstream ss;
        ss << "(" << id << ", " << username << ", " << email << ")";
        return ss.str();
    }
};

struct Pager {
    constexpr static const uint32_t PAGE_SIZE = 4 * 1024;
    constexpr static const uint32_t MAX_PAGES = MAX_STORE_SIZE / PAGE_SIZE;
    constexpr static const uint32_t ROWS_PER_PAGE = PAGE_SIZE / sizeof(UserSchema);

    static shared_ptr<Pager>
    Open(const string& db_path) {
        auto fd = open(db_path.c_str(), O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);
        if (fd == -1) {
            cerr << "Cannot open \"" << db_path << "\"" << endl;
            return nullptr;
        }
        auto pager = make_shared<Pager>();
        pager->file_descriptor = fd;
        pager->file_length = lseek(pager->file_descriptor, 0, SEEK_END);
        memset(pager->pages, 0, MAX_PAGES * sizeof(void*));
        return pager;
    }

    void
    Close() {
        Status status;
        for (auto num = 0; num < MAX_PAGES; ++num) {
            if (pages[num] == nullptr) {
                continue;
            }
            status = FlushPage(num);
            if (!status.ok()) {
                cerr << status.err_msg << endl;
                continue;
            }
            free(pages[num]);
            pages[num] = nullptr;
        }
    }

    Status
    OnPageMissing(const uint32_t& num) {
        Status status;
        pages[num] = calloc(PAGE_SIZE, 1);
        auto all_page_num = file_length / PAGE_SIZE;
        if (num < all_page_num) {
            lseek(file_descriptor, num * PAGE_SIZE, SEEK_SET);
            auto read_size = read(file_descriptor, pages[num], PAGE_SIZE);
            if (read_size == -1) {
                status.type = StatusType::PAGE_LOAD_ERR;
                status.err_msg = string("PAGE_LOAD_ERR: load page ") + to_string(num);
                return status;
            }
        }

        return status;
    }

    Status
    FlushPage(uint32_t num) {
        Status status;
        if (num > MAX_PAGES) {
            status.type = PAGE_NUM_OVERFLOW;
            status.err_msg = string("PAGE_NUM_OVERFLOW: ") + to_string(num);
            return status;
        }

        if (pages[num] == nullptr) {
            return status;
        }

        lseek(file_descriptor, num * PAGE_SIZE, SEEK_SET);
        auto written = write(file_descriptor, pages[num], PAGE_SIZE);
        if (written == -1) {
            status.type = PAGE_FLUSH_ERR;
            status.err_msg = string("PAGE_FLUSH_ERR: ") + to_string(num);
            return status;
        }
        return status;
    }

    Status
    GetPage(uint32_t num, void*& page) {
        Status status;
        if (num > MAX_PAGES) {
            status.type = StatusType::PAGE_NUM_OVERFLOW;
            status.err_msg = string("PAGE_NUM_OVERFLOW: ") + to_string(num);
        }

        if (pages[num] == nullptr) {
            status = OnPageMissing(num);
        }

        if (!status.ok()) {
            return status;
        }

        page = pages[num];

        return status;
    }

    uint32_t
    PageNums() const {
        return file_length / PAGE_SIZE;
    }

    ~Pager() {
        /* for (auto& page : pages) { */
        /*     if (page != nullptr) { */
        /*         free(page); */
        /*         page = nullptr; */
        /*     } */
        /* } */
        Close();
        if (file_descriptor != -1) {
            close(file_descriptor);
            file_descriptor = -1;
        }
    }

    int file_descriptor = -1;
    uint32_t file_length;
    void* pages[MAX_PAGES];
};

struct Table : public enable_shared_from_this<Table> {
    uint32_t num_rows;
    shared_ptr<Pager> pager = nullptr;
    static shared_ptr<Table>
    Open(const string db_path) {
        auto pager = Pager::Open(db_path);
        /* if (!pager) { */
        /*     return nullptr; */
        /* } */
        auto table = make_shared<Table>();
        table->pager = pager;
        /* table->num_rows = pager->file_length / sizeof(UserSchema); */
        table->num_rows = pager->PageNums() * Pager::ROWS_PER_PAGE;
        return table;
    }

    void
    Close() {
        pager->Close();
    }

    struct Cursor {
        void*
        Value() {
            uint32_t page_num = row_num / Pager::PAGE_SIZE;
            void* page;
            auto status = table->pager->GetPage(page_num, page);
            if (!status.ok()) {
                return nullptr;
            }
            uint32_t row_offset = row_num % Pager::ROWS_PER_PAGE;
            uint32_t byte_offset = row_offset * sizeof(UserSchema);
            return (char*)page + byte_offset;
        }

        Status
        Advance() {
            Status status;
            if (end_of_table) {
                status.type = StatusType::CURSOR_END_OF_FILE;
                status.err_msg = "CURSOR_END_OF_FILE";
                return status;
            }
            row_num += 1;
            if (row_num == table->num_rows) {
                end_of_table = true;
            }
            return status;
        }

        shared_ptr<Table> table;
        uint32_t row_num;
        bool end_of_table;
    };

    shared_ptr<Cursor>
    StartCursor() {
        auto c = make_shared<Cursor>();
        c->table = shared_from_this();
        c->row_num = 0;
        c->end_of_table = (c->table->num_rows == 0);
        return c;
    }
    shared_ptr<Cursor>
    LastCursor() {
        auto c = make_shared<Cursor>();
        c->table = shared_from_this();
        c->row_num = c->table->num_rows;
        c->end_of_table = true;
        return c;
    }
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
