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

TEST_F(MyUT, table_insert) {
    Status status;
    std::string path = "/tmp/xx";
    std::experimental::filesystem::remove_all(path);

    auto table = Table::Open(path);
    ASSERT_EQ(table->NumOfPages(), 1);
    ASSERT_TRUE(table->GetRootPage()->IsLeaf());

    UserSchema user;
    auto i = 0;
    for (; i < LeafPage::CellsCapacity; ++i) {
        user.id = i;
        user.SetUserName((std::string("user") + std::to_string(i)).c_str());
        user.SetEmail((std::string("user") + std::to_string(i) + "@163.com").c_str());
        status = table->Insert(user.id, user);
        ASSERT_TRUE(status.ok());
    }
    ASSERT_EQ(table->NumOfPages(), 1);

    user.id = i;
    user.SetUserName((std::string("user") + std::to_string(i)).c_str());
    user.SetEmail((std::string("user") + std::to_string(i) + "@163.com").c_str());
    status = table->Insert(user.id, user);
    ASSERT_TRUE(status.ok());
    ASSERT_EQ(table->NumOfPages(), 3);

    ASSERT_EQ(table->GetRootPageNum(), 0);
    ASSERT_TRUE(table->GetRootPage()->IsInternal());
    auto root_page = PageHelper::AsInternal(table->GetRootPage());
    ASSERT_TRUE(root_page);
    ASSERT_EQ(root_page->NumOfKeys(), 1);
    uint32_t first_child_page_num;
    status = root_page->GetChildPageNum(0, first_child_page_num);
    ASSERT_TRUE(status.ok());
    ASSERT_EQ(first_child_page_num, 2);
    void* first_child;
    status = table->pager->GetPage(first_child_page_num, first_child);
    ASSERT_TRUE(status.ok());
    LeafPage* first_leaf = PageHelper::AsLeaf(first_child);
    ASSERT_TRUE(first_leaf);
    uint32_t first_key;
    status = root_page->GetKey(0, first_key);
    ASSERT_TRUE(status.ok());
    uint32_t first_leaf_max_key;
    status = first_leaf->GetMaxKey(first_leaf_max_key);
    ASSERT_TRUE(status.ok());
    ASSERT_EQ(first_leaf_max_key, first_key);

    uint32_t right_child_page_num;
    status = root_page->GetChildPageNum(1, right_child_page_num);
    ASSERT_TRUE(status.ok());
    ASSERT_EQ(right_child_page_num, 1);

    void* right_child;
    status = table->pager->GetPage(right_child_page_num, right_child);
    ASSERT_TRUE(status.ok());

    LeafPage* right_leaf = PageHelper::AsLeaf(right_child);
    ASSERT_TRUE(right_leaf);

    ASSERT_EQ(first_leaf->NumOfCells(), 7);
    ASSERT_EQ(right_leaf->NumOfCells(), 7);

    for (auto i = 0; i < root_page->NumOfKeys(); ++i) {
        uint32_t key;
        status = root_page->GetKey(i, key);
        ASSERT_TRUE(status.ok());
        std::cout << "key[" << i << "] = " << key << std::endl;
    }

    ASSERT_EQ(root_page->FindCell(6), 0);
    ASSERT_EQ(root_page->FindCell(7), 1);
    /* for (auto i=0; i<14; ++i) { */
    /*     std::cout << i << " left  find_cell: " << first_leaf->FindCell(i) << std::endl; */
    /* } */
    /* for (auto i=0; i<14; ++i) { */
    /*     std::cout << i << " right find_cell: " << right_leaf->FindCell(i) << std::endl; */
    /* } */
    for (i = 0; i < LeafPage::CellsCapacity*100; ++i) {
        user.id = i + 14;
        user.SetUserName((std::string("user") + std::to_string(i)).c_str());
        user.SetEmail((std::string("user") + std::to_string(i) + "@163.com").c_str());
        status = table->Insert(user.id, user);
        ASSERT_TRUE(status.ok());
    }

    ASSERT_EQ(table->NumOfPages(), 188);

    uint32_t prev_page_num = std::numeric_limits<uint32_t>::max();
    for (i = 0; i < 1000; ++i) {
        auto c = table->Find(i);
        if (c->page_num != prev_page_num) {
            prev_page_num = c->page_num;
        } else {
            ASSERT_EQ(c->page_num, prev_page_num);
        }
        /* std::cout << "KEY=" << i << " PAGE_NUM=" << c->page_num << std::endl; */
    }
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
