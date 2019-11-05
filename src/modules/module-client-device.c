/* PipeWire
 *
 * Copyright © 2019 Wim Taymans
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
#include <stdio.h>
#include <errno.h>
#include <dlfcn.h>

#include "config.h"

#include <spa/utils/result.h>

#include <pipewire/pipewire.h>

#include "module-client-device/client-device.h"

#define NAME "client-device"

static const struct spa_dict_item module_props[] = {
	{ PW_KEY_MODULE_AUTHOR, "Wim Taymans <wim.taymans@gmail.com>" },
	{ PW_KEY_MODULE_DESCRIPTION, "Allow clients to create and control remote devices" },
	{ PW_KEY_MODULE_VERSION, PACKAGE_VERSION },
};

struct pw_proxy *pw_remote_spa_device_export(struct pw_remote *remote,
		uint32_t type, struct pw_properties *props, void *object,
		size_t user_data_size);

struct pw_protocol *pw_protocol_native_ext_client_device_init(struct pw_core *core);

struct factory_data {
	struct pw_factory *this;

	struct pw_module *module;
	struct spa_hook module_listener;

	struct pw_export_type export_spadevice;
};

static void *create_object(void *_data,
			   struct pw_resource *resource,
			   uint32_t type,
			   uint32_t version,
			   struct pw_properties *properties,
			   uint32_t new_id)
{
	struct factory_data *data = _data;
	struct pw_factory *this = data->this;
	void *result;
	struct pw_resource *device_resource;
	struct pw_client *client = pw_resource_get_client(resource);
	int res;

	device_resource = pw_resource_new(client, new_id, PW_PERM_RWX, type, version, 0);
	if (device_resource == NULL) {
		res = -errno;
		goto error_resource;
	}

	if (properties == NULL)
		properties = pw_properties_new(NULL, NULL);
	if (properties == NULL) {
		res = -errno;
		goto error_properties;
	}

	pw_properties_setf(properties, PW_KEY_FACTORY_ID, "%d",
			pw_global_get_id(pw_factory_get_global(this)));
	pw_properties_setf(properties, PW_KEY_CLIENT_ID, "%d",
			pw_global_get_id(pw_client_get_global(client)));

	result = pw_client_device_new(device_resource, properties);
	if (result == NULL) {
		res = -errno;
		goto error_device;
	}
	return result;

error_resource:
	pw_log_error("can't create resource: %s", spa_strerror(res));
	pw_resource_error(resource, res, "can't create resource: %s", spa_strerror(res));
	goto error_exit;
error_properties:
	pw_log_error("can't create properties: %s", spa_strerror(res));
	pw_resource_error(resource, res, "can't create properties: %s", spa_strerror(res));
	goto error_exit_free;
error_device:
	pw_log_error("can't create device: %s", spa_strerror(res));
	pw_resource_error(resource, res, "can't create device: %s", spa_strerror(res));
	goto error_exit_free;

error_exit_free:
	pw_resource_destroy(device_resource);
error_exit:
	errno = -res;
	return NULL;
}

static const struct pw_factory_implementation impl_factory = {
	PW_VERSION_FACTORY_IMPLEMENTATION,
	.create_object = create_object,
};

static void module_destroy(void *data)
{
	struct factory_data *d = data;

	spa_hook_remove(&d->module_listener);

	spa_list_remove(&d->export_spadevice.link);

	pw_factory_destroy(d->this);
}

static void module_registered(void *data)
{
	struct factory_data *d = data;
	struct pw_module *module = d->module;
	struct pw_factory *factory = d->this;
	struct spa_dict_item items[1];
	char id[16];
	int res;

	snprintf(id, sizeof(id), "%d", pw_global_get_id(pw_module_get_global(module)));
	items[0] = SPA_DICT_ITEM_INIT(PW_KEY_MODULE_ID, id);
	pw_factory_update_properties(factory, &SPA_DICT_INIT(items, 1));

	if ((res = pw_factory_register(factory, NULL)) < 0) {
		pw_log_error(NAME" %p: can't register factory: %s", factory, spa_strerror(res));
	}
}

static const struct pw_module_events module_events = {
	PW_VERSION_MODULE_EVENTS,
	.destroy = module_destroy,
	.registered = module_registered,
};

SPA_EXPORT
int pipewire__module_init(struct pw_module *module, const char *args)
{
	struct pw_core *core = pw_module_get_core(module);
	struct pw_factory *factory;
	struct factory_data *data;

	factory = pw_factory_new(core,
				 "client-device",
				 SPA_TYPE_INTERFACE_Device,
				 SPA_VERSION_DEVICE,
				 pw_properties_new(
					 PW_KEY_FACTORY_USAGE, CLIENT_DEVICE_USAGE,
					 NULL),
				 sizeof(*data));
	if (factory == NULL)
		return -errno;

	data = pw_factory_get_user_data(factory);
	data->this = factory;
	data->module = module;

	pw_log_debug("module %p: new", module);

	pw_factory_set_implementation(factory,
				      &impl_factory,
				      data);

	pw_protocol_native_ext_client_device_init(core);

	data->export_spadevice.type = SPA_TYPE_INTERFACE_Device;
	data->export_spadevice.func = pw_remote_spa_device_export;
	pw_core_register_export_type(core, &data->export_spadevice);

	pw_module_add_listener(module, &data->module_listener, &module_events, data);

	pw_module_update_properties(module, &SPA_DICT_INIT_ARRAY(module_props));

	return 0;
}
