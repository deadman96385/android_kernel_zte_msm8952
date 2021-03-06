/*
 * Copyright (C) 2013 Google, Inc.
 * Copyright (c) 2013-2015, The Linux Foundation. All rights reserved.
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
 */

#undef TRACE_SYSTEM
#define TRACE_SYSTEM mmc

#if !defined(_TRACE_MMC_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_MMC_H

#include <linux/tracepoint.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/core.h>
#include <linux/ktime.h>//tcd
/*
 * Unconditional logging of mmc block erase operations,
 * including cmd, address, size
 */
DECLARE_EVENT_CLASS(mmc_blk_erase_class,
	TP_PROTO(unsigned int cmd, unsigned int addr, unsigned int size),
	TP_ARGS(cmd, addr, size),
	TP_STRUCT__entry(
		__field(unsigned int, cmd)
		__field(unsigned int, addr)
		__field(unsigned int, size)
	),
	TP_fast_assign(
		__entry->cmd = cmd;
		__entry->addr = addr;
		__entry->size = size;
	),
	TP_printk("cmd=%u,addr=0x%08x,size=0x%08x",
		  __entry->cmd, __entry->addr, __entry->size)
);

DEFINE_EVENT(mmc_blk_erase_class, mmc_blk_erase_start,
	TP_PROTO(unsigned int cmd, unsigned int addr, unsigned int size),
	TP_ARGS(cmd, addr, size));

DEFINE_EVENT(mmc_blk_erase_class, mmc_blk_erase_end,
	TP_PROTO(unsigned int cmd, unsigned int addr, unsigned int size),
	TP_ARGS(cmd, addr, size));

#ifdef CONFIG_TASKSTATS
TRACE_EVENT(mmc_pid_blk_read_summary,
	TP_PROTO(struct task_struct *p),
	TP_ARGS(p),
	TP_STRUCT__entry(
		__field(pid_t, pid)
		__field(unsigned long long, io_read_bytes)
	),
	TP_fast_assign(
		__entry->pid = p->pid;
		__entry->io_read_bytes = p->ioac.read_bytes;
	),
	TP_printk("process [%u] do read io %llu bytes",
		  __entry->pid, __entry->io_read_bytes)
);

TRACE_EVENT(mmc_pid_blk_write_summary,
	TP_PROTO(struct task_struct *p),
	TP_ARGS(p),
	TP_STRUCT__entry(
		__field(pid_t, pid)
		__field(unsigned long long, io_write_bytes)
	),
	TP_fast_assign(
		__entry->pid = p->pid;
		__entry->io_write_bytes = p->ioac.write_bytes;
	),
	TP_printk("process [%u] do write io %llu bytes",
		  __entry->pid, __entry->io_write_bytes)
);

DECLARE_EVENT_CLASS(mmc_blk_rw_summary_class,
	TP_PROTO(unsigned long rb, s64 rt, unsigned long wb, s64 wt),
	TP_ARGS(rb,rt,wb,wt),
	TP_STRUCT__entry(
		__field(unsigned long, readbyte)
		__field(s64, readtime)
		__field(unsigned long, writebyte)
		__field(s64, writetime)
	),
	TP_fast_assign(
		__entry->readbyte = rb;
		__entry->writebyte = wb;
		__entry->readtime = rt;
		__entry->writetime = wt;
	),
	TP_printk("emmc_ior:%lu bytes in %lld us;emmc_iow:%lu bytes in %lld us", __entry->readbyte, __entry->readtime,__entry->writebyte, __entry->writetime)
);
		

DEFINE_EVENT(mmc_blk_rw_summary_class, mmc_blk_rw_summary,
	TP_PROTO(unsigned long rb, s64 rt, unsigned long wb, s64 wt),
	TP_ARGS(rb,rt,wb,wt));
#endif
/*
 * Logging of start of read or write mmc block operation,
 * including cmd, address, size
 */
DECLARE_EVENT_CLASS(mmc_blk_rw_class,
	TP_PROTO(unsigned int cmd, unsigned int addr, struct mmc_data *data),
	TP_ARGS(cmd, addr, data),
	TP_STRUCT__entry(
		__field(unsigned int, cmd)
		__field(unsigned int, addr)
		__field(unsigned int, size)
	),
	TP_fast_assign(
		__entry->cmd = cmd;
		__entry->addr = addr;
		__entry->size = data->blocks;
	),
	TP_printk("cmd=%u,addr=0x%08x,size=0x%08x",
		  __entry->cmd, __entry->addr, __entry->size)
);

DEFINE_EVENT_CONDITION(mmc_blk_rw_class, mmc_blk_rw_start,
	TP_PROTO(unsigned int cmd, unsigned int addr, struct mmc_data *data),
	TP_ARGS(cmd, addr, data),
	TP_CONDITION(((cmd == MMC_READ_MULTIPLE_BLOCK) ||
		      (cmd == MMC_WRITE_MULTIPLE_BLOCK)) &&
		      data));

DEFINE_EVENT_CONDITION(mmc_blk_rw_class, mmc_blk_rw_end,
	TP_PROTO(unsigned int cmd, unsigned int addr, struct mmc_data *data),
	TP_ARGS(cmd, addr, data),
	TP_CONDITION(((cmd == MMC_READ_MULTIPLE_BLOCK) ||
		      (cmd == MMC_WRITE_MULTIPLE_BLOCK)) &&
		      data));

TRACE_EVENT(mmc_cmd_rw_start,
	TP_PROTO(unsigned int cmd, unsigned int arg, unsigned int flags),
	TP_ARGS(cmd, arg, flags),
	TP_STRUCT__entry(
		__field(unsigned int, cmd)
		__field(unsigned int, arg)
		__field(unsigned int, flags)
	),
	TP_fast_assign(
		__entry->cmd = cmd;
		__entry->arg = arg;
		__entry->flags = flags;
	),
	TP_printk("cmd=%u,arg=0x%08x,flags=0x%08x",
		  __entry->cmd, __entry->arg, __entry->flags)
);

TRACE_EVENT(mmc_cmd_rw_end,
	TP_PROTO(unsigned int cmd, unsigned int status, unsigned int resp),
	TP_ARGS(cmd, status, resp),
	TP_STRUCT__entry(
		__field(unsigned int, cmd)
		__field(unsigned int, status)
		__field(unsigned int, resp)
	),
	TP_fast_assign(
		__entry->cmd = cmd;
		__entry->status = status;
		__entry->resp = resp;
	),
	TP_printk("cmd=%u,int_status=0x%08x,response=0x%08x",
		  __entry->cmd, __entry->status, __entry->resp)
);

TRACE_EVENT(mmc_data_rw_end,
	TP_PROTO(unsigned int cmd, unsigned int status),
	TP_ARGS(cmd, status),
	TP_STRUCT__entry(
		__field(unsigned int, cmd)
		__field(unsigned int, status)
	),
	TP_fast_assign(
		__entry->cmd = cmd;
		__entry->status = status;
	),
	TP_printk("cmd=%u,int_status=0x%08x",
		  __entry->cmd, __entry->status)
);

DECLARE_EVENT_CLASS(mmc_adma_class,
	TP_PROTO(unsigned int cmd, unsigned int len),
	TP_ARGS(cmd, len),
	TP_STRUCT__entry(
		__field(unsigned int, cmd)
		__field(unsigned int, len)
	),
	TP_fast_assign(
		__entry->cmd = cmd;
		__entry->len = len;
	),
	TP_printk("cmd=%u,sg_len=0x%08x", __entry->cmd, __entry->len)
);

DEFINE_EVENT(mmc_adma_class, mmc_adma_table_pre,
	TP_PROTO(unsigned int cmd, unsigned int len),
	TP_ARGS(cmd, len));

DEFINE_EVENT(mmc_adma_class, mmc_adma_table_post,
	TP_PROTO(unsigned int cmd, unsigned int len),
	TP_ARGS(cmd, len));

TRACE_EVENT(mmc_clk,
	TP_PROTO(char *print_info),

	TP_ARGS(print_info),

	TP_STRUCT__entry(
		__string(print_info, print_info)
	),

	TP_fast_assign(
		__assign_str(print_info, print_info);
	),

	TP_printk("%s",
		__get_str(print_info)
	)
);

DECLARE_EVENT_CLASS(mmc_pm_qos,
	TP_PROTO(const char *dev_name, int last_vote_type, int curr_vote,
	    unsigned int qos_type, unsigned long cpumask,
	    unsigned int power_policy, int opcode, int vote_type),
	TP_ARGS(dev_name, last_vote_type, curr_vote, qos_type, cpumask,
	    power_policy, opcode, vote_type),
	TP_STRUCT__entry(
		__string(dev_name, dev_name)
		__field(int, last_vote_type)
		__field(int, curr_vote)
		__field(unsigned int, qos_type)
		__field(unsigned long, cpumask)
		__field(unsigned int, power_policy)
		__field(int, opcode)
		__field(int, vote_type)
	),
	TP_fast_assign(
		__assign_str(dev_name, dev_name);
		__entry->last_vote_type = last_vote_type;
		__entry->curr_vote = curr_vote;
		__entry->qos_type = qos_type;
		__entry->cpumask = cpumask;
		__entry->power_policy = power_policy;
		__entry->opcode = opcode;
		__entry->vote_type = vote_type;
	),
	TP_printk("%s: last_vote_type = %d, curr_vote = %d, qos_type = %u, cpumask = 0x%lx, power_policy = %u, opcode = %d, vote_type = %d\n",
		__get_str(dev_name), __entry->last_vote_type,
		__entry->curr_vote, __entry->qos_type, __entry->cpumask,
		__entry->power_policy, __entry->opcode, __entry->vote_type)
);

DEFINE_EVENT(mmc_pm_qos, mmc_pm_qos_vote,
	TP_PROTO(const char *dev_name, int last_vote_type, int curr_vote,
	    unsigned int qos_type, unsigned long cpumask,
	    unsigned int power_policy, int opcode, int vote_type),
	TP_ARGS(dev_name, last_vote_type, curr_vote, qos_type, cpumask,
	    power_policy, opcode, vote_type));

DEFINE_EVENT(mmc_pm_qos, mmc_pm_qos_unvote,
	TP_PROTO(const char *dev_name, int last_vote_type, int curr_vote,
	    unsigned int qos_type, unsigned long cpumask,
	    unsigned int power_policy, int opcode, int vote_type),
	TP_ARGS(dev_name, last_vote_type, curr_vote, qos_type, cpumask,
	    power_policy, opcode, vote_type));

DECLARE_EVENT_CLASS(mmc_pm_template,
	TP_PROTO(const char *dev_name, int err, s64 usecs),

	TP_ARGS(dev_name, err, usecs),

	TP_STRUCT__entry(
		__field(s64, usecs)
		__field(int, err)
		__string(dev_name, dev_name)
	),

	TP_fast_assign(
		__entry->usecs = usecs;
		__entry->err = err;
		__assign_str(dev_name, dev_name);
	),

	TP_printk(
		"took %lld usecs, %s err %d",
		__entry->usecs,
		__get_str(dev_name),
		__entry->err
	)
);

DEFINE_EVENT(mmc_pm_template, mmc_suspend_host,
	     TP_PROTO(const char *dev_name, int err, s64 usecs),
	     TP_ARGS(dev_name, err, usecs));

DEFINE_EVENT(mmc_pm_template, mmc_resume_host,
	     TP_PROTO(const char *dev_name, int err, s64 usecs),
	     TP_ARGS(dev_name, err, usecs));

DEFINE_EVENT(mmc_pm_template, mmc_host_runtime_suspend,
	     TP_PROTO(const char *dev_name, int err, s64 usecs),
	     TP_ARGS(dev_name, err, usecs));

DEFINE_EVENT(mmc_pm_template, mmc_host_runtime_resume,
	     TP_PROTO(const char *dev_name, int err, s64 usecs),
	     TP_ARGS(dev_name, err, usecs));

#endif /* if !defined(_TRACE_MMC_H) || defined(TRACE_HEADER_MULTI_READ) */

/* This part must be outside protection */
#include <trace/define_trace.h>
