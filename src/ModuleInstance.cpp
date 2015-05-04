
#ifdef _WIN32
    #include <windows.h>
#else // Posix
    #include <dlfcn.h>
#endif

#include <iostream>

#include "ModuleInstance.hpp"

template<typename _RTy>
struct unwrap_function;

template<typename _RTy, typename... _ATy>
struct unwrap_function < std::function<_RTy(_ATy...)> >
{
    typedef _RTy(*function_ptr)(_ATy...);
};

Module ModuleTemplateInstance::CreateInstance() const
{
    return Module(new ModuleInstance(shared_from_this(), _function()));
}

std::string const& ModuleTemplateInstance::GetPlatformSpecificExtension()
{
    #ifdef _WIN32
        static std::string const extension = "dll";
    #else // Posix
        static std::string const extension = "so";
    #endif

    return extension;
}

boost::optional<ModuleTemplate> ModuleTemplateInstance::CreateFromPath(std::string const& path)
{
    #ifdef _WIN32
        HMODULE syshandle = LoadLibrary(path.c_str());
    #else // Posix
        void* syshandle = dlopen(path.c_str(), RTLD_LAZY);
    #endif

    if (!syshandle)
        return boost::none;

    std::cout << "Loaded library " << path << std::endl;

    #ifdef _WIN32
        unwrap_function<ModuleCreateFunction>::function_ptr const function =
            (unwrap_function<ModuleCreateFunction>::function_ptr)GetProcAddress(syshandle, CREATE_MODULE_FUNCTION_NAME);

    #else // Posix

        // Silences "warning: dereferencing type-punned pointer will break strict-aliasing rules" warnings according to:
        // http://en.wikipedia.org/wiki/Dynamic_loading
        // union { unwrap_function<ModuleCreateFunction>::function_ptr function; void* raw; } alias;
        // alias.raw = dlsym(syshandle, CREATE_MODULE_FUNCTION_NAME);
        // unwrap_function<ModuleCreateFunction>::function_ptr function = alias.function;

        unwrap_function<ModuleCreateFunction>::function_ptr const function =
            (unwrap_function<ModuleCreateFunction>::function_ptr)dlsym(syshandle, CREATE_MODULE_FUNCTION_NAME);

    #endif

    if (!function)
        return boost::none;

    std::cout << "Found function " << std::endl;

    return boost::make_optional(ModuleTemplate(
        new ModuleTemplateInstance(InternalHandleType(syshandle, [](void* handle)
        {
            #ifdef _WIN32
                FreeLibrary((HMODULE)handle);
            #else // Posix
                dlclose(handle);
            #endif
        }),
        ModuleCreateFunction(function))));
}
