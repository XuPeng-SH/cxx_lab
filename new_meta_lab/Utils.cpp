#include "Utils.h"
#include <assert.h>
#include <iostream>


void ReferenceProxy::Ref() {
    refcnt_ += 1;
}

void ReferenceProxy::UnRef() {
    std::cout << "refcnt_=" << refcnt_ << std::endl;
    if (refcnt_ == 0) return;
    refcnt_ -= 1;
    if (refcnt_ == 0) {
        OnDeRefCallBack();
        for (auto& cb : on_deref_cbs_) {
            cb();
        }
    }
}

void ReferenceProxy::RegisterOnDeRefCB(OnDeRefCBF cb) {
    on_deref_cbs_.emplace_back(cb);
}

void ReferenceProxy::OnDeRefCallBack() {
    return;
}

ReferenceProxy::~ReferenceProxy() {
    /* OnDeRef(); */
}
