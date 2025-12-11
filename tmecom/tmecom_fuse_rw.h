/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */
#ifndef _TMECOM_FUSE_RW_H_
#define _TMECOM_FUSE_RW_H_

#include "linux/tmecom_ioctl.h"

/**
 * Opcode and response structures
 */

/**
 * TMEQFPROMAddrSpace_t - Address space selector for QFPROM accesses
 * @TME_QFPROM_ADDR_SPACE_RAW:      Access the raw (uncorrected) address space.
 *                                  This returns the physical fuse word(s) as stored.
 * @TME_QFPROM_ADDR_SPACE_CORR:     Access the corrected address space.
 *                                  This applies ECC/correction as defined by QFPROM controller.
 * @TME_QFPROM_ADDR_SPACE_MAX:      Sentinel value to keep the enum 32-bit wide; not a valid input.
 *
 * Selects whether a read/write targets the raw or corrected address interpretation
 * used by the QFPROM controller.
 */
enum TMEQFPROMAddrSpace_t {
	TME_QFPROM_ADDR_SPACE_RAW = 0,
	TME_QFPROM_ADDR_SPACE_CORR,
	TME_QFPROM_ADDR_SPACE_MAX = 0x7FFFFFFF,
};

/**
 * tmeFuseWriteMultipleReq_t - Request payload for multi-fuse write
 * @cbor_header:    CBOR-encoded semantic tag for "Fuse Write Multiple" request.
 *                  it is 6 bytes and matches the firmware tag sequence.
 *                  Caller should not modify; it is populated by the helper before send.
 * @fuseArray:      Array of fuse descriptors to write. Each entry provides:
 *                  - addr: 32-bit QFPROM fuse address
 *                  - data[0..1]: 64-bit value split into two 32-bit words
 * @fuseArrayLen:   Number of valid entries in fuseArray. Must be >0 and <= TME_MAX_FUSE_WRITE_REQ.
 * Packed to match the firmware. The helper will serialize this and send via TME transport(QMP).
 */
struct tmeFuseWriteMultipleReq_t {
	uint8_t  cbor_header[6];
	struct TMEFuse_t fuseArray[TME_MAX_FUSE_WRITE_REQ];
	uint32_t  fuseArrayLen;
} __packed;

/**
 * tmeFuseWriteMultipleRsp_t - Response payload for multi-fuse write
 * @status:       Operation status returned by firmware.
 *                0 indicates success; non-zero indicates failure.
 * @fuseAddrErr:  Fuse address associated with an error (if applicable).
 *                Valid only when status indicates failure.
 */
struct tmeFuseWriteMultipleRsp_t {
	uint32_t status;
	uint32_t fuseAddrErr;
} __packed;

/**
 * tmeFuseReadReq_t - Request payload for single-fuse read
 * @cbor_header:  CBOR-encoded semantic tag for "Fuse Read" request.
 *                4-byte tag constant defined by the TME firmware.
 * @addrType:     Address space selector: see TMEQFPROMAddrSpace_t.
 * @fuseAddr:     QFPROM fuse address to read.
 */
struct tmeFuseReadReq_t {
	uint32_t cbor_header;
	uint32_t addrType;
	uint32_t fuseAddr;
} __packed;

/**
 * tmeFuseReadRsp_t - Response payload for single-fuse read
 * @status:          Operation status from firmware.
 *                   0 indicates success; non-zero indicates failure.
 * @fuseData:        Two 32-bit words representing the 64-bit fuse data read.
 *                   Conventionally:
 *                   - fuseData[0]: lower 32 bits (LSW)
 *                   - fuseData[1]: upper 32 bits (MSW)
 * @qfpromApiStatus: QFPROM driver/controller status code for diagnostic purposes.
 */
struct tmeFuseReadRsp_t {
	uint32_t status;
	uint32_t fuseData[2];
	uint32_t qfpromApiStatus;
} __packed;

/**
 * Tme_FuseRead - Read a single QFPROM fuse via TME transport
 * @addrType:         Address space selection (raw vs corrected).
 * @fuseAddr:         Fuse address to read.
 * @fuseData:         Output pointer to two 32-bit words receiving the 64-bit fuse value.
 *                    On success, fuseData[0] is the lower word, fuseData[1] is the upper word.
 *                    Must point to an array of at least two 32-bit entries.
 * @qfpromApiStatus:  Output pointer to receive the TME status code.
 * Returns: 0 on success; a negative  value on failure
 */
int Tme_FuseRead(enum TMEQFPROMAddrSpace_t addrType,
		 uint32_t fuseAddr,
		 uint32_t *fuseData,
		 uint32_t *qfpromApiStatus);

/**
 * Tme_FuseWriteMultiple - Write multiple QFPROM fuses via TME transport
 * @fuseArray:        Pointer to an array of TMEFuse_t entries describing address and 64-bit data.
 * @fuseArrayLen:     Number of entries in fuseArray; must be >0 and <= TME_MAX_FUSE_WRITE_REQ.
 * @qfpromApiStatus:  Output pointer to receive the controller/firmware status aggregate.
 *                    On success, set to 0; on failure, may carry a detailed code.
 * Returns: 0 on success; a negative value on failure.
 */
int Tme_FuseWriteMultiple(struct TMEFuse_t *fuseArray,
			  size_t fuseArrayLen,
			  uint32_t *qfpromApiStatus);
#endif /* _TMECOM_FUSE_RW_H_ */

