#include "test.h"
#include "table.h"
#include "pager.h"
#include "user_schema.h"

#include <memory.h>
#include <assert.h>

using namespace std;

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
