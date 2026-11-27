#ifndef __CSJEditor_H__
#define __CSJEditor_H__

#include <vector>
#include <optional>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include "CSJEditorGlobalContext.h"

class CSJEditorUI;

namespace csjruntime {
class CSJEngine;
}

class CSJEditor {
public:
    CSJEditor() = default;
    ~CSJEditor() = default;

    bool initialize(csjruntime::CSJEngine *engine);

    void run();

    void shutdown();

protected:

private:
    csjruntime::CSJEngine *m_pEngine{nullptr};
    std::shared_ptr<CSJEditorUI> m_pEditorUI = nullptr;

    // int m_iEffectType = 0;
    // float m_exposure = 1.0;
    // float m_intensity = 1.0;

};


#endif // __CSJEditor_H__