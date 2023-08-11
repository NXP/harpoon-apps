/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#if defined(__GNUC__)
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

int _close(int fd)
{
	errno = ENOSYS;
	return -1;
}

void _exit(int status)
{
	while (1);
}

pid_t _fork(void)
{
	errno = ENOSYS;
	return -1;
}

int _fstat(int fd, struct stat *statbuf)
{
	errno = ENOSYS;
	return -1;
}

pid_t _getpid(void)
{
	errno = ENOSYS;
	return -1;
}

int _isatty(int fd)
{
	errno = ENOSYS;
	return 0;
}

int _kill(pid_t pid, int sig)
{
	errno = ENOSYS;
	return -1;
}

off_t _lseek(int fd, off_t offset, int whence)
{
	errno = ENOSYS;
	return -1;
}

int _open(const char *pathname, int flags, mode_t mode)
{
	errno = ENOSYS;
	return -1;
}

ssize_t _read(int fd, void *buf, size_t count)
{
	errno = ENOSYS;
	return -1;
}

int _stat(const char *pathname, struct stat *statbuf)
{
	errno = ENOSYS;
	return -1;
}

ssize_t _write(int fd, const void *buf, size_t count)
{
	errno = ENOSYS;
	return -1;
}

#endif /* __GNUC__ */

