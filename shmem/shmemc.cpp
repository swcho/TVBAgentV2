#include "shmem.h"
#include "shmemc.h"

SHMEM_DLL_ENTRY shmem_h shmem_open(char const* file_name, char const* shared_name, 
                                   size_t max_size, shmem_open_mode_t mode, void* desired_address)
{
    shared_memory* shmem = new shared_memory();
    if (shmem->open(file_name, shared_name, max_size, (shared_memory::open_mode)mode, desired_address) == shared_memory::ok) {
        return (shmem_h)shmem;
    }
    delete shmem;
    return NULL;
}
        
SHMEM_DLL_ENTRY void* shmem_malloc(shmem_h hnd, size_t size)
{
    return((shared_memory*)hnd)->allocate(size);
}

SHMEM_DLL_ENTRY void* shmem_realloc(shmem_h hnd, void* ptr, size_t size)
{
    return ((shared_memory*)hnd)->reallocate(ptr, size);
}

SHMEM_DLL_ENTRY void shmem_free(void* ptr)
{
    shared_memory* shmem = shared_memory::find_storage(ptr);
    if (shmem != NULL) { 
        shmem->free(ptr);
    } else { 
        free(ptr);
    }    
}

SHMEM_DLL_ENTRY shmem_status_t shmem_close(shmem_h hnd)
{
    ((shared_memory*)hnd)->close();
    delete (shared_memory*)hnd;
    return SHMEM_OK;
}

SHMEM_DLL_ENTRY shmem_status_t shmem_flush(shmem_h hnd)
{
    return (shmem_status_t)((shared_memory*)hnd)->flush();
}

SHMEM_DLL_ENTRY shmem_status_t shmem_lock(shmem_h hnd, shmem_lock_descriptor* desc, unsigned msec)
{
    return (shmem_status_t)((shared_memory*)hnd)->lock(*(shared_memory::lock_descriptor*)desc, msec);
}

SHMEM_DLL_ENTRY shmem_status_t shmem_unlock(shmem_h hnd, shmem_lock_descriptor* desc)
{
    return (shmem_status_t)((shared_memory*)hnd)->unlock(*(shared_memory::lock_descriptor*)desc);
}

SHMEM_DLL_ENTRY void* shmem_get_root(shmem_h hnd)
{
    return ((shared_memory*)hnd)->get_root_object();
}

SHMEM_DLL_ENTRY void shmem_set_root(shmem_h hnd, void* root)
{
    ((shared_memory*)hnd)->set_root_object(root);
}

//-------------------------------------------------------------------*--------*
// event object C interface
//-------------------------------------------------------------------*--------*
SHMEM_DLL_ENTRY event_h event_open(char const* shared_name)
{
    event* pevent = new event();
    if (pevent->open(shared_name)) {
        return (event_h)pevent;
    }
    delete pevent;
    return NULL;
}

SHMEM_DLL_ENTRY bool event_wait(event_h hnd, unsigned msec)
{
	return ((event*)hnd)->wait(msec);
}

SHMEM_DLL_ENTRY void event_signal(event_h hnd)
{
	((event*)hnd)->signal();
}

SHMEM_DLL_ENTRY void event_reset(event_h hnd)
{
	((event*)hnd)->reset();
}

SHMEM_DLL_ENTRY void event_close(event_h hnd)
{
	((event*)hnd)->close();
}

