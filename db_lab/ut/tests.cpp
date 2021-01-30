#include <memory.h>
#include <iostream>
#include <gtest/gtest.h>
#include <assert.h>
#include <random>
#include <string>
#include <experimental/filesystem>

#include "user_schema.h"
#include "node.h"
#include "table.h"

using namespace std;

int
RandomInt(int start, int end) {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(start, end);
    return dist(rng);
}

class MyUT : public ::testing::Test {

};

TEST_F(MyUT, schema) {
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

    ASSERT_EQ(other1.id, user1.id);
    ASSERT_EQ(other2.id, user2.id);
};

TEST_F(MyUT, table) {
    std::string path = "/tmp/xx";
    {
        std::experimental::filesystem::remove_all(path);
        auto table = Table::Open(path);
        ASSERT_EQ(table->NumOfPages(), 1);

        auto c = table->StartCursor();
        assert(c->end_of_table);

        UserSchema user1;
        user1.id = 1;
        user1.SetUserName("XuPeng");
        user1.SetEmail("xupeng3112@163.com");

        LeafPage* leaf;
        Status status = c->GetLeafPage(leaf);
        ASSERT_TRUE(status.ok());
        ASSERT_EQ(leaf->NumOfCells(), 0);
        c->Insert(user1.id, user1);
        ASSERT_EQ(leaf->NumOfCells(), 1);

        UserSchema user2;
        user2.id = 2;
        user2.SetUserName("Nana");
        user2.SetEmail("nana@163.com");
        c->Insert(user2.id, user2);
        ASSERT_EQ(leaf->NumOfCells(), 2);
    }
    return;

    uint32_t num_keys = (uint32_t)RandomInt(100, 2000);
    {
        auto table = Table::Open(path);
        void* page = nullptr;
        auto status = table->pager->GetPage(0, page);
        ASSERT_TRUE(status.ok());
        InternalPage* ipage = new (page) InternalPage();
        ipage->SetNumOfKeys(num_keys);
        ipage->SetRoot(true);
        ASSERT_TRUE(ipage->IsRoot());
        /* auto p = page; */
        /* for (auto i = 0; i < 20; ++i) { */
        /*     p = (char*)p + i * sizeof(uint32_t); */
        /*     cout << *(uint32_t*)(p) << endl; */
        /* } */
    }
    {
        auto table = Table::Open(path);
        void* page = nullptr;
        auto status = table->pager->GetPage(0, page);
        ASSERT_TRUE(status.ok());
        InternalPage* ipage = new (page) InternalPage();
        /* auto p = page; */
        /* for (auto i = 0; i < 20; ++i) { */
        /*     p = (char*)p + i * sizeof(uint32_t); */
        /*     cout << *(uint32_t*)(p) << endl; */
        /* } */
        ASSERT_TRUE(ipage->IsRoot());
        ASSERT_EQ(ipage->NumOfKeys(), num_keys);
    }

    /* auto cursor = table->StartCursor(); */

    /* user1.SerializeTo(cursor->Value()); */

    /* UserSchema user2; */
    /* user2.DeserializeFrom(cursor->Value()); */
    /* assert(user1.id == user2.id); */
}

TEST_F(MyUT, node) {
    /* cout << "size of LeafNode " << sizeof(LeafNode<Pager::PAGE_SIZE>) << endl; */
    /* cout << "size of node " << sizeof(Node) << endl; */
    /* cout << "size of nodeheader " << sizeof(Node::NodeHeader) << endl; */
    /* cout << "size of Node::Type " << sizeof(Node::Type) << endl; */
    /* cout << "size of vod*" << sizeof(void*) << endl; */
    /* cout << "size of pager " << sizeof(Pager) << endl; */
    LeafPage ln;
    auto cell_k_p = ln.CellKeyPtr(10);
    auto cell_v_p = ln.CellValPtr(10);
    /* cout << "cell_k_p " << cell_k_p << endl; */
    /* cout << "cell_v_p " << cell_v_p << endl; */
    UserSchema u1;
    u1.id = 101;
    u1.SetUserName("one zero one");
    u1.SetEmail("101@163.com");

    ln.PutKey(2, u1.id);
    ln.PutVal(2, u1);
    UserSchema u2;
    uint32_t k;
    ln.GetCellKeyVal(2, k, u2);
    /* cout << "k=" << k << " u2=" << u2.ToString() << endl; */
    ASSERT_EQ(k, u1.id);
    ASSERT_EQ(u1.id, u2.id);

    InternalNode<4096> in;
    in.SetNumOfKeys(12);
    in.SetRoot(true);
    ASSERT_EQ(in.NumOfKeys(), 12);
    ASSERT_TRUE(in.IsRoot());

    auto in1 = in;
    ASSERT_EQ(in1.NumOfKeys(), 12);
    ASSERT_TRUE(in1.IsRoot());
}
