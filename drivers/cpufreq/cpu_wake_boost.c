/*
 * Copyright (C) 2018, Sultan Alsawaf <sultanxda@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/cpu.h>
#include <linux/cpufreq.h>
#include <linux/state_notifier.h>
#include <linux/slab.h>

enum boost_state {
	NO_BOOST,
	UNBOOST,
	BOOST
};

/* The duration in milliseconds for the wake boost */
#define FB_BOOST_MS (3000)

struct wake_boost_info {
	struct workqueue_struct *wq;
	struct work_struct boost_work;
	struct delayed_work unboost_work;
	struct notifier_block notif;
	struct notifier_block cpu_notif;
	enum boost_state state;
};

static void update_online_cpu_policy(void)
{
	int cpu;

	/* Trigger cpufreq notifier for online CPUs */
	get_online_cpus();
	for_each_online_cpu(cpu)
		cpufreq_update_policy(cpu);
	put_online_cpus();
}

static void wake_boost(struct work_struct *work)
{
	struct wake_boost_info *w = container_of(work, typeof(*w), boost_work);

	w->state = BOOST;
	update_online_cpu_policy();

	queue_delayed_work(w->wq, &w->unboost_work,
				msecs_to_jiffies(FB_BOOST_MS));
}

static void wake_unboost(struct work_struct *work)
{
	struct wake_boost_info *w =
		container_of(work, typeof(*w), unboost_work.work);

	w->state = UNBOOST;
	update_online_cpu_policy();
}

static int do_cpu_boost(struct notifier_block *nb,
		unsigned long action, void *data)
{
	struct wake_boost_info *w = container_of(nb, typeof(*w), cpu_notif);
	struct cpufreq_policy *policy = data;

	if (action != CPUFREQ_ADJUST)
		return NOTIFY_OK;

	switch (w->state) {
	case UNBOOST:
		policy->min = policy->cpuinfo.min_freq;
		w->state = NO_BOOST;
		break;
	case BOOST:
		policy->min = policy->max;
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

static int state_notifier_callback(struct notifier_block *this,
				unsigned long event, void *data)
{
	struct wake_boost_info *w = container_of(this, typeof(*w), notif);

	switch (event) {
		case STATE_NOTIFIER_ACTIVE:
			queue_work(w->wq, &w->boost_work);
			break;
		case STATE_NOTIFIER_SUSPEND:
			if (cancel_delayed_work_sync(&w->unboost_work))
				queue_delayed_work(w->wq, &w->unboost_work, 0);
			break;
		default:
			break;
	}

	return NOTIFY_OK;
}

static int __init cpu_wake_boost_init(void)
{
	struct wake_boost_info *w;

	w = kzalloc(sizeof(*w), GFP_KERNEL);
	if (!w)
		return -ENOMEM;

	w->wq = alloc_workqueue("wake_boost_wq", WQ_HIGHPRI, 0);
	if (!w->wq) {
		kfree(w);
		return -ENOMEM;
	}

	INIT_WORK(&w->boost_work, wake_boost);
	INIT_DELAYED_WORK(&w->unboost_work, wake_unboost);

	w->cpu_notif.notifier_call = do_cpu_boost;
	cpufreq_register_notifier(&w->cpu_notif, CPUFREQ_POLICY_NOTIFIER);

	w->notif.notifier_call = state_notifier_callback;
	w->notif.priority = INT_MAX;
	state_register_client(&w->notif);

	return 0;
}
late_initcall(cpu_wake_boost_init);
