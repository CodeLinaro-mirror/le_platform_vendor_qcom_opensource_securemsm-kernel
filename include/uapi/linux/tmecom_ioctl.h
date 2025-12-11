/* SPDX-License-Identifier: GPL-2.0-only WITH Linux-syscall-note */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef _UAPI_TMECOM_IOCTL_H_
#define _UAPI_TMECOM_IOCTL_H_

#include <linux/types.h>
#include <linux/ioctl.h>

#define TMECOM_IOCTL_BASE      'T'

/**
 * tmecom_ioctl - Handle TME fuse read/write IOCTLs.
 */
long tmecom_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

/**
 * TMECOM IOCTLs: QFPROM fuse read/write via HLOS TMECOM DRIVER.
 *
 * IOCTL semantics:
 * - TMECOM_IOCTL_FUSE_READ: _IOWR; user fills addr_type and fuse_addr; kernel
 *   returns fuse_data, qfprom_api_status, and ret.
 * - TMECOM_IOCTL_FUSE_WRITE: _IOWR; user provides an array of fuses to blow
 *   and fuse_array_len; kernel returns qfprom_api_status and ret.
 */
#define TMECOM_IOCTL_FUSE_READ \
	_IOWR(TMECOM_IOCTL_BASE, 1, struct tme_ioctl_fuse_read)
#define TMECOM_IOCTL_FUSE_WRITE \
	_IOWR(TMECOM_IOCTL_BASE, 2, struct tme_ioctl_fuse_write_multiple)

#define TME_MAX_FUSE_WRITE_REQ 64

/**
 * struct tme_ioctl_fuse_read - Read a single QFPROM fuse row
 * @addr_type:            QFPROM address space selector (see TMEQFPROMAddrSpace_t).
 *                        Selects the logical address space used by firmware
 *                        (e.g., raw vs. corrected address space).
 * @fuse_addr:            Address of the fuse to be read
 * @fuse_data:            Out: 64-bit fuse contents split into two 32-bit words.
 *                        fuse_data[0] holds the lower 32 bits (LSB),
 *                        fuse_data[1] holds the upper 32 bits (MSB).
 * @qfprom_api_status:    Result of the QFPROM read operation
 * @ret:                  Out: ioctl return code 0 on success; negative no on failure.
 */
struct tme_ioctl_fuse_read {
	uint32_t addr_type;
	uint32_t fuse_addr;
	uint32_t fuse_data[2];
	uint32_t qfprom_api_status;
	int ret;
};

/**
 * struct TMEFuse_t - Fuse write descriptor
 * @addr:     Address of fuse to be written
 *            within the selected QFPROM address spac).
 * @data:     64-bit value to write, split into two 32-bit words.
 *            data[0] holds the lower 32 bits (LSB); data[1] the upper 32 bits (MSB).
 */
struct TMEFuse_t {
	uint32_t addr;
	uint32_t data[2];
} __packed;

/**
 * struct tme_ioctl_fuse_write_multiple - Write multiple QFPROM fuses
 * @fuse_array:         In: packed array of fuses to blow. Each entry specifies
 *                      a target row address and a 64-bit value (LSB/MSB).
 * @fuse_array_len:     In: number of valid entries in fuse_array.
 *                      Must be > 0 and <= TME_MAX_FUSE_WRITE_REQ.
 * @qfprom_api_status:  Out: QFPROM API status returned by TME.
 *                      0 indicates success; non-zero conveys error.
 * @ret:                Out: ioctl return code 0 on success; negative no on failure.
 */
struct tme_ioctl_fuse_write_multiple {
	struct TMEFuse_t fuse_array[TME_MAX_FUSE_WRITE_REQ];
	uint32_t  fuse_array_len;
	uint32_t  qfprom_api_status;
	int ret;
};
#endif
