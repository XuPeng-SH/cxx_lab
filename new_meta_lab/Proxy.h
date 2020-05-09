#pragma once

template <typename ResourceT>
class ScopedResource {
public:
    using ResourcePtr = typename ResourceT::Ptr;
    ScopedResource(ResourcePtr res);

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
