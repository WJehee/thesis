#include <stdio.h>

#include <FreeRTOS.h>
#include <task.h>

#include <csp/csp.h>
#include <csp/arch/csp_time.h>
#include <param/param.h>

#include "obc_config.h"
#include "measure_performance.h"
#include "crypto.h"
#include "ascon/api.h"

#define OBC1 12
#define OBC2 13
#define PORT 16

int _crypt = 1;
int _fake_header = 0;
int _msg_len = 100;
int _batch_size = 50;

PARAM_DEFINE_STATIC_RAM(103, msg_len, PARAM_TYPE_UINT8, -1, sizeof(uint8_t), PM_CONF, NULL, "", &_msg_len, "");
PARAM_DEFINE_STATIC_RAM(104, batch_size, PARAM_TYPE_UINT8, -1, sizeof(uint8_t), PM_CONF, NULL, "", &_batch_size, "");
PARAM_DEFINE_STATIC_RAM(105, crypt, PARAM_TYPE_UINT8, -1, sizeof(uint8_t), PM_CONF, NULL, "", &_crypt, "");
PARAM_DEFINE_STATIC_RAM(106, fake_header, PARAM_TYPE_UINT8, -1, sizeof(uint8_t), PM_CONF, NULL, "", &_fake_header, "");

void init_obc(bool send) {
    init_crypto(1);
    if (send) {
        printf("OBC sending...\n");
        xTaskCreate(send_task, "Send", 2000, NULL, 1, NULL);
    } else {
        printf("OBC receiving...\n");
        xTaskCreate(recv_task, "Receive", 2000, NULL, 1, NULL);
    }
}

void recv_task(void* params) {
    #ifdef SIMULATOR
        csp_bind_callback(echo_callback, PORT);
    #else
        csp_socket_t* sock = csp_socket(CSP_SO_NONE);
        csp_socket_set_callback(sock, echo_callback);
        csp_bind(sock, PORT);
    #endif

    for (;;) {}
}

void echo_callback(csp_packet_t* packet) {
    if (_crypt == 1) {
        if (decrypt_packet(packet) != VALID) {
            return;
        };
    }
    encrypt_packet(packet);
    #ifdef SIMULATOR
        csp_sendto_reply(packet, packet, CSP_SO_SAME);
    #else
        csp_sendto_reply(packet, packet, CSP_SO_SAME, 1000);
    #endif
}

void send_recv(csp_conn_t* conn) {
    csp_packet_t* packet = prepare_packet(_crypt == 1);

    #ifdef SIMULATOR
        csp_send(conn, packet);
    #else
        csp_send(conn, packet, 1000);
    #endif   

    packet = csp_read(conn, 1000); 
    if (packet != NULL && _crypt == 1) {
        if (decrypt_packet(packet) == VALID) {
            printf("Packet: %s\n", packet->data);
        }
    }
    csp_buffer_free(packet);
}

void send_task(void* params) {
    csp_conn_t* conn = csp_connect(CSP_PRIO_NORM, OBC2, PORT, 1000, CSP_O_CRC32);
    if (conn == NULL) {
        printf("Connection failed\n");
    }
    for (;;) {
        int start_msecs = csp_get_ms();

        for (int i=0; i<_batch_size; i++) {
            send_recv(conn);
        }

        int msecs = csp_get_ms() - start_msecs;
        printf("Time taken: %d\n", msecs);
        vTaskDelay(100);
    }
    csp_close(conn);
}

csp_packet_t* prepare_packet(bool encrypt) {
    int len = _msg_len;
    csp_packet_t * packet = csp_buffer_get(len);
    if (packet == NULL) {
        printf("Failed to get CSP buffer\n");
    }
    for (int i=0; i<len; i++) {
        packet->data[i] = 'A';
    }
    packet->length = len;

    if (encrypt) {
        encrypt_packet(packet);
    } else {
#define HEADERSIZE CRYPTO_ABYTES
        if (_fake_header == 1) {
            for (int i=packet->length; i<packet->length+HEADERSIZE; i++) {
                packet->data[i] = 'Z';
            }
            packet->length = packet->length+HEADERSIZE;
        }
    }
    return packet;
}

#ifndef SIMULATOR
void init_custom(void) {
    #ifdef SEND_TASK
        // True for sender (measures time), node 12 (OBC1)
        init_obc(true);
    #endif
    #ifdef RECV_TASK
        // False for receiver (decrypts, encrypts, replies), node 13 (OBC2)
        init_obc(false);
    #endif
}

void my_delay(void) {
    vTaskDelay(0);
}
#endif

