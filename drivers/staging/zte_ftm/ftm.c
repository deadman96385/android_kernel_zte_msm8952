/*
 * drivers/staging/zte_ftm/ftm.c
 *
 * Copyright (C) 2012-2013 ZTE, Corp.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *  Date         Author           Comment
 *  -----------  --------------   -------------------------------------
 *  2012-08-03   Jia              The kernel module for FTM,
 *                                created by ZTE_BOO_JIA_20120803 jia.jia
 *  -------------------------------------------------------------------
 *
*/

#include <linux/module.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/platform_device.h>
#include <linux/io.h>

#include <soc/qcom/scm.h>
#include <soc/qcom/smem.h>
#include <linux/of.h>
#include <linux/of_address.h>
/*
 * Macro Definition
 */
#define FTM_VERSION "1.0"
#define FTM_CLASS_NAME "zte_ftm"

#define BYTES_PER_FUSE_ROW 0x08
#define QFPROM_SIMLOCK_MASK            0xC0000000LL
#define QFPROM_SECBOOT_MASK            0x00303030LL

#define QFPROM_SECBOOT_ROW_NUM            19
#define QFPROM_SIMLOCK_ROW_NUM            6

#define QFPROM_EFUSE_BASE_ADDR          0x00058000
#define QFPROM_EFUSE_MAX_LEN            0x1000

/*
 * Type Definition
 */
typedef struct {
	uint32_t secboot_status;
	uint32_t simlock_status;
} ftm_sysdev_attrs_t;


typedef struct {
       /* eFuse parameters */
        phys_addr_t     pa;
        void __iomem    *va;
}efuse_base_addr_t;


typedef struct  {
       /* eFuse parameters */
	efuse_base_addr_t efuse_base_addr;
} ftm_sysdev_priv_data_t; 


/*
 * Global Variables Definition
 */
static ftm_sysdev_attrs_t ftm_sysdev_attrs;
static ftm_sysdev_priv_data_t ftm_sysdev_priv_data;


/*
 * Function declaration
 */

static u64 ftm_read_efuse_row(efuse_base_addr_t *pbase,
                                        u32 row_num, bool use_tz_api)
{
        int rc;
        u64 efuse_bits;
        struct scm_desc desc = {0};
        struct efuse_read_req {
                u32 row_address;
                int addr_type;
        } req;

        struct efuse_read_rsp {
                u32 row_data[2];
                u32 status;
        } rsp;

        if (!use_tz_api) {
                efuse_bits = readq_relaxed(pbase->va
                        + row_num * BYTES_PER_FUSE_ROW);
                return efuse_bits;
        }

        desc.args[0] = req.row_address = pbase->pa +
                                        row_num * BYTES_PER_FUSE_ROW;
        desc.args[1] = req.addr_type = 0;
        desc.arginfo = SCM_ARGS(2);
        efuse_bits = 0;

        if (!is_scm_armv8()) {
                rc = scm_call(SCM_SVC_FUSE, SCM_FUSE_READ,
                        &req, sizeof(req), &rsp, sizeof(rsp));
        } else {
                rc = scm_call2(SCM_SIP_FNID(SCM_SVC_FUSE, SCM_FUSE_READ),
                                &desc);
                rsp.row_data[0] = desc.ret[0];
                rsp.row_data[1] = desc.ret[1];
                rsp.status = desc.ret[2];
        }

        if (rc) {
                pr_err("read row %d failed, err code = %d", row_num, rc);
        } else {
                efuse_bits = ((u64)(rsp.row_data[1]) << 32) +
                                (u64)rsp.row_data[0];
        }

        return efuse_bits;
}


static ssize_t show_simlock_status(struct device *dev, struct device_attribute *attr, char *buf)
{
        u64 efuse_data = 0;
        efuse_base_addr_t *pbase = &ftm_sysdev_priv_data.efuse_base_addr;

        /*
        * 0: No Blown
        * 1: Has Blown
        * 0xff: status error
        */
        efuse_data = ftm_read_efuse_row(pbase,QFPROM_SIMLOCK_ROW_NUM,0);

	 pr_info("%s: simlock_fuse_status = %08lx\n", __func__,(long)efuse_data);

        ftm_sysdev_attrs.simlock_status = ((efuse_data & QFPROM_SIMLOCK_MASK) == QFPROM_SIMLOCK_MASK) ? 1 : 0;

        return snprintf(buf, PAGE_SIZE, "%d\n", ftm_sysdev_attrs.simlock_status);

}

static DEVICE_ATTR(simlock_status, S_IRUGO | S_IRUSR, show_simlock_status, NULL);

static ssize_t show_secboot_status(struct device *dev, struct device_attribute *attr, char *buf)
{
        u64 efuse_data = 0;
        efuse_base_addr_t *pbase = &ftm_sysdev_priv_data.efuse_base_addr;
        
        /*
        * 0: No Blown
        * 1: Has Blown
        * 0xff: status error
        */
        efuse_data = ftm_read_efuse_row(pbase,QFPROM_SECBOOT_ROW_NUM,0);

	 pr_info("%s: secboot_fuse_status = %08lx\n", __func__,(long)efuse_data);

        ftm_sysdev_attrs.secboot_status =((efuse_data & QFPROM_SECBOOT_MASK) == QFPROM_SECBOOT_MASK) ? 1 : 0;

        return snprintf(buf, PAGE_SIZE, "%d\n", ftm_sysdev_attrs.secboot_status);
}

static DEVICE_ATTR(secboot_status, S_IRUGO | S_IRUSR, show_secboot_status, NULL);

/* ZTE_BOOT_20130123 huang.yanjun add for efs golden recovery <----*/

static struct attribute  *ftm_attrs[] = {
	&dev_attr_secboot_status.attr,
	&dev_attr_simlock_status.attr,
	NULL,
};

static struct attribute_group ftm_attr_grp = {
        .name = "zte_ftm",
        .attrs = ftm_attrs
};

static int32_t __init ftm_probe(struct platform_device *pdev)
{
        struct device *dev = &pdev->dev;
        int ret = 0;

        pr_info("%s: e\n", __func__);

        ftm_sysdev_priv_data.efuse_base_addr.pa = QFPROM_EFUSE_BASE_ADDR;

        ftm_sysdev_priv_data.efuse_base_addr.va = ioremap(ftm_sysdev_priv_data.efuse_base_addr.pa,QFPROM_EFUSE_MAX_LEN);

        if (ftm_sysdev_priv_data.efuse_base_addr.va == NULL) {
                pr_err("%s: failed to map memory!\n", __func__);
                return -ENOMEM;
        }

        /*
         * Initialize sys device
         */
        ret = sysfs_create_group(&dev->kobj, &ftm_attr_grp);
	if(ret) {
                pr_err("cannot create sysfs group err: %d\n",ret);
                return ret;
        }

        pr_info("%s: x\n", __func__);

	return 0;
}

static int32_t ftm_remove(struct platform_device *pdev)
{
	pr_info("%s: e\n", __func__);

	pr_info("%s: x\n", __func__);

	return 0;
}


#define DRIVER_NAME "zte_ftm"
static struct platform_device ftm_dev = {
        .name = DRIVER_NAME,
        .id   = -1,
};


static struct platform_driver ftm_driver = {
	.remove = ftm_remove,
	.driver = {
		.name = DRIVER_NAME,
	},
};

/*
 * Initializes the module.
 */
static int32_t __init ftm_init(void)
{
	int ret;

	pr_info("%s: e\n", __func__);

        ret = platform_device_register(&ftm_dev);
        if (ret){
                pr_info("platform_device_register error,%d\n",ret);

                return -ENODEV;
        }


	ret = platform_driver_probe(&ftm_driver, ftm_probe);
	if (ret) {
                pr_info("platform_driver_probe error,%d\n",ret);

	    return ret;
	}



	pr_info("%s: x\n", __func__);

	return 0;
}

/*
 * Cleans up the module.
 */
static void __exit ftm_exit(void)
{
	platform_driver_unregister(&ftm_driver);
	platform_device_unregister(&ftm_dev);
}

module_init(ftm_init);
module_exit(ftm_exit);

MODULE_DESCRIPTION("ZTE FTM Driver Ver %s" FTM_VERSION);
#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif
