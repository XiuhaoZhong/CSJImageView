#pragma once 

namespace csjruntime {

class ICSJWindowUI {
public:
    ICSJWindowUI() = default;
    virtual ~ICSJWindowUI() {}

    virtual bool initialize() = 0;
    virtual void renderUI() = 0;
};

} // namespace csjruntime