#include <string.h>
#include <stdio.h>

#include "csp/csp_types.h"

#include "crypto.h"

#include "ascon/crypto_aead.h"
#include "ascon/api.h"

int SECURITY_HEADER_LENGTH = 4;
int AD_LEN = 0;

uint8_t key[CRYPTO_KEYBYTES] = {
    0xA3, 0x7F, 0x12, 0xD5, 0x8E, 0x4C, 0x29, 0x6B,
    0xF1, 0x02, 0xC4, 0x9A, 0x55, 0x3E, 0x71, 0xE8,
};

uint8_t public_number[CRYPTO_NPUBBYTES] = {
    0xA3, 0x7F, 0x12, 0xD5, 0x8E, 0x4C, 0x29, 0x6B,
    0xF1, 0x02, 0xC4, 0x9A, 0x55, 0x3E, 0x71, 0xE8,
};

void init_crypto(int csp_version) {
    if (csp_version == 1) {
        AD_LEN = 4;
    } else if (csp_version == 2) {
        AD_LEN = 6;
    } else {
        printf("Invalid CSP version, initialization failed\n");
        return;
    }
    SECURITY_HEADER_LENGTH = CRYPTO_KEYBYTES + AD_LEN;
    printf("Ascon initialized\n");
}

enum CryptoResult decrypt_packet(csp_packet_t* packet) {
    if (AD_LEN == 0) {
        printf("Crypto not initialized, not decrypting packet\n");
        return INVALID;
    }
    printf("Decrypt packet length: %d\n", packet->length);

    packet->length -= SECURITY_HEADER_LENGTH;
    unsigned long long length = 0;
    unsigned char decrypted[packet->length];

    if (crypto_aead_decrypt(
            // Plaintext
            decrypted, &length,
            // Secret message number
            0,
            // Cyphertext
            packet->data, packet->length,
            // Associated data
            packet->frame_begin, AD_LEN,
            // Public message number
            public_number,
            // Secret key
            key
        ) != 0) {
        printf("Message forged!\n");
        return INVALID;
    }
    packet->length = length;
    memcpy(packet->data, decrypted, packet->length);
    return VALID;
}

enum CryptoResult encrypt_packet(csp_packet_t* packet) {
    if (AD_LEN == 0) {
        printf("Crypto not initialized, not encrypting packet\n");
        return INVALID;
    }
    unsigned long long length = packet->length;
    unsigned char encrypted[packet->length+CRYPTO_ABYTES];

    crypto_aead_encrypt(
        // Cyphertext
        encrypted, &length,
        // Plaintext
        packet->data, packet->length,
        // Associated data,
        packet->frame_begin, AD_LEN,
        // Secret message number
        0,
        // Public message number
        public_number,
        // Secret key,
        key
    );
    packet->length = length;
    memcpy(packet->data, encrypted, packet->length); 
    packet->length += SECURITY_HEADER_LENGTH; 
    printf("Encrypt packet length: %d\n", packet->length);
    return VALID;
}

