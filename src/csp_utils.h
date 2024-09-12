#include <stdio.h>
#include <stdlib.h>
#include <endian.h>
#include "arpa/inet.h"

#include <csp/arch/csp_time.h>
#include <csp/interfaces/csp_if_udp.h>
#include <csp/interfaces/csp_if_zmqhub.h>

#include <param/param.h>
#include <param/param_list.h>
#include <param/param_server.h>
#include <src/param/list/param_list.h>

#include "obc_config.h"
#include "unistd.h"

csp_iface_t* add_udp_interface(char* name, int node_id) {
    csp_iface_t *iface = malloc(sizeof(csp_iface_t));
    memset(iface, 0, sizeof(csp_iface_t));
    iface->netmask = inet_addr("255.255.255.0");
    iface->addr = node_id;
    iface->name = name;

    csp_if_udp_conf_t *ifconf = malloc(sizeof(csp_if_udp_conf_t));
    memset(ifconf, 0, sizeof(csp_if_udp_conf_t));

    ifconf->host = PEER_IP_ADDR;
    ifconf->lport = UDP_LISTEN_PORT;
    ifconf->rport = UDP_SEND_PORT;

    csp_if_udp_init(iface, ifconf);
    return iface;
}

csp_iface_t* add_zmq_interface(int node_id) {
    csp_iface_t* zmq_if;
    csp_zmqhub_init(node_id, CSP_ZMQHUB_ADDR, 0, &zmq_if);
    // Setting this is important, otherwise we don't reply
    zmq_if->is_default = 1;
    return zmq_if;
}

void rparam_list_handler(csp_conn_t * conn) {
    param_t * param;
    param_list_iterator i = {};
    while ((param = param_list_iterate(&i)) != NULL) {
        csp_packet_t * packet = csp_buffer_get(256);
        if (packet == NULL)
            break;

        memset(packet->data, 0, 256);

        param_transfer3_t * rparam = (void *) packet->data;
        int node = param->node;
        rparam->id = htobe16(param->id);
        rparam->node = htobe16(node);
        rparam->type = param->type;
        rparam->size = param->array_size;
        rparam->mask = htobe32(param->mask);

        strncpy(rparam->name, param->name, 35);

        if (param->vmem) {
            rparam->storage_type = param->vmem->type;
        }

        if (param->unit != NULL) {
            strncpy(rparam->unit, param->unit, 9);
        }
        int helplen = 0;
        if (param->docstr != NULL) {
            strncpy(rparam->help, param->docstr, 149);
            helplen = strnlen(param->docstr, 149);
        }
        packet->length = offsetof(param_transfer3_t, help) + helplen + 1;
        // Receiving on csp is inconsistent based on how fast you send, so we sleep here
        usleep(2000);
        csp_send(conn, packet);
    }
}

void vmem_server_loop(void* param) {
    static csp_socket_t vmem_server_socket = {0};

    csp_bind(&vmem_server_socket, PARAM_PORT_LIST);
    csp_listen(&vmem_server_socket, 3);

    csp_conn_t *conn;

    while (1) {
        if ((conn = csp_accept(&vmem_server_socket, CSP_MAX_DELAY)) == NULL) {
            continue;
        }
        if (csp_conn_dport(conn) == PARAM_PORT_LIST) {
            rparam_list_handler(conn);
            csp_close(conn);
            continue;
        }
        csp_close(conn);
    }
}
