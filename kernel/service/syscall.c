#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/mman.h>
#include "service/hypercall.h"
#include "memory/usermem.h"
#include "utils/misc.h"

#define NR_read         0
#define NR_write        1

#define NR_mmap         9
#define NR_mprotect     10
#define NR_munmap       11
#define NR_brk          12

#define NR_exit         60

#define NR_getflag      4296

ssize_t sys_read(int fd, uint64_t buf, size_t count);
ssize_t sys_write(int fd, uint64_t buf, size_t count);
uint64_t sys_mmap(uint64_t addr, size_t length, int prot, int flags, int fd, off_t offset);
uint64_t sys_mprotect(uint64_t addr, size_t length, int prot);
int sys_munmap(uint64_t addr, size_t length);
int sys_brk(uint64_t addr);
void sys_exit(int status);
uint64_t sys_getflag(void);

uint64_t syscall(uint64_t nr, uint64_t argv[]){
	uint64_t ret = -1;

	switch(nr){
		case NR_read:
			ret = sys_read(argv[0], argv[1], argv[2]);
			break;
		case NR_write:
			ret = sys_write(argv[0], argv[1], argv[2]);
			break;
		case NR_mmap:
			ret = sys_mmap(argv[0], argv[1], argv[2], argv[3], argv[4], argv[5]);
			break;
		case NR_mprotect:
			ret = sys_mprotect(argv[0], argv[1], argv[2]);
			break;
		case NR_munmap:
			ret = sys_munmap(argv[0], argv[1]);
			break;
		case NR_brk:
			ret = sys_brk(argv[0]);
			break;
		case NR_exit:
			sys_exit(argv[0]);
			break;
		case NR_getflag:
			ret = sys_getflag();
			break;
	}

	return ret;
}

ssize_t sys_read(int fd, uint64_t buf, size_t count){
	return hc_read((void*)buf, count, 1);
}

ssize_t sys_write(int fd, uint64_t buf, size_t count){
	return hc_write((void*)buf, count, 1);
}

uint64_t sys_mmap(uint64_t addr, size_t length, int prot, int flags, int fd, off_t offset){
	return mmap_user(addr, length, prot);
}

uint64_t sys_mprotect(uint64_t addr, size_t length, int prot){
	return mprotect_user(addr, length, prot);
}

int sys_munmap(uint64_t addr, size_t length){
	return munmap_user(addr, length);
}

int sys_brk(uint64_t addr){
	if(!addr)
		return brk;

	if(addr > brk){
		if(mmap_user(brk, addr-brk, PROT_READ | PROT_WRITE) < 0)
			return -1;
	}
	else{
		if(munmap_user(addr, brk-addr) < 0)
			return -1;
	}

	brk = addr;
	return 0;
}

void sys_exit(int status){
	hlt();
}

uint64_t sys_getflag(void){
	uint64_t addr;
	char flag[] = "Here is first flag : "FLAG1;

	addr = mmap_user(0, 0x1000, PROT_WRITE);
	copy_to_user(addr, flag, sizeof(flag));
	mprotect_user(addr, 0x1000, PROT_NONE);

	return addr;
}
