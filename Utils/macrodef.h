#pragma once

#include <ctime>

#define VTK_SP(type, name)\
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

#define GFE_Get(name, type) \
    inline type Get##name(){ \
        return name; \
}

#define GFE_GetObject(name, type) \
    inline type Get##name(){ \
        return m_##name; \
}

#ifdef __GNUC__
    #define FuncSig __PRETTY_FUNCTION__
#elif defined _MSC_VER
    #define FuncSig __FUNCSIG__
#endif

#define TimerMacro(description, id, code) \
    auto TimerMacro_start_##id = clock(); \
    code \
    printf(description": %ld\n", clock()-TimerMacro_start_##id);

#ifdef GFE_DEBUG_MODE
#define DebugInfo(desc, ...) \
    printf("%s: " desc "\n", __FUNCTION__, __VA_ARGS__);
#else
#define DebugInfo(desc, ...)
#endif
