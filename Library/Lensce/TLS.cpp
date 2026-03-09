#include "TLS.h"

#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>

namespace Lensce {
	namespace TLS {

		std::string base64Encode(const std::vector<BYTE>& data) {
			BIO* bio;
			BIO* b64;
			BUF_MEM* bufferPtr;

			b64 = BIO_new(BIO_f_base64());
			bio = BIO_new(BIO_s_mem());

			// Disable newlines in output (Steam expects one continuous string)
			BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

			// Chain them: b64 filter writes into the memory bio
			bio = BIO_push(b64, bio);

			BIO_write(bio, &data[0], data.size());
			BIO_flush(bio);
			BIO_get_mem_ptr(bio, &bufferPtr);

			std::string result(bufferPtr->data, bufferPtr->length);

			BIO_free_all(bio);
			return result;
		}

	}
}
