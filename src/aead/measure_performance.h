#include "csp/csp_types.h"

void recv_task(void* params);
void send_task(void* params);

void echo_callback(csp_packet_t* packet);
csp_packet_t* prepare_packet(bool encrypt);

