
#define _GNU_SOURCE

#include <err.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// for vmlinuz_3_11_10_301_fc20_x86_64_bare
#include "bare.h"
// for vmlinuz_3_11_10_301_fc20_x86_64_signed
#include "signed.h"

#define ALIGNMENT_PADDING(address, align) ((align - (address % align)) % align)

int
main(int argc, char *argv[])
{
	int fd;
	int rc;
	size_t in_size = sizeof (vmlinuz_3_11_10_301_fc20_x86_64_bare);

	if (argc != 2)
		errx(1, "Usage: harness <output_file>");

	// open("/home/pjones/ext3/vmlinuz-3.11.10-301.fc20.x86_64", O_RDWR|O_CREAT|O_TRUNC|O_CLOEXEC, 0100755) = 6
	fd = open(argv[1], O_RDWR|O_CREAT|O_TRUNC|O_CLOEXEC, 0755);
	if (fd < 0)
		err(1, "Couldn't open \"%s\"", argv[1]);

	// ftruncate(6, 5136912)                   = 0
	rc = ftruncate(fd, in_size);
	if (rc < 0)
		err(1, "ftruncate");

	// lseek(6, 0, SEEK_SET)                   = 0
	off_t loc;
	loc = lseek(fd, 0, SEEK_SET);
	if (loc < 0)
		err(1, "lseek");
	else if (loc != 0)
		errx(1, "lseek returned wrong value: %zd instead of 0", loc);

	// write(6, "MZ\352\7\0\300\7"..., 5136912) = 5136912
	rc = write(fd, vmlinuz_3_11_10_301_fc20_x86_64_bare, in_size);
	if (rc < 0)
		err(1, "write");
	else if (rc != in_size)
		err(1, "write didn't complete: %d instead of %zd", rc, in_size);

	// fcntl(6, F_GETFL)                       = 0x8002 (flags O_RDWR|O_LARGEFILE)
	rc = fcntl(fd, F_GETFL);
	if (rc != (O_RDWR|O_LARGEFILE | 0x8002))
		warnx("fcntl has wrong value: %08x instead of %08x\n", rc, O_RDWR|O_LARGEFILE|0x8002);

	// fstat(6, {st_mode=S_IFREG|0755, st_size=5136912, ...}) = 0
	struct stat statbuf;
	rc = fstat(fd, &statbuf);
	if (rc < 0)
		err(1, "fstat");

	// mmap(NULL, 5136912, PROT_READ|PROT_WRITE, MAP_SHARED, 6, 0) = 0x7f5d83175000
	void *addr;
	addr = mmap(NULL, in_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if (addr == MAP_FAILED)
		err(1, "mmap");

	// mremap(0x7f5d83175000, 5136912, 5136912, 0) = 0x7f5d83175000
	void *addr2;
	addr2 = mremap(addr, in_size, in_size, 0);
	if (addr2 == MAP_FAILED)
		err(1, "mremap");
	addr = addr2;

	// ftruncate(6, 5136912)                   = 0
	rc = ftruncate(fd, in_size);
	if (rc < 0)
		err(1, "ftruncate");

	// ftruncate(6, 5139720)                   = 0
	size_t out_size = sizeof (vmlinuz_3_11_10_301_fc20_x86_64_signed);
	rc = ftruncate(fd, out_size);
	if (rc < 0)
		err(1, "ftruncate");

	// mremap(0x7f5d83175000, 5136912, 5139720, MREMAP_MAYMOVE) = 0x7f5d83175000
	addr2 = mremap(addr, in_size, out_size, MREMAP_MAYMOVE);
	if (addr2 == MAP_FAILED)
		err(1, "mremap");
	addr = addr2;

	// mremap(0x7f5d83175000, 5139720, 5136912, 0) = 0x7f5d83175000
	addr2 = mremap(addr, out_size, in_size, 0);
	if (addr2 == MAP_FAILED)
		err(1, "mremap");
	addr = addr2;

	// ftruncate(6, 5136912)                   = 0
	rc = ftruncate(fd, in_size);
	if (rc < 0)
		err(1, "ftruncate");

	// ftruncate(6, 5139720)                   = 0
	rc = ftruncate(fd, out_size);
	if (rc < 0)
		err(1, "ftruncate");

	// mremap(0x7f5d83175000, 5136912, 5139720, MREMAP_MAYMOVE) = 0x7f5d83175000
	addr2 = mremap(addr, in_size, out_size, MREMAP_MAYMOVE);
	if (addr2 == MAP_FAILED)
		err(1, "mremap");
	addr = addr2;

	// not in strace
	void *mem0 = (void *)((intptr_t)addr + 0x164);
	void *mem1 = (void *)((intptr_t)vmlinuz_3_11_10_301_fc20_x86_64_signed + 0x164);
	memcpy(mem0, mem1, 4);

	mem0 = (void *)((intptr_t)addr + 0x4e6210);
	mem1 = (void *)((intptr_t)vmlinuz_3_11_10_301_fc20_x86_64_signed + 0x4e6210);
	memcpy(mem0, mem1, 0xaf8);

	// ftruncate(6, 5140480)                   = 0
	size_t page_size = sysconf(_SC_PAGESIZE);
	size_t big_out_size = out_size + ALIGNMENT_PADDING(out_size, page_size);
	rc = ftruncate(fd, big_out_size);
	if (rc < 0)
		err(1, "ftruncate");

	// mremap(0x7f5d83175000, 5139720, 5140480, MREMAP_MAYMOVE) = 0x7f5d83175000
	addr2 = mremap(addr, out_size, big_out_size, MREMAP_MAYMOVE);
	if (addr2 == MAP_FAILED)
		err(1, "mremap");
	addr = addr2;

	// msync(0x7f5d83175000, 5140480, MS_SYNC) = 0
	rc = msync(addr, big_out_size, MS_SYNC);
	if (rc < 0)
		err(1, "msync");

	// mremap(0x7f5d83175000, 5140480, 5139720, 0) = 0x7f5d83175000
	addr2 = mremap(addr, big_out_size, out_size, 0);
	if (addr2 == MAP_FAILED)
		err(1, "mremap");
	addr = addr2;

	// ftruncate(6, 5139720)                   = 0
	rc = ftruncate(fd, out_size);
	if (rc < 0)
		err(1, "ftruncate");

	// munmap(0x7f5d83175000, 5139720)         = 0
	rc = munmap(addr, out_size);
	if (rc < 0)
		err(1, "munmap");

	// close(6)                                = 0
	rc = close(fd);
	if (rc < 0)
		err(1, "close");

	exit(0);
}
