#pragma once
#include <memory>

template <typename ResourceT>
class ScopedResource {
public:
    using ThisT = ScopedResource<ResourceT>;
    using Ptr = std::shared_ptr<ThisT>;
    using ResourcePtr = typename ResourceT::Ptr;
    ScopedResource(ResourcePtr res);

    ResourcePtr Get() { return res_; }

    ~ScopedResource();

protected:
    ResourcePtr res_;
};


template <typename ResourceT>
ScopedResource<ResourceT>::ScopedResource(ScopedResource<ResourceT>::ResourcePtr res) : res_(res) {
    res_->Ref();
}

template <typename ResourceT>
ScopedResource<ResourceT>::~ScopedResource() {
    res_->UnRef();
}
