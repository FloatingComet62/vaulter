#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/aes.h>
#include "lib.h"

void encrypt_file(int in, int out, skey skey) {
  int _;
  EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

  if (!ctx) {
    fprintf(stderr, "Failed to create cipher context\n");
    return;
  }

  if (EVP_EncryptInit_ex(
    ctx,
    EVP_aes_256_cbc(),
    NULL,
    skey.key,
    skey.iv
  ) != 1) {
    fprintf(stderr, "Encryption initialization failed\n");
    EVP_CIPHER_CTX_free(ctx);
    return;
  }

  unsigned char in_buf[1024], out_buf[1024];
  int num_read, out_buf_len;

  while ((num_read = read(in, in_buf, 1024)) > 0) {
    if (EVP_EncryptUpdate(ctx, out_buf, &out_buf_len, in_buf, num_read) != 1) {
      fprintf(stderr, "Encryption update failed\n");
      break;
    }
    _ = write(out, out_buf, out_buf_len);
  }

  if (num_read == -1) {
    fprintf(stderr, "Reading input file failed\n");
  } else {
    if (EVP_EncryptFinal_ex(ctx, out_buf, &out_buf_len) != 1) {
      fprintf(stderr, "Encryption finalization failed\n");
    }
    _ = write(out, out_buf, out_buf_len);
  }

  EVP_CIPHER_CTX_free(ctx);
}

void decrypt_file(int in, int out, skey skey) {
  int _;
  EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

  if (!ctx) {
    fprintf(stderr, "Failed to create cipher context\n");
    return;
  }

  if (EVP_DecryptInit_ex(
    ctx,
    EVP_aes_256_cbc(),
    NULL,
    skey.key,
    skey.iv
  ) != 1) {
    fprintf(stderr, "Decryption initialization failed\n");
    EVP_CIPHER_CTX_free(ctx);
    return;
  }

  unsigned char in_buf[1024], out_buf[1024];
  int num_read, out_buf_len;

  while ((num_read = read(in, in_buf, 1024)) > 0) {
    if (EVP_DecryptUpdate(ctx, out_buf, &out_buf_len, in_buf, num_read) != 1) {
      fprintf(stderr, "Decryption update failed\n");
      break;
    }
    _ = write(out, out_buf, out_buf_len);
  }

  if (num_read == -1) {
    fprintf(stderr, "Reading input file failed\n");
  } else {
    if (EVP_DecryptFinal_ex(ctx, out_buf, &out_buf_len) != 1) {
      fprintf(stderr, "Decryption finalization failed\n");
    }
    _ = write(out, out_buf, out_buf_len);
  }

  EVP_CIPHER_CTX_free(ctx);
}
