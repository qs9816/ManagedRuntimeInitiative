// Copyright 2010 Azul Systems, Inc.  All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is free software; you can redistribute it and/or modify it under 
// the terms of the GNU General Public License version 2 only, as published by 
// the Free Software Foundation. 
//
// This code is distributed in the hope that it will be useful, but WITHOUT ANY 
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License version 2 for  more
// details (a copy is included in the LICENSE file that accompanied this code).
//
// You should have received a copy of the GNU General Public License version 2 
// along with this work; if not, write to the Free Software Foundation,Inc., 
// 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
// 
// Please contact Azul Systems, Inc., 1600 Plymouth Street, Mountain View, 
// CA 94043 USA, or visit www.azulsystems.com if you need additional information 
// or have any questions.

/*
 * List of symbols which are redefined with the __ prefix so that they don't collide
 * with the proxied implementation which are defined in libproxy
 */
#define access		__access
#define acct		__acct
#define chdir		__chdir
#define chmod		__chmod
#define chown		__chown
#define chroot		__chroot
#define close		__close
#define dup		__dup
#define dup2		__dup2
#define fchdir		__fchdir
#define fcntl		__fcntl
#define fstatfs		__fstatfs
#define fsync		__fsync
#define getdomain	__getdomain
#define getgid		__getgid
#define getgroups	__getgroups
#define getitimer	__getitimer
#define getpid		__getpid
#define getrlimit	__getrlimit
#define getuid		__getuid
#define ioctl		__ioctl
#define kill		__kill
#define link		__link
#define lseek		__lseek
#define mkdir		__mkdir
#define open		__open
#define profil		__profil
#define ptrace		__ptrace
#define read		__read
#define readlink	__readlink
#define readv		__readv
#define reboot		__reboot
#define rename		__rename
#define rmdir		__rmdir
#define select		__select
#define setdomain	__setdomain
#define setegid		__setegid
#define seteuid		__seteuid
#define setgid		__setgid
#define setgroups	__setgroups
#define setrlimit	__setrlimit
#define setsid		__setsid
#define settimeofday	__settimeofday
#define setuid		__setuid
#define sigsuspend	__sigsuspend
#define sstk		__sstk
#define statfs		__statfs
#define swapoff		__swapoff
#define swapon		__swapon
#define symlink		__symlink
#define sync		__sync
#define sys_fstat	__sys_fstat
#define sys_mknod	__sys_mknod
#define sys_stat	__sys_stat
#define umask		__umask
#define uname		__uname
#define unlink		__unlink
#define utimes		__utimes
#define write		__write
#define writev		__writev
#define adjtime		__adjtime	
#define fchmod		__fchmod	
#define fchown		__fchown	
#define ftruncate	__ftruncate
#define getrusage	__getrusage
#define gettimeofday	__gettimeofday
#define setpgid		__setpgid	
#define setregid	__setregid
#define setreuid	__setreuid
#define sys_lstat	__sys_lstat
#define truncate	__truncate
#define vhangup		__vhangup
#define arch_prctl	__arch_prctl
#define modify_ldt	__modify_ldt
#define msgctl		__msgctl
#define msgget		__msgget
#define msgrcv		__msgrcv
#define msgsnd		__msgsnd
#define accept		__accept	
#define bind		__bind	
#define connect		__connect	
#define gethostid	__gethostid
#define gethostname	__gethostname
#define getpeername	__getpeername
#define getsockname	__getsockname
#define getsockopt	__getsockopt
#define listen		__listen	
#define recv		__recv
#define recvfrom	__recvfrom
#define recvmsg		__recvmsg	
#define sendmsg		__sendmsg	
#define send		__send
#define sendto		__sendto	
#define sethostid	__sethostid
#define sethostname	__sethostname
#define setsockopt	__setsockopt
#define shutdown	__shutdown
#define socket		__socket	
#define socketpair	__socketpair
#define flock		__flock	
#define getdents	__getdents
#define getdtsz		__getdtsz	
#define getpagesize	__getpagesize
#define killpg		__killpg
#define wait3		__wait3
#define waitpid		__waitpid
#define ftime		__ftime	
#define nice		__nice	
#define pause		__pause	
#define poll		__poll	
#define s_getdents	__s_getdents
#define stime		__stime	
#define time		__time	
#define times		__times	
#define ulimit		__ulimit	
#define utime		__utime	
