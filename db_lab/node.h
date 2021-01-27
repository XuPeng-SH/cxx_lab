#pragma once

#include "pager.h"

#pragma pack(push)
#pragma pack(4)

struct Node {
    enum Type {
        NONE,
        ROOT,
        INTERNAL,
        LEAF
    };

    template <uint32_t BodySize>
    struct Body {
        char buff[BodySize];
    };

    bool
    IsRoot() const {
        return header.type == Type::ROOT;
    }

    void
    SetType(Type type) {
        header.type = type;
    }

    struct NodeHeader {
        void* parent;
        Type type;
    };

    NodeHeader header;
};

template <int PageSize>
struct InternalNode : public Node {
    struct InternalHeader {
        uint32_t num_keys;
        uint32_t right_child;
    };

    constexpr static const uint32_t BodySize = PageSize - sizeof(Node) - sizeof(InternalHeader);
    constexpr static const uint32_t KeySize = sizeof(uint32_t);
    constexpr static const uint32_t ChildSize = sizeof(uint32_t);
    constexpr static const uint32_t CellSize = KeySize + ChildSize;

    /* InternalNode(InternalNode& o) : Node(o), */
    /*     internal_header(o.internal_header), body(o.body) { */
    /* } */
    InternalNode() {
    }

    /* ~InternalNode() { */
    /* } */

    void*
    CellPtr(uint32_t cell_num) {
        if (!body.buff) {
            return nullptr;
        }
        auto pos = cell_num * CellSize;
        if (pos + CellSize <= BodySize) {
            return body.buff[pos];
        }
        return nullptr;
    }

    uint32_t*
    ChildPtr(uint32_t child_num) {
        auto& num_keys = internal_header.num_keys;
        if (child_num > num_keys) {
            return nullptr;
        }
        if (child_num == num_keys) {
            return &internal_header.right_child;
        }
        auto ptr = CellPtr(child_num);
        return ptr;
    }

    uint32_t*
    KeyPtr(uint32_t key_num) {
        auto& ptr = CellPtr(key_num);
        if (!ptr) {
            return ptr;
        }
        return (uint8_t*)ptr + ChildSize;
    }

    uint32_t
    NumOfKeys() const  {
        return internal_header.num_keys;
    }

    void
    SetNumOfKeys(uint32_t num) {
        internal_header.num_keys = num;
    }

    void
    SetRightChild(uint32_t rc) {
        internal_header.right_child = rc;
    }

    InternalHeader internal_header;
    Body<BodySize> body;
};

template <int PageSize>
struct LeafNode : public Node {
    struct LeafHeader {
        uint32_t key_size;
        uint32_t value_size;
        uint32_t num_cells;
    };

    constexpr static const uint32_t BodySize = PageSize - sizeof(Node) - sizeof(LeafHeader);

    void
    SetKeySize(uint32_t key_size) {
        leaf_header.key_size = key_size;
    }

    void
    SetValSize(uint32_t val_size) {
        leaf_header.value_size = val_size;
    }

    uint32_t
    NumOfCells() const {
        return leaf_header.num_cells;
    }

    uint32_t
    CellSize() const {
        return leaf_header.key_size + leaf_header.value_size;
    }

    void*
    CellPtr(uint32_t cell_num) {
        if (!body.buff) {
            return nullptr;
        }
        auto pos = cell_num * CellSize();
        if (pos + CellSize() <= BodySize) {
            return &body.buff[pos];
        }
        return nullptr;
    }

    void*
    CellKeyPtr(uint32_t cell_num) {
        return CellPtr(cell_num);
    }

    void*
    CellValPtr(uint32_t cell_num) {
        void* ret = CellPtr(cell_num);
        if (!ret) {
            return ret;
        }
        return (char*)ret + leaf_header.key_size;
    }

    Status
    PutKey(const uint32_t& cell_num, const uint32_t& key) {
        Status status;
        auto cell_loc = CellKeyPtr(cell_num);
        if (!cell_loc) {
            status.type = StatusType::CELL_OVERFLOW;
            status.err_msg = std::string("CELL_OVERFLOW: ") + std::to_string(cell_num);
            return status;
        }
        memcpy(cell_loc, &key, sizeof(key));
        return status;
    }

    Status
    PutVal(const uint32_t& cell_num, const UserSchema& val) {
        Status status;
        auto cell_loc = CellValPtr(cell_num);
        if (!cell_loc) {
            status.type = StatusType::CELL_OVERFLOW;
            status.err_msg = std::string("CELL_OVERFLOW: ") + std::to_string(cell_num);
            return status;
        }
        val.SerializeTo(cell_loc);
        return status;
    }

    Status
    GetCellKeyVal(const uint32_t& cell_num, uint32_t& key, UserSchema& val) {
        Status status;
        auto cell_loc = CellPtr(cell_num);
        if (!cell_loc) {
            status.type = StatusType::CELL_OVERFLOW;
            status.err_msg = std::string("CELL_OVERFLOW: ") + std::to_string(cell_num);
            return status;
        }
        memcpy(&key, cell_loc, sizeof(key));
        val.DeserializeFrom((char*)cell_loc + sizeof(key));
        return status;
    }

    LeafHeader leaf_header;
    Body<BodySize> body;
};

using InternalPage = InternalNode<Pager::PAGE_SIZE>;
using LeafPage = LeafNode<Pager::PAGE_SIZE>;

#pragma pack(pop)
