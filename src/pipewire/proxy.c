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

#include <pipewire/log.h>
#include <pipewire/proxy.h>
#include <pipewire/core.h>
#include <pipewire/remote.h>
#include <pipewire/private.h>
#include <pipewire/type.h>
#include <pipewire/interfaces.h>

#include <spa/debug/types.h>

#define NAME "proxy"

/** \cond */
struct proxy {
	struct pw_proxy this;
};
/** \endcond */

/** Create a proxy object with a given id and type
 *
 * \param factory another proxy object that serves as a factory
 * \param type Type of the proxy object
 * \param user_data_size size of user_data
 * \return A newly allocated proxy object or NULL on failure
 *
 * This function creates a new proxy object with the supplied id and type. The
 * proxy object will have an id assigned from the client id space.
 *
 * \sa pw_remote
 *
 * \memberof pw_proxy
 */
SPA_EXPORT
struct pw_proxy *pw_proxy_new(struct pw_proxy *factory,
			      uint32_t type, uint32_t version,
			      size_t user_data_size)
{
	struct proxy *impl;
	struct pw_proxy *this;
	struct pw_remote *remote = factory->remote;
	int res;

	impl = calloc(1, sizeof(struct proxy) + user_data_size);
	if (impl == NULL)
		return NULL;

	this = &impl->this;
	this->remote = remote;
	this->refcount = 1;
	this->version = version;

	this->marshal = pw_protocol_get_marshal(remote->conn->protocol, type, version);
	if (this->marshal == NULL) {
		pw_log_error(NAME" %p: no marshal for type %s/%d", this,
				spa_debug_type_find_name(pw_type_info(), type),
				version);
		res = -EPROTO;
		goto error_clean;
	}

	this->id = pw_map_insert_new(&remote->objects, this);
	if (this->id == SPA_ID_INVALID) {
		res = -errno;
		pw_log_error(NAME" %p: can't allocate new id: %m", this);
		goto error_clean;
	}

	spa_hook_list_init(&this->listener_list);
	spa_hook_list_init(&this->object_listener_list);

	if (user_data_size > 0)
		this->user_data = SPA_MEMBER(impl, sizeof(struct proxy), void);

	this->impl = SPA_INTERFACE_INIT(
			type,
			this->marshal->version,
			this->marshal->method_marshal, this);

	pw_log_debug(NAME" %p: new %u type %s/%d remote:%p, marshal:%p",
			this, this->id,
			spa_debug_type_find_name(pw_type_info(), type), version,
			remote, this->marshal);

	return this;

error_clean:
	free(impl);
	errno = -res;
	return NULL;
}

SPA_EXPORT
void *pw_proxy_get_user_data(struct pw_proxy *proxy)
{
	return proxy->user_data;
}

SPA_EXPORT
uint32_t pw_proxy_get_id(struct pw_proxy *proxy)
{
	return proxy->id;
}

SPA_EXPORT
struct pw_protocol *pw_proxy_get_protocol(struct pw_proxy *proxy)
{
	return proxy->remote->conn->protocol;
}

SPA_EXPORT
void pw_proxy_add_listener(struct pw_proxy *proxy,
			   struct spa_hook *listener,
			   const struct pw_proxy_events *events,
			   void *data)
{
	spa_hook_list_append(&proxy->listener_list, listener, events, data);
}

SPA_EXPORT
void pw_proxy_add_object_listener(struct pw_proxy *proxy,
				 struct spa_hook *listener,
				 const void *funcs,
				 void *data)
{
	spa_hook_list_append(&proxy->object_listener_list, listener, funcs, data);
}

/** Destroy a proxy object
 *
 * \param proxy Proxy object to destroy
 *
 * \note This is normally called by \ref pw_remote when the server
 *       decides to destroy the server side object
 * \memberof pw_proxy
 */
SPA_EXPORT
void pw_proxy_destroy(struct pw_proxy *proxy)
{
	struct pw_remote *remote = proxy->remote;

	if (!proxy->zombie) {
		pw_log_debug(NAME" %p: destroy %u", proxy, proxy->id);
		pw_proxy_emit_destroy(proxy);
	}
	if (!proxy->removed) {
		/* if the server did not remove this proxy, remove ourselves
		 * from the proxy objects and schedule a destroy. */
		if (remote->core_proxy) {
			proxy->zombie = true;
			pw_core_proxy_destroy(remote->core_proxy, proxy);
		} else {
			proxy->removed = true;
		}
	}
	if (proxy->removed) {
		pw_map_remove(&remote->objects, proxy->id);

		pw_proxy_unref(proxy);
	}
}

SPA_EXPORT
void pw_proxy_unref(struct pw_proxy *proxy)
{
	if (--proxy->refcount > 0)
		return;

	pw_log_debug(NAME" %p: free %u", proxy, proxy->id);
	free(proxy);
}

SPA_EXPORT
int pw_proxy_sync(struct pw_proxy *proxy, int seq)
{
	int res = -EIO;
	struct pw_remote *remote = proxy->remote;

	if (remote->core_proxy != NULL) {
		res = pw_core_proxy_sync(remote->core_proxy, proxy->id, seq);
		pw_log_debug(NAME" %p: %u seq:%d sync %u", proxy, proxy->id, seq, res);
	}
	return res;
}

SPA_EXPORT
int pw_proxy_error(struct pw_proxy *proxy, int res, const char *error, ...)
{
	va_list ap;
	int r = -EIO;
	struct pw_remote *remote = proxy->remote;

	va_start(ap, error);
	if (remote->core_proxy != NULL)
		r = pw_core_proxy_errorv(remote->core_proxy, proxy->id,
				remote->recv_seq, res, error, ap);
	va_end(ap);
	return r;
}

SPA_EXPORT
struct spa_hook_list *pw_proxy_get_object_listeners(struct pw_proxy *proxy)
{
	return &proxy->object_listener_list;
}

SPA_EXPORT
const struct pw_protocol_marshal *pw_proxy_get_marshal(struct pw_proxy *proxy)
{
	return proxy->marshal;
}
