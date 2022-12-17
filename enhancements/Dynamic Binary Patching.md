# Dynamic Binary Patching

To reduce false negative test results, we devise *dynamic binary patching* for quickly prototyping and deploying our proposed fixes to the design defects and implementation bugs inside certain vendors’ proprietary system components.

The source code of this enhancement is provided below.

```c++
const char SO_PATH[] = "the path to the target .so file";
const char TARGET_FUNCTION[] = "the target function";
const u_int64_t OFFSET = 0x0ULL; // the offset of the target instruction from the beginnning of the target function
const u_int64_t PATCHED_INSTRUCTION = 0x0ULL; // whatever instructions you want to execute

void dynamic_binary_patching() {
    void *handle = dlopen(SO_PATH, RTLD_NOW);
    if (handle == nullptr) {
        LOGE("DBP: dlopen target .so (%s) failed!", SO_PATH);
        return;
    }
    void *base_address = nullptr;
    if ((base_address = dlsym(handle, TARGET_FUNCTION)) == nullptr) {
        LOGE("DBP: dlsym target function (%s) failed!", TARGET_FUNCTION);
        return;
    }
    void *target_address = (void*)(((u_int8_t *)base_address) + OFFSET);
    void *page_address = (void*)(((u_int64_t)target_address) & (~(0x1000ULL - 1)));
    if (mprotect(page_address, 0x1000, PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
        LOGE("DBP: mprotect failed!");
        return;
    }

    // here you can inject code to the page target_address is in
    *(u_int64_t *)target_address = PATCHED_INSTRUCTION;

    mprotect(page_address, 0x1000, PROT_READ | PROT_EXEC);
    cacheflush(page_address, (page_address + 0x1000));
    
    LOGI("DBP: patch success!");
}
```