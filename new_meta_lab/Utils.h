#pragma once
#include <functional>
#include <vector>

using OnDeRefCBF = std::function<bool(void)>;

class ReferenceProxy {
public:
    void RegisterOnDeRefCB(OnDeRefCBF cb);

    virtual void OnDeRefCallBack();

    virtual void Ref();
    virtual void UnRef();

    virtual ~ReferenceProxy();

protected:

    int refcnt_ = 0;
    std::vector<OnDeRefCBF> on_deref_cbs_;
};
