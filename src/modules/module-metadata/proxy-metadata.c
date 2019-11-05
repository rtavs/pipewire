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

#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include <spa/pod/parser.h>

#include "pipewire/pipewire.h"
#include "pipewire/private.h"
#include "extensions/metadata.h"

struct object_data {
	struct pw_remote *remote;
	struct pw_core *core;

	struct pw_metadata *object;
	struct spa_hook object_listener;
	struct spa_hook object_methods;

        struct pw_proxy *proxy;
	struct spa_hook proxy_listener;
};

static void object_proxy_destroy(void *_data)
{
	struct object_data *data = _data;
	spa_hook_remove(&data->object_listener);
}

static const struct pw_proxy_events proxy_events = {
	PW_VERSION_PROXY_EVENTS,
	.destroy = object_proxy_destroy,
};

struct pw_proxy *pw_remote_metadata_export(struct pw_remote *remote,
		uint32_t type, struct pw_properties *props, void *object,
		size_t user_data_size)
{
	struct pw_metadata *meta = object;
	struct spa_interface *iface;
	struct pw_proxy *proxy;
	struct object_data *data;

	proxy = pw_core_proxy_create_object(remote->core_proxy,
					    "metadata",
					    PW_TYPE_INTERFACE_Metadata,
					    PW_VERSION_METADATA,
					    props ? &props->dict : NULL,
					    user_data_size + sizeof(struct object_data));
	if (props)
		pw_properties_free(props);
        if (proxy == NULL)
		return NULL;

	data = pw_proxy_get_user_data(proxy);
	data = SPA_MEMBER(data, user_data_size, struct object_data);
	data->remote = remote;
	data->object = object;
	data->core = pw_remote_get_core(remote);
	data->proxy = proxy;

	iface = (struct spa_interface*)proxy;

	pw_proxy_install_marshal(proxy, true);

	pw_proxy_add_listener(proxy, &data->proxy_listener, &proxy_events, data);

	pw_proxy_add_object_listener(proxy, &data->object_methods,
			meta->iface.cb.funcs, meta->iface.cb.data);
	pw_metadata_add_listener(meta, &data->object_listener,
			iface->cb.funcs, iface->cb.data);

	return proxy;
}
