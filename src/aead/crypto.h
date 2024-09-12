#include "csp/csp_types.h"

enum CryptoResult {
    VALID = 0,
    INVALID = 1,
};

void init_crypto(int csp_version);
enum CryptoResult decrypt_packet(csp_packet_t* packet);
enum CryptoResult encrypt_packet(csp_packet_t* packet);

