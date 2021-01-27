#pragma once

#pragma pack(push)
#pragma pack(4)

struct Node {
    enum Type {
        NONE,
        ROOT,
        INTERNAL,
        LEAF
    };

    struct Body {
        char* buff = nullptr;
    };

    Node(void* p, Type t = Type::NONE) : header(p, t) {}

    bool
    IsRoot() const {
        return header.type == Type::ROOT;
    }

    void
    SetType(Type type) {
        header.type = type;
    }

    struct NodeHeader {
        NodeHeader(void* p, Type t) : parent(p), type(t) {}
        void* parent = nullptr;
        Type type = Type::NONE;
    };

    NodeHeader header;
};

template <int PageSize>
struct InternalNode : public Node {
    struct InternalHeader {
        InternalHeader(uint32_t nk, uint32_t rc) : num_keys(nk), right_child(rc) {}

        InternalHeader() = delete;

        uint32_t num_keys = 0;
        uint32_t right_child = 0;
    };

    constexpr static const uint32_t BodySize = PageSize - sizeof(Node) - sizeof(InternalHeader);
    constexpr static const uint32_t KeySize = sizeof(uint32_t);
    constexpr static const uint32_t ChildSize = sizeof(uint32_t);
    constexpr static const uint32_t CellSize = KeySize + ChildSize;

    InternalNode(void* parent, uint32_t num_keys, uint32_t right_child) :
        Node(parent, Type::INTERNAL), internal_header(num_keys, right_child) {
            Init();
    }

    InternalNode(InternalNode& o) : Node(o),
        internal_header(o.internal_header) {
    }

    InternalNode() = delete;

    ~InternalNode() {
        if (body.buff) {
            free(body.buff);
            body.buff = nullptr;
        }
    }

    void
    Init() {
        if (body.buff) {
            return;
        }
        body.buff = (char*)calloc(BodySize, 1);
    }

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
    Body body;
};

template <int PageSize>
struct LeafNode : public Node {
    struct LeafHeader {
        LeafHeader(uint32_t ksize, uint32_t vsize, uint32_t ncells) :
            key_size(ksize), value_size(vsize), num_cells(ncells) {}

        LeafHeader() = delete;

        uint32_t key_size = 1;
        uint32_t value_size = 1;
        uint32_t num_cells = 0;
    };

    constexpr static const uint32_t BodySize = PageSize - sizeof(Node) - sizeof(LeafHeader);

    LeafNode(void* parent, uint32_t key_size, uint32_t value_size, uint32_t num_cells = 0) :
        Node(parent, Type::LEAF), leaf_header(key_size, value_size, num_cells) {
            Init();
    }

    LeafNode() = delete;

    ~LeafNode() {
        if (leaf_body.buff) {
            free(leaf_body.buff);
            leaf_body.buff = nullptr;
        }
    }

    void
    Init() {
        if (leaf_body.buff) {
            return;
        }
        leaf_body.buff = (char*)calloc(BodySize, 1);
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
        if (!leaf_body.buff) {
            return nullptr;
        }
        auto pos = cell_num * CellSize();
        if (pos + CellSize() <= BodySize) {
            return &leaf_body.buff[pos];
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
    Body leaf_body;
};
#pragma pack(pop)
