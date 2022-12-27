#include <stdint.h>
#include <stdio.h>
#include <sys/cachectl.h>
#include <sys/mman.h>
#include <dlfcn.h>


const char SO_PATH[] = "/system/lib/libbinder.so";// the path to the target .so file
const char TARGET_FUNCTION[] = "_ZTv0_n12_N7android15IServiceManagerD0Ev"; // the target function
const uint64_t OFFSET = 0x44abe; // the offset of the target instruction from the beginnning of the target function
const uint64_t PATCHED_INSTRUCTION = 0xbf00bf00; // whatever instructions you want to execute

void dynamic_binary_patching() {
    void *handle = dlopen(SO_PATH, RTLD_NOW);
    if (handle == nullptr) {
        perror("DBP: dlopen target .so (%s) failed!", SO_PATH);
        return;
    }
    void *base_address = nullptr;
    if ((base_address = dlsym(handle, TARGET_FUNCTION)) == nullptr) {
        perror("DBP: dlsym target function (%s) failed!", TARGET_FUNCTION);
        return;
    }
    void *target_address = (void*)(((uint8_t *)base_address) + OFFSET);
    void *page_address = (void*)(((uint64_t)target_address) & (~(0x1000ULL - 1)));
    if (mprotect(page_address, 0x1000, PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
        perror("DBP: mprotect failed!");
        return;
    }

    // here you can inject code to the page target_address is in
    *(uint64_t *)target_address = PATCHED_INSTRUCTION;

    mprotect(page_address, 0x1000, PROT_READ | PROT_EXEC);
    cacheflush(page_address, (page_address + 0x1000));
    
    printf("DBP: patch success!");
}