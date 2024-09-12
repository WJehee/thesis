project('obc-simulator', 'c',
  subproject_dir: 'lib',
  version : '0.1',
  default_options : [
    'buildtype=minsize',
	'c_std=gnu99',
    'warning_level=3',
    'optimization=3',
    'default_library=static',

    'csp:qfifo_len=100',
    'csp:conn_rxqueue_len=100',
    'csp:buffer_count=100',
    'csp:conn_max=100',
    'csp:port_max_bind=32',
  ]
)
sources = []
includes = []
c_args = []
link_args = []
dependencies = []

dependencies += dependency('csp', fallback: ['csp', 'csp_dep'])
dependencies += dependency('slirp', fallback: ['slirp', 'libslirp_dep'])
dependencies += dependency('freertos', fallback: ['freertos', 'freertos_dep'])
dependencies += dependency('freertos_tcp', fallback: ['freertos_tcp', 'freertos_tcp_dep'])
dependencies += dependency('param',
  fallback: ['param', 'param_dep'],
  default_options: [
    'list_dynamic=true',
  ],
)

src_dir = 'src'

sources += [
  src_dir / 'main.c',
  src_dir / 'freertos/freertos_required.c',
]

subdir(src_dir / 'aead')

includes += include_directories([
  src_dir,
  src_dir / 'freertos',
  src_dir / 'hardware',
])

executable(
  'obc-simulator',
  [ sources ],
  include_directories: [ includes ],
  dependencies: dependencies,
  install : true
)

