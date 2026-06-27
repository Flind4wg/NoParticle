#include <sys/mman.h>
#include <dlfcn.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

void patch_memory(uintptr_t addr, uint32_t value) {
    uintptr_t page = addr & ~(uintptr_t)(4095);
    mprotect((void*)page, 4096, PROT_READ | PROT_WRITE | PROT_EXEC);
    *(uint32_t*)addr = value;
    __builtin___clear_cache((char*)addr, (char*)addr + 4);
    mprotect((void*)page, 4096, PROT_READ | PROT_EXEC);
}

static void* patch_thread(void*) {
    sleep(3);

    void* handle = dlopen("libminecraftpe.so", RTLD_NOLOAD | RTLD_LAZY);
    if (!handle) return nullptr;

    // Hook ProcessParticleEventRequests - makes it return immediately
    // effectively stopping particle event processing
    void* sym = dlsym(handle, "_Z28ProcessParticleEventRequestsv");
    if (sym) {
        patch_memory((uintptr_t)sym, 0xD65F03C0); // RET
    }

    // Hook EmitParticleSystem - stops new particles from being emitted
    void* sym2 = dlsym(handle, "_Z18EmitParticleSystemv");
    if (sym2) {
        patch_memory((uintptr_t)sym2, 0xD65F03C0); // RET
    }

    return nullptr;
}

extern "C" void _init() {
    pthread_t t;
    pthread_create(&t, nullptr, patch_thread, nullptr);
    pthread_detach(t);
}
