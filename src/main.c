#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>

#include <FreeRTOS.h>
#include <FreeRTOS_Sockets.h>
#include <FreeRTOSIPConfig.h>
#include <FreeRTOS_Routing.h>
#include <task.h>
#include <freertos_hooks.h>

#include <csp/csp.h>

#include <param/param.h>
#include <param/param_list.h>

#include <vmem/vmem.h>
#include <vmem/vmem_ram.h>

#include "freertos_required.h"
#include "obc_config.h"
#include "csp_utils.h"
#include "main_csp.h"


uint8_t _logging = 0;
uint8_t _logging_all = 0;
PARAM_DEFINE_STATIC_RAM(50, logging, PARAM_TYPE_UINT8, -1, sizeof(uint8_t), PM_CONF, NULL, "", &_logging, "Enable logging");
PARAM_DEFINE_STATIC_RAM(51, logging_all, PARAM_TYPE_UINT8, -1, sizeof(uint8_t), PM_CONF, NULL, "", &_logging_all, "Log messages for all nodes instead of just this one");

int node_id = NODE_ID;
int csp_version = CSP_VERSION;

enum Interface interface = UDP;

bool setup_default = true;

int main(int argc, char** argv) {
    int opt;
    while((opt = getopt(argc, argv, "hdn:v:i:")) != -1) {
        switch(opt) {
            case 'n': node_id = atoi(optarg); break;
            case 'v': csp_version = atoi(optarg); break;
            case 'i': interface = atoi(optarg); break;
            case 'd': setup_default = false; break;
            case 'h':
            default:
                printf(
                    "Usage\n"
                    "-n NODE_ID\t CSP node id to use\n"
                    "-v CSP_VERSION\t CSP version to use (1 or 2)\n"
                    "-i\t\t Select CSP interface: 0 = UDP, 1 = ZMQ\n"
                    "-d\t\t Disable default handlers\n"
                    "-h\t\t Show this help menu\n"
                    );
                return 0;
        } 
    }
    initialize();
    return 0;
}

// Gets called after network setup
void vApplicationIPNetworkEventHook_Multi(
        eIPCallbackEvent_t eNetworkEvent,
        struct xNetworkEndPoint *pxEndPoint
        ) {
    uint32_t ulIPAddress, ulNetMask, ulGatewayAddress, ulDNSServerAddress;
    char cBuffer[ 16 ];
    static BaseType_t xTasksAlreadyCreated = pdFALSE;

    /* Both eNetworkUp and eNetworkDown events can be processed here. */
    if( eNetworkEvent == eNetworkUp ) {
        if( xTasksAlreadyCreated == pdFALSE ) {
            // Initialize CSP and start core application
            setup_csp();
            xTasksAlreadyCreated = pdTRUE;
        }
        /* Print out the network configuration */
        FreeRTOS_GetEndPointConfiguration( &ulIPAddress, &ulNetMask, &ulGatewayAddress, &ulDNSServerAddress, pxNetworkEndPoints );
        FreeRTOS_inet_ntoa( ulIPAddress, cBuffer );
        printf("IP Address: %s\r\n", cBuffer);

        FreeRTOS_inet_ntoa( ulNetMask, cBuffer );
        printf("Subnet Mask: %s\r\n", cBuffer);

        FreeRTOS_inet_ntoa( ulGatewayAddress, cBuffer );
        printf("Gateway Address: %s\r\n", cBuffer);

        FreeRTOS_inet_ntoa( ulDNSServerAddress, cBuffer );
        printf("DNS Server Address: %s\r\n\r\n", cBuffer);
    }
}

void setup_csp(void) {
    printf("Initialising CSP version %d interface, node ID = %d\n", csp_version, node_id);
    csp_conf_t *conf = csp_get_conf();

    conf->hostname = "OBC";
    conf->version = csp_version;
    csp_init();

    csp_iface_t* iface;
    switch (interface) {
        case UDP:
            iface = add_udp_interface("UDP-1", node_id);
            break;
        case ZMQ:
            iface = add_zmq_interface(node_id);
            break;
    }

    xTaskCreate(route_work, "CSP route work",
            STACK_SIZE, NULL, TASK_PRIORITY, NULL);

    if (setup_default) {
        setup_default_handlers();
    }

    csp_iflist_print();
}

void setup_default_handlers(void) {
    for (int i=0; i<=6; i++) {
        csp_bind_callback(csp_service_handler, i);
    }
    csp_bind_callback(param_serve, PARAM_PORT_SERVER);

    xTaskCreate(vmem_server_loop, "List download handler",
            STACK_SIZE, NULL, TASK_PRIORITY, NULL);
}

void route_work(void* params) {
    for (;;) {
        csp_route_work();
    }
}

void csp_input_hook(csp_iface_t * iface, csp_packet_t * packet) {
    if (_logging != 0) {
        if (packet->id.dst == node_id || _logging_all != 1) {
            printf("INP: S %u, D %u, Dp %u, Sp %u, Pr %u, Fl 0x%02X, Sz %" PRIu16 " VIA: %s, Tms %u\n",
                    packet->id.src, packet->id.dst, packet->id.dport,
                    packet->id.sport, packet->id.pri, packet->id.flags, packet->length, iface->name, csp_get_ms());
        }
    }
}

void csp_output_hook(csp_id_t *idout, csp_packet_t *packet, csp_iface_t *iface, uint16_t via, int from_me) {
    if (_logging != 0) {
        printf("OUT: S %u, D %u, Dp %u, Sp %u, Pr %u, Fl 0x%02X, Sz %u VIA: %s (%u), Tms %u\n",
                idout->src, idout->dst, idout->dport, idout->sport,
                idout->pri, idout->flags, packet->length, iface->name,
                (via != CSP_NO_VIA_ADDRESS) ? via : idout->dst, csp_get_ms());
    }
}

// Required for libparam
int serial_get(void) {
    return 0;
}

