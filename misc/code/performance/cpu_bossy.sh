#!/bin/bash
# Copyright (C) 2017  Ricardo Biehl Pasquali <rbpoficial@gmail.com>
# under the terms of the GNU General Public License (see LICENSE file)

# put all processes except victim_pid (pid argument to this script) to run in a
# different cpu  --  victim_pid is bossy of cpu :-)
#
# about cpuset mountpoint:
#
#   cpuset_mountpoint/
#       tasks             processes/tasks belonging to this (main) cpuset
#       cpuset.cpus       list of cpus where tasks in this (main) cpuset can run
#       ...               another (now irrelevant) files in this (main) cpuset
#       foobar_cpuset/    directory containing other cpuset
#           tasks
#           cpuset.cpus
#           ...
#
# NOTES:
# -> a process can belong to only one cpuset at a time
# -> cpusets have higher precedence compared to sched_setaffinity, mbind etc.
#    (it means cpuset placement is always enforced)
# -> for further details, read `man 7 cpuset`

# global
unset victim_pid
unset victim_cpu

victim_cpuset="ipmic_cpuset"
other_cpuset="other_cpuset"

opt_undo=0


# functions ...

# try to find a mounted cpuset and `cd` into it
#   usage: func
function cpuset_do_access_mountpoint ()
{
	local mountpoint="$(grep -m 1 cpuset /proc/mounts | cut -d " " -f 2)"

	if [[ -z "$mountpoint" ]]; then
		return 1
	fi

	cd "$mountpoint"
	return
}

# mount a cpuset and `cd` into it
#   usage: func
function cpuset_do_mount_and_access ()
{
	local mountpoint="/dev/cpuset"

	mkdir "$mountpoint"
	mount -t cpuset cpuset "$mountpoint"
	if (( $? != 0 )); then
		rmdir "$mountpoint"
		return 1
	fi

	cd "$mountpoint"
	return
}

# transfer all tasks from one cpuset to another
#   usage: func <dst_cpuset> <src_cpuset>
function cpuset_do_transfer_tasks ()
{
	local cpuset_dst="$1"
	local cpuset_src="$2"
	local n_errors=0
	local task

	for task in `cat "./${cpuset_src}/tasks"`; do
		/bin/echo "$task" > "./${cpuset_dst}/tasks"
		if (( $? != 0 )); then
			let n_errors++
		fi
	done

	return $n_errors
}

# remove a cpuset
#   usage: func <cpuset>
function cpuset_do_remove ()
{
	local cpuset_to_remove="$1"

	if [[ ! -d "./$cpuset_to_remove" ]]; then
		return 1
	fi

	cpuset_do_transfer_tasks "." "$cpuset_to_remove"
	if (( $? != 0)); then
		return 1
	fi

	rmdir "$cpuset_to_remove"
	return
}

# create a cpuset
#   usage: func <cpuset>
function cpuset_do_create ()
{
	local cpuset_to_add="$1"

	if [[ -d "./$cpuset_to_add" ]]; then
		# cpuset already exists
		return 1
	fi

	mkdir "$cpuset_to_add"
	return
}

# pick up a cpu and store it in `victim_cpu`
#   usage: func
function cpuset_do_pick_up_cpu ()
{
	local sys_cpu_path="/sys/devices/system/cpu"
	local n_online_cpus="$(getconf _NPROCESSORS_ONLN)"
	local last_cpu="$(( n_online_cpus - 1 ))"
	# cpus allowed in the main cpuset
	local curr_cpus_allowed="$(cat "./cpuset.cpus")"
	local tmp_cpu

	if (( $n_online_cpus < 2 )); then
		return 1
	fi

	for (( tmp_cpu = last_cpu; tmp_cpu != 0; tmp_cpu-- )); do
		if (( $(cat "${sys_cpu_path}/cpu${tmp_cpu}/online") == 1 )) &&
		   [[ "$curr_cpus_allowed" == *${tmp_cpu}* ]]; then
			victim_cpu="$tmp_cpu"
			break
		fi
	done

	if (( $victim_cpu == 0 )); then
		return 1
	fi

	return 0
}

# check if pid is valid and active
#   usage: func <pid>
function cpuset_do_check_pid ()
{
	local tmp_pid="$1"

	if [[ ! ( "$tmp_pid" =~ ^[0-9]+$ ) ]] ||
	   [[ ! -d "/proc/$tmp_pid" ]]; then
		return 1
	fi

	return 0
}


#
# main()  :-)
#

case "$1" in
"-h"|"--help")
	echo "put all processes except pid argument to this"
	echo "script to run in a different cpu"
;&
"")
	echo
	echo "  usage: cmd [<pid>|--help|--undo]"
	echo
	echo "<pid>       a valid process id"
	echo "-h|--help   print help"
	echo "-u|--undo   undo changes made by this script"
	exit 0
;;
"-u"|"--undo")
	opt_undo=1
;;
*)
	victim_pid="$1"
;;
esac

# make sure we are root
if (( $EUID != 0 )); then
	echo "you must be root (uid 0) to run this, exiting"
	exit 1
fi

# check if system support cpusets
if ! grep "cpuset" /proc/filesystems >/dev/null; then
	echo "cpusets aren't supported, exiting"
	exit 1
fi

# access cpuset mountpoint
if ! cpuset_do_access_mountpoint && ! cpuset_do_mount_and_access; then
	echo "we couldn't access nor mount cpuset, exiting"
	exit 1
fi

# if user has specified  --undo  option
if (( $opt_undo == 1 )); then
	cpuset_do_remove "$other_cpuset"
	cpuset_do_remove "$ipmic_cpuset"
	exit 0
fi

# pick up a cpu
if ! cpuset_do_pick_up_cpu; then
	echo "we couldn't pick up a cpu for victim pid, exiting"
	exit 1
fi

# check if victim_pid is valid
if ! cpuset_do_check_pid "$victim_pid"; then
	echo "your pid is invalid, exiting"
	exit 1
fi

# create cpuset for victim
if ! cpuset_do_create "$victim_cpuset"; then
	echo "error while creating victim cpuset, exiting"
	exit 1
fi

if ! cat "./cpuset.mems" > "./${victim_cpuset}/cpuset.mems" ||
   ! echo "$victim_cpu" > "./${victim_cpuset}/cpuset.cpus" ||
   ! echo "$victim_pid" > "./${victim_cpuset}/tasks"; then
	echo "error while configuring victim cpuset, exiting"
	cpuset_do_remove "$victim_cpuset"
	exit 1
fi

# create cpuset for other tasks
if ! cpuset_do_create "$other_cpuset"; then
	echo "error while creating other cpuset, exiting"
	cpuset_do_remove "$victim_cpuset"
	exit 1
fi

cat "./cpuset.mems" > "./${other_cpuset}/cpuset.mems"

if (( $victim_cpu == 1 )); then
	# only cpu 0 available for other tasks
	echo "0" > "./${other_cpuset}/cpuset.cpus"
else
	# we have a range of cpus available for other tasks
	echo "0-$(( $victim_cpu - 1 ))" > "./${other_cpuset}/cpuset.cpus"
fi

if ! cpuset_do_transfer_tasks "$other_cpuset" "."; then
	echo "error while transferring tasks to other cpuset, exiting"
	cpuset_do_remove "$victim_cpuset"
	cpuset_do_remove "$other_cpuset"
fi

# We are ready ;-)
