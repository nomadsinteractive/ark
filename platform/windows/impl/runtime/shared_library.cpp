#include <windows.h>
#include <stdio.h>

BOOL WINAPI DllMain(_In_ HINSTANCE hinstDLL, _In_ DWORD fdwReason, _In_ LPVOID /*lpvReserved*/)
{
    //  Perform global initialization.
    switch(fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            //  For optimization.
            DisableThreadLibraryCalls(hinstDLL);
            break;

        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}
