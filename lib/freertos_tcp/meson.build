project('freertos-tcp`', 'c')

src_dir = 'FreeRTOS-Plus-TCP/source'

inc = include_directories([
  src_dir,
  src_dir / 'include',
  src_dir / 'portable/Compiler/GCC',
  src_dir / 'portable/NetworkInterface/linux',
])

src = files([
  src_dir / 'FreeRTOS_ARP.c',
  src_dir / 'FreeRTOS_BitConfig.c',
  src_dir / 'FreeRTOS_DHCP.c',
  src_dir / 'FreeRTOS_DHCPv6.c',
  src_dir / 'FreeRTOS_DNS.c',
  src_dir / 'FreeRTOS_DNS_Cache.c',
  src_dir / 'FreeRTOS_DNS_Callback.c',
  src_dir / 'FreeRTOS_DNS_Networking.c',
  src_dir / 'FreeRTOS_DNS_Parser.c',
  src_dir / 'FreeRTOS_ICMP.c',
  src_dir / 'FreeRTOS_IP.c',
  src_dir / 'FreeRTOS_IP_Timers.c',
  src_dir / 'FreeRTOS_IP_Utils.c',
  src_dir / 'FreeRTOS_IPv4.c',
  src_dir / 'FreeRTOS_IPv4_Sockets.c',
  src_dir / 'FreeRTOS_IPv4_Utils.c',
  src_dir / 'FreeRTOS_IPv6.c',
  src_dir / 'FreeRTOS_IPv6_Sockets.c',
  src_dir / 'FreeRTOS_IPv6_Utils.c',
  src_dir / 'FreeRTOS_ND.c',
  src_dir / 'FreeRTOS_RA.c',
  src_dir / 'FreeRTOS_Routing.c',
  src_dir / 'FreeRTOS_Sockets.c',
  src_dir / 'FreeRTOS_Stream_Buffer.c',
  src_dir / 'FreeRTOS_TCP_IP.c',
  src_dir / 'FreeRTOS_TCP_IP_IPv4.c',
  src_dir / 'FreeRTOS_TCP_IP_IPv6.c',
  src_dir / 'FreeRTOS_TCP_Reception.c',
  src_dir / 'FreeRTOS_TCP_State_Handling.c',
  src_dir / 'FreeRTOS_TCP_State_Handling_IPv4.c',
  src_dir / 'FreeRTOS_TCP_State_Handling_IPv6.c',
  src_dir / 'FreeRTOS_TCP_Transmission.c',
  src_dir / 'FreeRTOS_TCP_Transmission_IPv4.c',
  src_dir / 'FreeRTOS_TCP_Transmission_IPv6.c',
  src_dir / 'FreeRTOS_TCP_Utils.c',
  src_dir / 'FreeRTOS_TCP_Utils_IPv4.c',
  src_dir / 'FreeRTOS_TCP_Utils_IPv6.c',
  src_dir / 'FreeRTOS_TCP_WIN.c',
  src_dir / 'FreeRTOS_Tiny_TCP.c',
  src_dir / 'FreeRTOS_UDP_IP.c',
  src_dir / 'FreeRTOS_UDP_IPv4.c',
  src_dir / 'FreeRTOS_UDP_IPv6.c',

  src_dir / 'portable/BufferManagement/BufferAllocation_2.c',
  src_dir / 'portable/NetworkInterface/libslirp/MBuffNetifBackendLibslirp.c',
  src_dir / 'portable/NetworkInterface/libslirp/MBuffNetworkInterface.c',
])

deps = []
deps += dependency('freertos', fallback: ['freertos', 'freertos_dep'])

freertos_tcp_dep = declare_dependency(
  	include_directories : inc,
	sources : src,
    dependencies: deps,
)

