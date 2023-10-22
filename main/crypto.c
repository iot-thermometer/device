#include "esp_system.h"
#include "mbedtls/aes.h"
#include "mbedtls/platform.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"

// char *encrypt_text(char *input_text, char *key_text)
// {
//     uint8_t input[16] = "mihrbvpmluyvbsyk";
//     uint8_t key[16] = "mihrbvpmluyvbsyk";
//     uint8_t output[33];
//     mbedtls_aes_context aes;
//     mbedtls_aes_init(&aes);

//     mbedtls_aes_setkey_enc(&aes, key, 128);

//     mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_ENCRYPT, input, output);

//     mbedtls_aes_free(&aes);

//     char *encoded = (char *)malloc(24 * sizeof(char));
//     for (int i = 0; i < 16; i++)
//     {
//         printf("%02x", output[i]);
//     }

//     printf("\nEncoded: %s\n", encoded);

//     return encoded;
// }

char *encrypt_text(const char *input, const char *key)
{
    mbedtls_aes_context aes;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    char output[512];
    int ret;

    // Initialize the structures
    mbedtls_aes_init(&aes);
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);

    // Seed the random number generator
    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, NULL, 0);
    if (ret != 0)
    {
        mbedtls_printf("Error seeding random number generator: %d\n", ret);
        return NULL;
    }

    // Set the AES key
    ret = mbedtls_aes_setkey_enc(&aes, (const unsigned char *)key, 128);
    if (ret != 0)
    {
        mbedtls_printf("Error setting AES key: %d\n", ret);
        return NULL;
    }
    size_t input_len = strlen(input);
    size_t block_size = 16; // AES block size is 16 bytes
    size_t num_blocks = (input_len + block_size - 1) / block_size;
    size_t output_len = num_blocks * block_size;

    // Encrypt the data block by block
    for (size_t i = 0; i < num_blocks; i++)
    {
        ret = mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_ENCRYPT, (const unsigned char *)(input + i * block_size), (unsigned char *)(output + i * block_size));
        if (ret != 0)
        {
            mbedtls_printf("Error encrypting data: %d\n", ret);
            return NULL;
        }
    }

    // Convert the encrypted data to a hexadecimal string
    char *hex_output = (char *)malloc(2 * output_len + 1);
    for (int i = 0; i < output_len; i++)
    {
        printf("%02x", output[i]);
        snprintf(hex_output + 2 * i, 3, "%02X", (unsigned char)output[i]);
    }

    // Clean up and return the result
    mbedtls_aes_free(&aes);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);

    // printf("Encrypted: %s\n", hex_output);
    return hex_output;
}