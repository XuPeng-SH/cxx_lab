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

    Node(void* p, Type t = Type::NONE) : header(p, t) {}

    struct NodeHeader {
        NodeHeader(void* p, Type t) : parent(p), type(t) {}
        void* parent = nullptr;
        Type type = Type::NONE;
    };

    NodeHeader header;
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

    struct LeafBody {
        char* buff = nullptr;
    };

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
    LeafBody leaf_body;
};
#pragma pack(pop)