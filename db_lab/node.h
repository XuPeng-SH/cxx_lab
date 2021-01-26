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

    LeafHeader leaf_header;
    LeafBody leaf_body;
};
#pragma pack(pop)
