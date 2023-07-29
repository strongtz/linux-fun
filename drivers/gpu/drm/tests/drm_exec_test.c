// SPDX-License-Identifier: MIT
/*
 * Copyright 2022 Advanced Micro Devices, Inc.
 */

#define pr_fmt(fmt) "drm_exec: " fmt

#include <kunit/test.h>

#include <linux/module.h>
#include <linux/prime_numbers.h>

#include <drm/drm_exec.h>
#include <drm/drm_device.h>
#include <drm/drm_gem.h>

#include "../lib/drm_random.h"

static struct drm_device dev;

static void sanitycheck(struct kunit *test)
{
	struct drm_exec exec;

	drm_exec_init(&exec, DRM_EXEC_INTERRUPTIBLE_WAIT);
	drm_exec_fini(&exec);
	KUNIT_SUCCEED(test);
}

static void test_lock(struct kunit *test)
{
	struct drm_gem_object gobj = { };
	struct drm_exec exec;
	int ret;

	drm_gem_private_object_init(&dev, &gobj, PAGE_SIZE);

	drm_exec_init(&exec, DRM_EXEC_INTERRUPTIBLE_WAIT);
	drm_exec_until_all_locked(&exec) {
		ret = drm_exec_lock_obj(&exec, &gobj);
		drm_exec_retry_on_contention(&exec);
		KUNIT_EXPECT_EQ(test, ret, 0);
		if (ret)
			break;
	}
	drm_exec_fini(&exec);
}

static void test_lock_unlock(struct kunit *test)
{
	struct drm_gem_object gobj = { };
	struct drm_exec exec;
	int ret;

	drm_gem_private_object_init(&dev, &gobj, PAGE_SIZE);

	drm_exec_init(&exec, DRM_EXEC_INTERRUPTIBLE_WAIT);
	drm_exec_until_all_locked(&exec) {
		ret = drm_exec_lock_obj(&exec, &gobj);
		drm_exec_retry_on_contention(&exec);
		KUNIT_EXPECT_EQ(test, ret, 0);
		if (ret)
			break;

		drm_exec_unlock_obj(&exec, &gobj);
		ret = drm_exec_lock_obj(&exec, &gobj);
		drm_exec_retry_on_contention(&exec);
		KUNIT_EXPECT_EQ(test, ret, 0);
		if (ret)
			break;
	}
	drm_exec_fini(&exec);
}

static void test_duplicates(struct kunit *test)
{
	struct drm_gem_object gobj = { };
	struct drm_exec exec;
	int ret;

	drm_gem_private_object_init(&dev, &gobj, PAGE_SIZE);

	drm_exec_init(&exec, DRM_EXEC_IGNORE_DUPLICATES);
	drm_exec_until_all_locked(&exec) {
		ret = drm_exec_lock_obj(&exec, &gobj);
		drm_exec_retry_on_contention(&exec);
		KUNIT_EXPECT_EQ(test, ret, 0);
		if (ret)
			break;

		ret = drm_exec_lock_obj(&exec, &gobj);
		drm_exec_retry_on_contention(&exec);
		KUNIT_EXPECT_EQ(test, ret, 0);
		if (ret)
			break;
	}
	drm_exec_unlock_obj(&exec, &gobj);
	drm_exec_fini(&exec);
}



static void test_prepare(struct kunit *test)
{
	struct drm_gem_object gobj = { };
	struct drm_exec exec;
	int ret;

	drm_gem_private_object_init(&dev, &gobj, PAGE_SIZE);

	drm_exec_init(&exec, DRM_EXEC_INTERRUPTIBLE_WAIT);
	drm_exec_until_all_locked(&exec) {
		ret = drm_exec_prepare_obj(&exec, &gobj, 1);
		drm_exec_retry_on_contention(&exec);
		KUNIT_EXPECT_EQ(test, ret, 0);
		if (ret)
			break;
	}
	drm_exec_fini(&exec);
}

static void test_prepare_array(struct kunit *test)
{
	struct drm_gem_object gobj1 = { };
	struct drm_gem_object gobj2 = { };
	struct drm_gem_object *array[] = { &gobj1, &gobj2 };
	struct drm_exec exec;
	int ret;

	drm_gem_private_object_init(&dev, &gobj1, PAGE_SIZE);
	drm_gem_private_object_init(&dev, &gobj2, PAGE_SIZE);

	drm_exec_init(&exec, DRM_EXEC_INTERRUPTIBLE_WAIT);
	drm_exec_until_all_locked(&exec)
		ret = drm_exec_prepare_array(&exec, array, ARRAY_SIZE(array),
					     1);
	KUNIT_EXPECT_EQ(test, ret, 0);
	drm_exec_fini(&exec);
}

static struct kunit_case drm_exec_tests[] = {
	KUNIT_CASE(sanitycheck),
	KUNIT_CASE(test_lock),
	KUNIT_CASE(test_lock_unlock),
	KUNIT_CASE(test_duplicates),
	KUNIT_CASE(test_prepare),
	KUNIT_CASE(test_prepare_array),
	{}
};

static struct kunit_suite drm_exec_test_suite = {
	.name = "drm_exec",
	.test_cases = drm_exec_tests,
};

kunit_test_suite(drm_exec_test_suite);

MODULE_AUTHOR("AMD");
MODULE_LICENSE("GPL and additional rights");
