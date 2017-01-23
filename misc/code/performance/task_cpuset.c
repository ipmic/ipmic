/*
 * Copyright (C) 2017  Ricardo Biehl Pasquali <rbpoficial@gmail.com>
 * under the terms of the GNU General Public License (see LICENSE file)
 */

#include <fcntl.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static int
cpuset_mount(char *mountpoint)
{
	struct stat finfo;
	int err;

	err = stat(mountpoint, &finfo);
	if (err) {
		/* only if  errno == ENOENT  we try to create the directory */
		if (errno != ENOENT  ||  mkdir(mountpoint, 0555))
			return -1;
	} else if (! S_ISDIR(finfo.st_mode))
		return -1;

	err = mount("cpuset", mountpoint, "cpuset", MS_MGC_VAL, NULL);
	if (err)
		return -1;

	return 0;
}

static int
cpuset_create(void)
{
	int fd;

	fd = open();
}

// TODO
