/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _SELINUX_POLICYCAP_H_
#define _SELINUX_POLICYCAP_H_

/* Policy capabilities */
enum {
	POLICYDB_CAP_NETPEER,
	POLICYDB_CAP_OPENPERM,
	POLICYDB_CAP_EXTSOCKCLASS,
	POLICYDB_CAP_ALWAYSNETWORK,
	POLICYDB_CAP_CGROUPSECLABEL,
	POLICYDB_CAP_NNP_NOSUID_TRANSITION,
	POLICYDB_CAP_GENFS_SECLABEL_SYMLINKS,
	POLICYDB_CAP_IOCTL_SKIP_CLOEXEC,
	POLICYDB_CAP_USERSPACE_INITIAL_CONTEXT,
	__POLICYDB_CAP_MAX
};
#define POLICYDB_CAP_MAX (__POLICYDB_CAP_MAX - 1)

extern const char *const selinux_policycap_names[__POLICYDB_CAP_MAX];

#endif /* _SELINUX_POLICYCAP_H_ */
