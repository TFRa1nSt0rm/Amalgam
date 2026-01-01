#include <Windows.h>
#include "Core/Core.h"
#include "Utils/ExceptionHandler/ExceptionHandler.h"

// Track if we were manually mapped
static bool g_bManualMap = false;
static HMODULE g_hModule = nullptr;

DWORD WINAPI MainThread(LPVOID lpParam)
{
    U::ExceptionHandler.Initialize(lpParam);
    U::Core.Load();
    U::Core.Loop();
    U::ExceptionHandler.Unload();
    U::Core.Unload();
    
    // Only call FreeLibraryAndExitThread if we were loaded normally
    if (!g_bManualMap)
    {
        FreeLibraryAndExitThread(static_cast<HMODULE>(lpParam), EXIT_SUCCESS);
    }
    
    return EXIT_SUCCESS;
}

// Manual map entry point
extern "C" __declspec(dllexport) BOOL WINAPI ManualMapEntry(HMODULE hModule)
{
    g_bManualMap = true;
    g_hModule = hModule;
    
    if (const auto hThread = CreateThread(nullptr, 0, MainThread, hModule, 0, nullptr))
    {
        CloseHandle(hThread);
        return TRUE;
    }
    
    return FALSE;
}

// Standard injection entry point (works for all LoadLibrary variants and Ldr* functions)
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hinstDLL);
        g_hModule = hinstDLL;
        g_bManualMap = false;
        
        if (const auto hThread = CreateThread(nullptr, 0, MainThread, hinstDLL, 0, nullptr))
        {
            CloseHandle(hThread);
        }
    }
    
    return TRUE;
}
