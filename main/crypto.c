#include "esp_system.h"
#include "esp_log.h"
#include "mbedtls/aes.h"
#include "mbedtls/md5.h"
#include "mbedtls/base64.h"
#include "string.h"

char *encrypt_text(char *input_text, char *key_text)
{
    uint8_t input[16] = "mihrbvpmluyvbsyk";
    uint8_t key[16] = "mihrbvpmluyvbsyk";
    uint8_t output[33];
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);

    mbedtls_aes_setkey_enc(&aes, key, 128);

    mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_ENCRYPT, input, output);

    mbedtls_aes_free(&aes);

    char *encoded = (char *)malloc(24 * sizeof(char));
    for (int i = 0; i < 16; i++)
    {
        printf("%02x", output[i]);
    }

    printf("\nEncoded: %s\n", encoded);

    return encoded;
}
