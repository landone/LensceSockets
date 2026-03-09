#include "TLS.h"

#include <iostream>

#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/param_build.h>
#include <openssl/core_names.h>
#include <openssl/buffer.h>
#include <openssl/err.h>

namespace Lensce {
	namespace TLS
	{
		PrivateKey::PrivateKey(const std::string& modulusHex, const std::string& exponentHex) {
			
			EVP_PKEY* pkey = nullptr;
			EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_from_name(nullptr, "RSA", nullptr);
			OSSL_PARAM_BLD* paramBuilder = OSSL_PARAM_BLD_new();

			BIGNUM* modulus = BN_new();
			BIGNUM* exponent = BN_new();
			BN_hex2bn(&modulus, modulusHex.c_str());
			BN_hex2bn(&exponent, exponentHex.c_str());

			OSSL_PARAM_BLD_push_BN(paramBuilder, OSSL_PKEY_PARAM_RSA_N, modulus);
			OSSL_PARAM_BLD_push_BN(paramBuilder, OSSL_PKEY_PARAM_RSA_E, exponent);

			OSSL_PARAM* params = OSSL_PARAM_BLD_to_param(paramBuilder);

			EVP_PKEY_fromdata_init(ctx);
			if (EVP_PKEY_fromdata(ctx, &pkey, EVP_PKEY_PUBLIC_KEY, params) <= 0) {
				std::cerr << "Failed to create EVP_PKEY from RSA parameters: " << ERR_get_error() << std::endl;
				return;
			}
			EVP_PKEY_CTX_free(ctx);

			ctx = EVP_PKEY_CTX_new(pkey, nullptr);
			EVP_PKEY_encrypt_init(ctx);
			EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING);

			OSSL_PARAM_free(params);
			OSSL_PARAM_BLD_free(paramBuilder);
			BN_free(modulus);
			BN_free(exponent);
			EVP_PKEY_free(pkey);

			privateKeyContext = (void*)ctx;

		}

		PrivateKey::~PrivateKey() {
			if (privateKeyContext) {
				EVP_PKEY_CTX_free((EVP_PKEY_CTX*)privateKeyContext);
				privateKeyContext = nullptr;
			}
		}

		std::vector<BYTE> PrivateKey::encrypt(const std::vector<BYTE>& data) {

			size_t rsaSize = 0;
			EVP_PKEY_encrypt((EVP_PKEY_CTX*)privateKeyContext, nullptr, &rsaSize, (const unsigned char*)&data[0], data.size());
			std::vector<BYTE> encryptedPass(rsaSize);
			int result = EVP_PKEY_encrypt((EVP_PKEY_CTX*)privateKeyContext, &encryptedPass[0], &rsaSize, (const unsigned char*)&data[0], data.size());
			if (result == -1) {
				std::cerr << "RSA encryption failed: " << ERR_get_error() << std::endl;
			}
			return encryptedPass;
		}
	}
}