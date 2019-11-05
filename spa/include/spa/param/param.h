/* Simple Plugin API
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

#ifndef SPA_PARAM_H
#define SPA_PARAM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <spa/utils/defs.h>

/** different parameter types that can be queried */
enum spa_param_type {
	SPA_PARAM_Invalid,		/**< invalid */
	SPA_PARAM_PropInfo,		/**< property information as SPA_TYPE_OBJECT_PropInfo */
	SPA_PARAM_Props,		/**< properties as SPA_TYPE_OBJECT_Props */
	SPA_PARAM_EnumFormat,		/**< available formats as SPA_TYPE_OBJECT_Format */
	SPA_PARAM_Format,		/**< configured format as SPA_TYPE_OBJECT_Format */
	SPA_PARAM_Buffers,		/**< buffer configurations as SPA_TYPE_OBJECT_ParamBuffers*/
	SPA_PARAM_Meta,			/**< allowed metadata for buffers as SPA_TYPE_OBJECT_ParamMeta*/
	SPA_PARAM_IO,			/**< configurable IO areas as SPA_TYPE_OBJECT_ParamIO */
	SPA_PARAM_EnumProfile,		/**< profile enumeration as SPA_TYPE_OBJECT_ParamProfile */
	SPA_PARAM_Profile,		/**< profile configuration as SPA_TYPE_OBJECT_ParamProfile */
	SPA_PARAM_EnumPortConfig,	/**< port configuration enumeration as SPA_TYPE_OBJECT_ParamPortConfig */
	SPA_PARAM_PortConfig,		/**< port configuration as SPA_TYPE_OBJECT_ParamPortConfig */
};

/** information about a parameter */
struct spa_param_info {
	uint32_t id;			/**< enum spa_param_type */
#define SPA_PARAM_INFO_SERIAL		(1<<0)	/**< bit to signal update even when the
						 *   read/write flags don't change */
#define SPA_PARAM_INFO_READ		(1<<1)
#define SPA_PARAM_INFO_WRITE		(1<<2)
#define SPA_PARAM_INFO_READWRITE	(SPA_PARAM_INFO_WRITE|SPA_PARAM_INFO_READ)
	uint32_t flags;
	uint32_t padding[6];
};

#define SPA_PARAM_INFO(id,flags) (struct spa_param_info){ (id), (flags) }

/** properties for SPA_TYPE_OBJECT_ParamBuffers */
enum spa_param_buffers {
	SPA_PARAM_BUFFERS_START,
	SPA_PARAM_BUFFERS_buffers,	/**< number of buffers (Int) */
	SPA_PARAM_BUFFERS_blocks,	/**< number of data blocks per buffer (Int) */
	SPA_PARAM_BUFFERS_size,		/**< size of a data block memory (Int)*/
	SPA_PARAM_BUFFERS_stride,	/**< stride of data block memory (Int) */
	SPA_PARAM_BUFFERS_align,	/**< alignment of data block memory (Int) */
};

/** properties for SPA_TYPE_OBJECT_ParamMeta */
enum spa_param_meta {
	SPA_PARAM_META_START,
	SPA_PARAM_META_type,	/**< the metadata, one of enum spa_meta_type (Id enum spa_meta_type) */
	SPA_PARAM_META_size,	/**< the expected maximum size the meta (Int) */
};

/** properties for SPA_TYPE_OBJECT_ParamIO */
enum spa_param_io {
	SPA_PARAM_IO_START,
	SPA_PARAM_IO_id,	/**< type ID, uniquely identifies the io area (Id enum spa_io_type) */
	SPA_PARAM_IO_size,	/**< size of the io area (Int) */
};

/** properties for SPA_TYPE_OBJECT_ParamProfile */
enum spa_param_profile {
	SPA_PARAM_PROFILE_START,
	SPA_PARAM_PROFILE_index,	/**< profile index (Int) */
	SPA_PARAM_PROFILE_name,		/**< profile name (String) */
};

enum spa_param_port_config_mode {
	SPA_PARAM_PORT_CONFIG_MODE_none,	/**< no configuration */
	SPA_PARAM_PORT_CONFIG_MODE_passthrough,	/**< passthrough configuration */
	SPA_PARAM_PORT_CONFIG_MODE_convert,	/**< convert configuration */
	SPA_PARAM_PORT_CONFIG_MODE_dsp,		/**< dsp configuration, depending on the external
						  *  format. For audio, ports will be configured for
						  *  the given number of channels with F32 format. */
};

/** properties for SPA_TYPE_OBJECT_ParamPortConfig */
enum spa_param_port_config {
	SPA_PARAM_PORT_CONFIG_START,
	SPA_PARAM_PORT_CONFIG_direction,	/**< direction, input/output (Id enum spa_direction) */
	SPA_PARAM_PORT_CONFIG_mode,		/**< (Id enum spa_param_port_config_mode) mode */
	SPA_PARAM_PORT_CONFIG_monitor,		/**< (Bool) enable monitor output ports on input ports */
	SPA_PARAM_PORT_CONFIG_format,		/**< (Object) format filter */
};

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* SPA_PARAM_H */
