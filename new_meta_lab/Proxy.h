#pragma once
#include <memory>

template <typename ResourceT>
class ScopedResource {
public:
    using ThisT = ScopedResource<ResourceT>;
    using Ptr = std::shared_ptr<ThisT>;
    using ResourcePtr = typename ResourceT::Ptr;
    ScopedResource(ResourcePtr res, bool scoped = true);

    ResourcePtr Get() { return res_; }

    ResourceT operator*() { return *res_; }
    ResourcePtr operator->() { return res_; }

    operator bool () const {
        if (res_) return true;
        else return false;
    }

    ~ScopedResource();

protected:
    ResourcePtr res_;
    bool scoped_;
};


template <typename ResourceT>
ScopedResource<ResourceT>::ScopedResource(ScopedResource<ResourceT>::ResourcePtr res,
        bool scoped) : res_(res), scoped_(scoped) {
    if (scoped) {
        res_->Ref();
    }
}

template <typename ResourceT>
ScopedResource<ResourceT>::~ScopedResource() {
    if (scoped_) {
        res_->UnRef();
    }
}
