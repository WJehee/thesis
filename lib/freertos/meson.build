project('freertos', 'c')

src_dir = 'FreeRTOS-Kernel'

src = files([
    src_dir / 'croutine.c',
	src_dir / 'event_groups.c',
	src_dir / 'list.c',
    src_dir / 'stream_buffer.c',
	src_dir / 'queue.c',
	src_dir / 'tasks.c',
	src_dir / 'timers.c',

	src_dir / 'portable/MemMang/heap_3.c',
    src_dir / 'portable/ThirdParty/GCC/Posix/utils/wait_for_event.c',
    src_dir / 'portable/ThirdParty/GCC/Posix/port.c',
])

inc = include_directories([
	src_dir,
	src_dir / 'include',
    src_dir / 'portable/ThirdParty/GCC/Posix',
    src_dir / 'portable/ThirdParty/GCC/Posix/utils',
])

deps = []
# deps += dependency('start', fallback: ['start', 'start_dep'])

freertos_dep = declare_dependency(
	include_directories : inc,
	sources : src,
	dependencies: deps,
)

