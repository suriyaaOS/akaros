// System call stubs.
#ifdef __DEPUTY__
#pragma nodeputy
#endif

#include <arch/types.h>
#include <arch/arch.h>
#include <ros/syscall.h>
#include <lib.h>

intreg_t syscall(uint16_t num, intreg_t a1,
                intreg_t a2, intreg_t a3,
                intreg_t a4, intreg_t a5);

static error_t async_syscall(syscall_req_t* req, syscall_desc_t* desc)
{
	// Note that this assumes one global frontring (TODO)
	// abort if there is no room for our request.  ring size is currently 64.
	// we could spin til it's free, but that could deadlock if this same thread
	// is supposed to consume the requests it is waiting on later.
	if (RING_FULL(&sysfrontring))
		return -EBUSY;
	// req_prod_pvt comes in as the previously produced item.  need to
	// increment to the next available spot, which is the one we'll work on.
	// at some point, we need to listen for the responses.
	desc->idx = ++(sysfrontring.req_prod_pvt);
	desc->sysfr = &sysfrontring;
	syscall_req_t* r = RING_GET_REQUEST(&sysfrontring, desc->idx);
	memcpy(r, req, sizeof(syscall_req_t));
	// push our updates to sysfrontring.req_prod_pvt
	RING_PUSH_REQUESTS(&sysfrontring);
	//cprintf("DEBUG: sring->req_prod: %d, sring->rsp_prod: %d\n", \
	        sysfrontring.sring->req_prod, sysfrontring.sring->rsp_prod);
	return 0;
}

// consider a timeout too
error_t waiton_syscall(syscall_desc_t* desc, syscall_rsp_t* rsp)
{
	// Make sure we were given a desc with a non-NULL frontring.  This could
	// happen if someone forgot to check the error code on the paired syscall.
	if (!desc->sysfr)
		return -EFAIL;
	// this forces us to call wait in the order in which the syscalls are made.
	if (desc->idx != desc->sysfr->rsp_cons + 1)
		return -EDEADLOCK;
	while (!(RING_HAS_UNCONSUMED_RESPONSES(desc->sysfr)))
		cpu_relax();
	memcpy(rsp, RING_GET_RESPONSE(desc->sysfr, desc->idx), sizeof(*rsp));
	desc->sysfr->rsp_cons++;
    // run a cleanup function for this desc, if available
    if (desc->cleanup)
    	desc->cleanup(desc->data);
	return 0;
}

void sys_null()
{
	syscall(SYS_null,0,0,0,0,0);
}

error_t sys_null_async(syscall_desc_t* desc)
{
	syscall_req_t syscall = {SYS_null, 0, {[0 ... (NUM_SYSCALL_ARGS-1)] 0}};
	desc->cleanup = NULL;
	desc->data = NULL;
	return async_syscall(&syscall, desc);
}

void sys_cache_buster(uint32_t num_writes, uint32_t num_pages, uint32_t flags)
{
	syscall(SYS_cache_buster, num_writes, num_pages, flags, 0, 0);
}

error_t	sys_cache_buster_async(syscall_desc_t* desc, uint32_t num_writes,
			                       uint32_t num_pages, uint32_t flags)
{
	syscall_req_t syscall = {SYS_cache_buster, 0, {num_writes, num_pages, flags,
	                                               [3 ... (NUM_SYSCALL_ARGS-1)] 0}};
	// just to be safe, 0 these out.  they should have been 0'd right after
	// the desc was POOL_GET'd
	desc->cleanup = NULL;
	desc->data = NULL;
	return async_syscall(&syscall, desc);
}

void sys_cache_invalidate()
{
	syscall(SYS_cache_invalidate, 0, 0, 0, 0, 0);
}

ssize_t sys_cputs(const char *s, size_t len)
{
	return syscall(SYS_cputs, (uintreg_t)s,  len, 0, 0, 0);
}

error_t sys_cputs_async(const char *s, size_t len, syscall_desc_t* desc,
                        void (*cleanup_handler)(void*), void* cleanup_data)
{
	// could just hardcode 4 0's, will eventually wrap this marshaller anyway
	syscall_req_t syscall = {SYS_cputs, 0, {(uint32_t)s, len, [2 ... (NUM_SYSCALL_ARGS-1)] 0} };
	desc->cleanup = cleanup_handler;
	desc->data = cleanup_data;
	return async_syscall(&syscall, desc);
}

uint16_t sys_cgetc(void)
{
	return syscall(SYS_cgetc, 0, 0, 0, 0, 0);
}

size_t sys_getcpuid(void)
{
	return syscall(SYS_getcpuid, 0, 0, 0, 0, 0);
}

int sys_getpid(void)
{
	return syscall(SYS_getpid, 0, 0, 0, 0, 0);
}

error_t sys_proc_destroy(int pid)
{
	return syscall(SYS_proc_destroy, pid, 0, 0, 0, 0);
}

void sys_yield(void)
{
	syscall(SYS_yield, 0, 0, 0, 0, 0);
	return;
}

int sys_proc_create(char* path)
{
	return syscall(SYS_proc_create, (uintreg_t)path, 0, 0, 0, 0);
}

error_t sys_proc_run(int pid)
{
	return syscall(SYS_proc_run, pid, 0, 0, 0, 0);
}

