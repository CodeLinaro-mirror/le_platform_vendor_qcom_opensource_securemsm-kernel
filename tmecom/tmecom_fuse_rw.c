// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/slab.h>

#include "tmecom_fuse_rw.h"
#include "tmecom.h"
#include "linux/tmecom_ioctl.h"

#define TMECOM_MSG_CBOR_TAG_FUSE_READ   (301)
#define TMECOM_MSG_CBOR_TAG_FUSE_WRITE_MULTIPLE   (316)

#define TME_MSG_CBOR_TAG_FUSE_READ           0x482D01D9 /* _be32 0xD9012D50 */
#define TME_MSG_CBOR_TAG_FUSE_WRITE_MULTIPLE 0x0403593C01D9 /* _be32 0xD9013C590304 */

long tmecom_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;

	switch (cmd) {
	case TMECOM_IOCTL_FUSE_READ: {
		struct tme_ioctl_fuse_read user;

		if (copy_from_user(&user, (void __user *)arg, sizeof(user)))
			return -EFAULT;


		user.ret = Tme_FuseRead(
			user.addr_type,
			user.fuse_addr,
			user.fuse_data,
			&user.qfprom_api_status
		);

		if (copy_to_user((void __user *)arg, &user, sizeof(user)))
			return -EFAULT;
		break;
	}

	case TMECOM_IOCTL_FUSE_WRITE: {
		struct tme_ioctl_fuse_write_multiple user;
		size_t i = 0;

		if (copy_from_user(&user, (void __user *)arg, sizeof(user)))
			return -EFAULT;


		if (user.fuse_array_len == 0 || user.fuse_array_len > TME_MAX_FUSE_WRITE_REQ) {
			ret = -EINVAL;
			break;
		}


		struct TMEFuse_t karr[TME_MAX_FUSE_WRITE_REQ];

		for (i = 0; i < user.fuse_array_len; i++) {
			karr[i].addr     = user.fuse_array[i].addr;
			karr[i].data[0]  = user.fuse_array[i].data[0];
			karr[i].data[1]  = user.fuse_array[i].data[1];
		}

		user.ret = Tme_FuseWriteMultiple(karr, user.fuse_array_len,
						 &user.qfprom_api_status);

		if (copy_to_user((void __user *)arg, &user, sizeof(user)))
			return -EFAULT;
		break;
	}

	default:
		return -ENOTTY;
	}
	return ret;
}
int Tme_FuseRead(enum TMEQFPROMAddrSpace_t addrType,
		 uint32_t fuseAddr,
		 uint32_t *fuseData,
		 uint32_t *qfpromApiStatus)
{
	struct tmeFuseReadReq_t *request = NULL;
	struct tmeFuseReadRsp_t *response = NULL;
	uint32_t ret = 0;
	size_t responseLen = sizeof(*response);

	// Validate output pointers
	if (!fuseData || !qfpromApiStatus)
		return -EINVAL;

	request = kzalloc(sizeof(*request), GFP_KERNEL);
	response = kzalloc(responseLen, GFP_KERNEL);

	if (!request || !response) {
		ret = -ENOMEM;
		goto err_exit;
	}

	// Prepare request
	request->addrType = (uint32_t)addrType;
	request->fuseAddr = fuseAddr;
	request->cbor_header = TME_MSG_CBOR_TAG_FUSE_READ;

	// Send request and receive response
	ret = tmecom_process_request(request,
				     sizeof(*request),
				     response,
				     &responseLen);

	if (ret != 0) {
		pr_err("Fuse read request failed for addrType %u, fuseAddr %u\n",
		       addrType, fuseAddr);
		goto err_exit;
	}

	if (responseLen != sizeof(*response)) {
		pr_err("Fuse read response failed with invalid length: %zu, expected: %zu\n",
		       responseLen, sizeof(*response));
		ret = -EBADMSG;
		goto err_exit;
	}

	// Copy fuse data and status
	memcpy(fuseData, response->fuseData, sizeof(response->fuseData));
	*qfpromApiStatus = response->qfpromApiStatus;

err_exit:
	kfree(request);
	kfree(response);
	return ret;
}
EXPORT_SYMBOL_GPL(Tme_FuseRead);

int Tme_FuseWriteMultiple(struct TMEFuse_t *fuseArray,
			  size_t fuseArrayLen,
			  uint32_t *qfpromApiStatus)
{
	struct tmeFuseWriteMultipleReq_t *request = NULL;
	struct tmeFuseWriteMultipleRsp_t response;
	uint32_t ret = 0;
	size_t responseLen = sizeof(response);

	// Validate pointers
	if (!fuseArray || !qfpromApiStatus)
		return -EFAULT;

	if (fuseArrayLen == 0)
		return -ENODATA;

	if (fuseArrayLen > TME_MAX_FUSE_WRITE_REQ)
		return -E2BIG;

	request = kzalloc(sizeof(*request), GFP_KERNEL);
	if (!request)
		return -ENOMEM;

	request->cbor_header[0] = 0xD9;
	request->cbor_header[1] = 0x01;
	request->cbor_header[2] = 0x3c;
	request->cbor_header[3] = 0x59;
	request->cbor_header[4] = 0x03;
	request->cbor_header[5] = 0x04;

	request->fuseArrayLen = fuseArrayLen;

	for (size_t i = 0; i < fuseArrayLen; ++i) {
		request->fuseArray[i].addr    = fuseArray[i].addr;
		request->fuseArray[i].data[0] = fuseArray[i].data[0];
		request->fuseArray[i].data[1] = fuseArray[i].data[1];
	}

	*qfpromApiStatus = 0;

	// Send request and receive response
	ret = tmecom_process_request(request,
				     sizeof(*request),
				     &response,
				     &responseLen);

	if (ret != 0)
		goto err_exit;

	if (responseLen == sizeof(response)) {
		*qfpromApiStatus = response.status;
		if (response.status == 0)
			ret = 0; // Success
		else
			ret = -EIO; // Failure
	} else {
		ret = -EBADMSG;
	}

err_exit:
	kfree(request);
	return ret;
}
EXPORT_SYMBOL_GPL(Tme_FuseWriteMultiple);
