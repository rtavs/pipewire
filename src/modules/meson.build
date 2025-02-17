subdir('spa')

pipewire_module_c_args = [
  '-DHAVE_CONFIG_H',
  '-D_GNU_SOURCE',
]

pipewire_module_access = shared_library('pipewire-module-access', [ 'module-access.c' ],
  c_args : pipewire_module_c_args,
  include_directories : [configinc, spa_inc],
  install : true,
  install_dir : modules_install_dir,
  dependencies : [mathlib, dl_lib, pipewire_dep],
)

if dbus_dep.found()
pipewire_module_rtkit = shared_library('pipewire-module-rtkit', [ 'module-rtkit.c' ],
  c_args : pipewire_module_c_args,
  include_directories : [configinc, spa_inc],
  install : true,
  install_dir : modules_install_dir,
  dependencies : [dbus_dep, mathlib, dl_lib, pipewire_dep],
)
endif

pipewire_module_client_node = shared_library('pipewire-module-client-device',
  [ 'module-client-device.c',
    'module-client-device/resource-device.c',
    'module-client-device/proxy-device.c',
    'module-client-device/protocol-native.c', ],
  c_args : pipewire_module_c_args,
  include_directories : [configinc, spa_inc],
  install : true,
  install_dir : modules_install_dir,
  dependencies : [mathlib, dl_lib, pipewire_dep],
)

pipewire_module_link_factory = shared_library('pipewire-module-link-factory',
  [ 'module-link-factory.c' ],
  c_args : pipewire_module_c_args,
  include_directories : [configinc, spa_inc],
  install : true,
  install_dir : modules_install_dir,
  dependencies : [mathlib, dl_lib, pipewire_dep],
)

pipewire_module_protocol_native_deps = [mathlib, dl_lib, pipewire_dep]

if get_option('systemd')
  pipewire_module_protocol_native_deps += systemd_dep
endif

pipewire_module_protocol_native = shared_library('pipewire-module-protocol-native',
  [ 'module-protocol-native.c',
    'module-protocol-native/local-socket.c',
    'module-protocol-native/portal-screencast.c',
    'module-protocol-native/protocol-native.c',
    'module-protocol-native/v0/protocol-native.c',
    'module-protocol-native/connection.c' ],
  c_args : pipewire_module_c_args,
  include_directories : [configinc, spa_inc],
  install : true,
  install_dir : modules_install_dir,
  dependencies : pipewire_module_protocol_native_deps,
)

pipewire_module_client_node = shared_library('pipewire-module-client-node',
  [ 'module-client-node.c',
    'module-client-node/remote-node.c',
    'module-client-node/client-node.c',
    'module-client-node/protocol-native.c',
    'module-client-node/v0/client-node.c',
    'module-client-node/v0/transport.c',
    'module-client-node/v0/protocol-native.c',
    'spa/spa-node.c', ],
  c_args : pipewire_module_c_args,
  include_directories : [configinc, spa_inc],
  link_with : pipewire_module_protocol_native,
  install : true,
  install_dir : modules_install_dir,
  dependencies : [mathlib, dl_lib, pipewire_dep],
)

pipewire_module_metadata = shared_library('pipewire-module-metadata',
  [ 'module-metadata.c',
    'module-metadata/resource-metadata.c',
    'module-metadata/proxy-metadata.c',
    'module-metadata/metadata.c',
    'module-metadata/protocol-native.c'],
  c_args : pipewire_module_c_args,
  include_directories : [configinc, spa_inc],
  link_with : pipewire_module_protocol_native,
  install : true,
  install_dir : modules_install_dir,
  dependencies : [mathlib, dl_lib, pipewire_dep],
)

test('pw-test-protocol-native',
	executable('pw-test-protocol-native',
		[ 'module-protocol-native/test-connection.c',
		  'module-protocol-native/connection.c' ],
			c_args : libpipewire_c_args,
			include_directories : [configinc, spa_inc ],
			dependencies : [pipewire_dep],
			install : false),
	env : [
		'SPA_PLUGIN_DIR=@0@/spa/plugins/'.format(meson.build_root()),
		'PIPEWIRE_MODULE_DIR=@0@/src/modules/'.format(meson.build_root())
	])

pipewire_module_adapter = shared_library('pipewire-module-adapter',
  [ 'module-adapter.c',
    'module-adapter/adapter.c',
    'spa/spa-node.c' ],
  c_args : pipewire_module_c_args,
  include_directories : [configinc, spa_inc],
  install : true,
  install_dir : modules_install_dir,
  dependencies : [mathlib, dl_lib, rt_lib, pipewire_dep],
)
