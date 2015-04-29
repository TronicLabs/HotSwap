
#include <iostream>

#include <boost/filesystem.hpp>

#include "ModuleInstance.hpp"
#include "ExampleModule.hpp"
#include "ExampleSingleton.hpp"

#include <windows.h>

typedef ModuleInstance*(*CreateModule)();

int main(int argc, char** argv)
{
    Log::instance().count();

    CreateModule function;
    BOOL fFreeResult;

    HINSTANCE moduleHandle = LoadLibrary("MyModule.dll");

    if (moduleHandle)
    {
        function = (CreateModule) GetProcAddress(moduleHandle, "CreateModule");

        if (function != NULL)
        {
            std::unique_ptr<ModuleInstance> module((*function)());

            std::cout << module->GetName() << std::endl;

            std::cout << module->GetInterface<Example>()->GetHey() << std::endl;
        }
        else
            std::cout << "function not found!";

        fFreeResult = FreeLibrary(moduleHandle) ;
    }
    else
        std::cout << "could not load library";

    return 0;
}