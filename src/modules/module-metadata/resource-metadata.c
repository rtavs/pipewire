/* PipeWire
 *
 * Copyright © 2018 Wim Taymans
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/eventfd.h>

#include <spa/monitor/device.h>
#include <spa/monitor/utils.h>
#include <spa/pod/filter.h>
#include <spa/pod/parser.h>
#include <spa/debug/types.h>

#include <pipewire/pipewire.h>
#include "pipewire/private.h"

struct impl {
	struct pw_core *core;
	struct pw_device *device;
	struct spa_hook device_listener;

	struct pw_resource *resource;
	struct spa_hook resource_listener;
	struct spa_hook object_listener;

	unsigned int registered:1;
};

static void device_info(void *data, const struct spa_device_info *info)
{
	struct impl *impl = data;
	if (!impl->registered) {
		pw_device_register(impl->device, NULL);
		pw_device_set_implementation(impl->device,
				(struct spa_device*)impl->resource);
		impl->registered = true;
	}
}

static const struct spa_device_events object_events = {
	SPA_VERSION_DEVICE_EVENTS,
	.info = device_info,
};

static void device_resource_destroy(void *data)
{
	struct impl *impl = data;

	pw_log_debug("client-device %p: destroy", impl);

	impl->resource = NULL;
	spa_hook_remove(&impl->device_listener);
	spa_hook_remove(&impl->resource_listener);
	spa_hook_remove(&impl->object_listener);
	pw_device_destroy(impl->device);
}

static const struct pw_resource_events resource_events = {
	PW_VERSION_RESOURCE_EVENTS,
	.destroy = device_resource_destroy,
};

static void device_destroy(void *data)
{
	struct impl *impl = data;

	pw_log_debug("client-device %p: destroy", impl);

	impl->device = NULL;
	spa_hook_remove(&impl->device_listener);
	spa_hook_remove(&impl->resource_listener);
	spa_hook_remove(&impl->object_listener);
	pw_resource_destroy(impl->resource);
}

static const struct pw_device_events device_events = {
	PW_VERSION_DEVICE_EVENTS,
	.destroy = device_destroy,
};

struct pw_device *pw_client_device_new(struct pw_resource *resource,
		struct pw_properties *properties)
{
	struct impl *impl;
	struct pw_device *device;
	struct pw_client *client = pw_resource_get_client(resource);
	struct pw_core *core = pw_client_get_core(client);

	if (properties == NULL)
		properties = pw_properties_new(NULL, NULL);
	if (properties == NULL)
		return NULL;

	pw_properties_setf(properties, PW_KEY_CLIENT_ID, "%d", client->global->id);

	device = pw_device_new(core, properties, sizeof(struct impl));
	if (device == NULL)
		return NULL;

	impl = pw_device_get_user_data(device);
	impl->device = device;
	impl->core = core;
	impl->resource = resource;

	pw_resource_install_marshal(resource, true);

	pw_device_add_listener(impl->device,
			&impl->device_listener,
			&device_events, impl);

	pw_resource_add_listener(impl->resource,
				&impl->resource_listener,
				&resource_events,
				impl);
	pw_resource_add_object_listener(impl->resource,
				&impl->object_listener,
				&object_events,
				impl);

	return device;
}
