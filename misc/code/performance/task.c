/*
 * Copyright (C) 2017  Ricardo Biehl Pasquali <rbpoficial@gmail.com>
 * under the terms of the GNU General Public License (see LICENSE file)
 */

#include <sched.h>
#include <sys/types.h>
#include <unistd.h>

#include "task_cpuset.h" /* yet not implemented */


/*
 * return true if effective user id is 0
 */
static inline int
are_we_root(void)
{
	return (geteuid() == 0);
}

/*
 * I decided to make a function where everything related to the isolation of our
 * task is made  :-)
 */
static int
cpuset_do_stuff(void)
{
	char cpuset_mountpoint[] = "/dev/cpuset";

	if (cpuset_mount(cpuset_mountpoint))
		return -1;

	if (cpuset_create(cpuset_mountpoint, "ipmic_cpuset"))
		return -1;

	/*
	 * many things TODO -- I'm deciding if I should use a script or C code
	 * to do process isolation (script is cpu_bossy.sh)
	 */
}

/*
 * set (in current task) SCHED_FIFO policy with maximum/highest priority
 */
int
task_do_highest_prio(void)
{
	int highest_prio;
	struct sched_param sched_param;
	int err;

	if (!are_we_root())
		return -1;

	/*
	 * get highest priority for SCHED_FIFO policy and decrement it a bit so
	 * we'll not bother the true highest priority tasks :-)
	 */
	highest_prio = sched_get_priority_max(SCHED_FIFO);
	highest_prio -= 1;

	sched_param.sched_priority = highest_prio;
	err = sched_setscheduler(0, SCHED_FIFO, &sched_param);
	if (err)
		return -1;

	return 0;
}

/*
 * isolate task in a single cpu
 */
int
task_do_isolate(void)
{
	/* TODO
	 * separate task-isolation job from task_do_cpu_binding()
	 */
}

/*
 * optimize cpu affinity of current task
 */
int
task_do_cpu_binding(void)
{
	int n_online_cpus;
	cpu_set_t cpu_mask;
	int err;

	if (!are_we_root())
		return -1;

	/*
	 * we need at least two cpus online
	 */
	n_online_cpus = sysconf(_SC_NPROCESSORS_ONLN);
	if (n_online_cpus < 2)
		return -1;

	CPU_ZERO(&cpu_mask);

	// TODO CPU_SET()  this mustn't be in collision with cpuset

	err = sched_setaffinity(0, sizeof(cpu_set_t), &cpu_mask);
	if (err)
		return -1;

	if ( cpuset_do_stuff() ) {
		/* not critical */
		/*
		 * DEBUG: You could use the cpu_bossy.sh script to bind ipmic
		 * process to a cpu ;-)
		 * ---
		 * maybe we could call cpu_bossy.sh ...
		 */
	}

	return 0;
}
