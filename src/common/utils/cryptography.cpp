#include "string.hpp"
#include "cryptography.hpp"
#include "nt.hpp"

#include <gsl/gsl>

/// http://www.opensource.apple.com/source/CommonCrypto/CommonCrypto-55010/Source/libtomcrypt/doc/libTomCryptDoc.pdf

namespace utils::cryptography
{
	namespace
	{
		void initialize_math()
		{
			static bool initialized = false;
			if (!initialized)
			{
				initialized = true;
				ltc_mp = ltm_desc;
			}
		}
	}

	ecc::key::key()
	{
		ZeroMemory(&this->key_storage_, sizeof(this->key_storage_));
	}

	ecc::key::~key()
	{
		this->free();
	}

	ecc::key::key(key&& obj) noexcept
		: key()
	{
		this->operator=(std::move(obj));
	}

	ecc::key::key(const key& obj)
		: key()
	{
		this->operator=(obj);
	}

	ecc::key& ecc::key::operator=(key&& obj) noexcept
	{
		if (this != &obj)
		{
			std::memmove(&this->key_storage_, &obj.key_storage_, sizeof(this->key_storage_));
			ZeroMemory(&obj.key_storage_, sizeof(obj.key_storage_));
		}

		return *this;
	}

	ecc::key& ecc::key::operator=(const key& obj)
	{
		if (this != &obj && obj.is_valid())
		{
			this->deserialize(obj.serialize(obj.key_storage_.type));
		}

		return *this;
	}

	bool ecc::key::is_valid() const
	{
		return (!memory::is_set(&this->key_storage_, 0, sizeof(this->key_storage_)));
	}

	ecc_key* ecc::key::get()
	{
		return &this->key_storage_;
	}

	std::string ecc::key::get_public_key() const
	{
		uint8_t buffer[512] = {0};
		DWORD length = sizeof(buffer);

		if (ecc_ansi_x963_export(&this->key_storage_, buffer, &length) == CRYPT_OK)
		{
			return std::string(reinterpret_cast<char*>(buffer), length);
		}

		return {};
	}

	void ecc::key::set(const std::string& pub_key_buffer)
	{
		this->free();

		if (ecc_ansi_x963_import(reinterpret_cast<const uint8_t*>(pub_key_buffer.data()), ULONG(pub_key_buffer.size()),
		                         &this->key_storage_) != CRYPT_OK)
		{
			ZeroMemory(&this->key_storage_, sizeof(this->key_storage_));
		}
	}

	void ecc::key::deserialize(const std::string& key)
	{
		this->free();

		if (ecc_import(reinterpret_cast<const uint8_t*>(key.data()), ULONG(key.size()), &this->key_storage_) != CRYPT_OK
		)
		{
			ZeroMemory(&this->key_storage_, sizeof(this->key_storage_));
		}
	}

	std::string ecc::key::serialize(const int type) const
	{
		uint8_t buffer[4096] = {0};
		DWORD length = sizeof(buffer);

		if (ecc_export(buffer, &length, type, &this->key_storage_) == CRYPT_OK)
		{
			return std::string(reinterpret_cast<char*>(buffer), length);
		}

		return "";
	}

	void ecc::key::free()
	{
		if (this->is_valid())
		{
			ecc_free(&this->key_storage_);
		}

		ZeroMemory(&this->key_storage_, sizeof(this->key_storage_));
	}

	bool ecc::key::operator==(key& key) const
	{
		return (this->is_valid() && key.is_valid() && this->serialize(PK_PUBLIC) == key.serialize(PK_PUBLIC));
	}

	uint64_t ecc::key::get_hash() const
	{
		auto hash = sha1::compute(this->get_public_key());
		if (hash.size() >= 8)
		{
			return *reinterpret_cast<uint64_t*>(const_cast<char*>(hash.data()));
		}

		return 0;
	}

	ecc::key ecc::generate_key(const int bits)
	{
		key key;

		initialize_math();
		register_prng(&sprng_desc);
		ecc_make_key(nullptr, find_prng("sprng"), bits / 8, key.get());

		return key;
	}

	ecc::key ecc::generate_key(const int bits, const std::string& entropy)
	{
		key key;

		initialize_math();

		const auto state = std::make_unique<prng_state>();
		register_prng(&yarrow_desc);
		yarrow_start(state.get());

		yarrow_add_entropy(reinterpret_cast<const uint8_t*>(entropy.data()), static_cast<unsigned long>(entropy.size()),
		                   state.get());
		yarrow_ready(state.get());

		ecc_make_key(state.get(), find_prng("yarrow"), bits / 8, key.get());
		yarrow_done(state.get());

		return key;
	}

	std::string ecc::sign_message(key& key, const std::string& message)
	{
		if (!key.is_valid()) return "";

		uint8_t buffer[512];
		DWORD length = sizeof(buffer);

		initialize_math();
		register_prng(&sprng_desc);
		ecc_sign_hash(reinterpret_cast<const uint8_t*>(message.data()), ULONG(message.size()), buffer, &length, nullptr,
		              find_prng("sprng"), key.get());

		return std::string(reinterpret_cast<char*>(buffer), length);
	}

	bool ecc::verify_message(key& key, const std::string& message, const std::string& signature)
	{
		if (!key.is_valid()) return false;

		initialize_math();

		auto result = 0;
		return (ecc_verify_hash(reinterpret_cast<const uint8_t*>(signature.data()), ULONG(signature.size()),
		                        reinterpret_cast<const uint8_t*>(message.data()), ULONG(message.size()), &result,
		                        key.get()) == CRYPT_OK && result != 0);
	}

	namespace rsa
	{
		namespace
		{
			void initialize()
			{
				static auto initialized = false;
				if (initialized) return;
				initialized = true;

				initialize_math();
				register_hash(&sha1_desc);
				register_prng(&yarrow_desc);
			}
		}
	}

	std::string rsa::encrypt(const std::string& data, const std::string& hash, const std::string& key)
	{
		initialize();

		const auto prng_id = find_prng("yarrow");

		rsa_key new_key;
		rsa_import(PBYTE(key.data()), ULONG(key.size()), &new_key);

		const auto yarrow = std::make_unique<prng_state>();
		rng_make_prng(128, prng_id, yarrow.get(), nullptr);

		unsigned char buffer[0x80];
		unsigned long length = sizeof(buffer);

		const auto rsa_result = rsa_encrypt_key( //
			PBYTE(data.data()), //
			ULONG(data.size()), //
			buffer, //
			&length, //
			PBYTE(hash.data()), //
			ULONG(hash.size()), //
			yarrow.get(), //
			prng_id, //
			find_hash("sha1"), //
			&new_key);

		rsa_free(&new_key);
		yarrow_done(yarrow.get());

		if (rsa_result == CRYPT_OK)
		{
			return std::string(PCHAR(buffer), length);
		}

		return {};
	}

	namespace des3
	{
		namespace
		{
			void initialize()
			{
				static auto initialized = false;
				if (initialized) return;
				initialized = true;

				register_cipher(&des3_desc);
			}
		}
	}

	std::string des3::encrypt(const std::string& data, const std::string& iv, const std::string& key)
	{
		initialize();

		std::string enc_data;
		enc_data.resize(data.size());

		symmetric_CBC cbc;
		const auto des3 = find_cipher("3des");

		cbc_start(des3, reinterpret_cast<const uint8_t*>(iv.data()), reinterpret_cast<const uint8_t*>(key.data()),
		          static_cast<int>(key.size()), 0, &cbc);
		cbc_encrypt(reinterpret_cast<const uint8_t*>(data.data()),
		            reinterpret_cast<uint8_t*>(const_cast<char*>(enc_data.data())), ULONG(data.size()), &cbc);
		cbc_done(&cbc);

		return enc_data;
	}

	std::string des3::decrypt(const std::string& data, const std::string& iv, const std::string& key)
	{
		initialize();

		std::string dec_data;
		dec_data.resize(data.size());

		symmetric_CBC cbc;
		const auto des3 = find_cipher("3des");

		cbc_start(des3, reinterpret_cast<const uint8_t*>(iv.data()), reinterpret_cast<const uint8_t*>(key.data()),
		          static_cast<int>(key.size()), 0, &cbc);
		cbc_decrypt(reinterpret_cast<const uint8_t*>(data.data()),
		            reinterpret_cast<uint8_t*>(const_cast<char*>(dec_data.data())), ULONG(data.size()), &cbc);
		cbc_done(&cbc);

		return dec_data;
	}

	std::string tiger::compute(const std::string& data, const bool hex)
	{
		return compute(reinterpret_cast<const uint8_t*>(data.data()), data.size(), hex);
	}

	std::string tiger::compute(const uint8_t* data, const size_t length, const bool hex)
	{
		uint8_t buffer[24] = {0};

		hash_state state;
		tiger_init(&state);
		tiger_process(&state, data, ULONG(length));
		tiger_done(&state, buffer);

		std::string hash(reinterpret_cast<char*>(buffer), sizeof(buffer));
		if (!hex) return hash;

		return string::dump_hex(hash, "");
	}

	namespace aes
	{
		namespace
		{
			void initialize()
			{
				static auto initialized = false;
				if (initialized) return;
				initialized = true;

				register_cipher(&aes_desc);
			}
		}
	}

	std::string aes::encrypt(const std::string& data, const std::string& iv, const std::string& key)
	{
		initialize();

		std::string enc_data;
		enc_data.resize(data.size());

		symmetric_CBC cbc;
		const auto aes = find_cipher("aes");

		cbc_start(aes, reinterpret_cast<const uint8_t*>(iv.data()), reinterpret_cast<const uint8_t*>(key.data()),
		          static_cast<int>(key.size()), 0, &cbc);
		cbc_encrypt(reinterpret_cast<const uint8_t*>(data.data()),
		            reinterpret_cast<uint8_t*>(const_cast<char*>(enc_data.data())),
		            static_cast<unsigned long>(data.size()), &cbc);
		cbc_done(&cbc);

		return enc_data;
	}

	std::string aes::decrypt(const std::string& data, const std::string& iv, const std::string& key)
	{
		initialize();

		std::string dec_data;
		dec_data.resize(data.size());

		symmetric_CBC cbc;
		const auto aes = find_cipher("aes");

		cbc_start(aes, reinterpret_cast<const uint8_t*>(iv.data()), reinterpret_cast<const uint8_t*>(key.data()),
		          static_cast<int>(key.size()), 0, &cbc);
		cbc_decrypt(reinterpret_cast<const uint8_t*>(data.data()),
		            reinterpret_cast<uint8_t*>(const_cast<char*>(dec_data.data())),
		            static_cast<unsigned long>(data.size()), &cbc);
		cbc_done(&cbc);

		return dec_data;
	}

	namespace hmac_sha1
	{
		namespace
		{
			void initialize()
			{
				static auto initialized = false;
				if (initialized) return;
				initialized = true;

				register_hash(&sha1_desc);
			}
		}
	}

	std::string hmac_sha1::process(const std::string& data, const std::string& key, unsigned int* len)
	{
		if (*len > 20)
		{
			printf("hmac error\n");
			return "";
		}
		initialize();

		std::string buffer;
		buffer.resize(*len);

		hmac_state state;
		const auto sha1 = find_hash("sha1");
		//hmac_memory
		hmac_init(&state, sha1, reinterpret_cast<const unsigned char*>(key.data()),
		          static_cast<unsigned long>(key.size()));
		hmac_process(&state, reinterpret_cast<const unsigned char*>(data.data()), static_cast<int>(data.size()));

		unsigned long outlen = *len;
		hmac_done(&state, reinterpret_cast<unsigned char*>(buffer.data()), &outlen);

		*len = outlen;
		return buffer;
	}

	std::string sha1::compute(const std::string& data, const bool hex)
	{
		return compute(reinterpret_cast<const uint8_t*>(data.data()), data.size(), hex);
	}

	std::string sha1::compute(const uint8_t* data, const size_t length, const bool hex)
	{
		uint8_t buffer[20] = {0};

		hash_state state;
		sha1_init(&state);
		sha1_process(&state, data, ULONG(length));
		sha1_done(&state, buffer);

		std::string hash(reinterpret_cast<char*>(buffer), sizeof(buffer));
		if (!hex) return hash;

		return string::dump_hex(hash, "");
	}

	std::string sha256::compute(const std::string& data, const bool hex)
	{
		return compute(reinterpret_cast<const uint8_t*>(data.data()), data.size(), hex);
	}

	std::string sha256::compute(const uint8_t* data, const size_t length, const bool hex)
	{
		uint8_t buffer[32] = {0};

		hash_state state;
		sha256_init(&state);
		sha256_process(&state, data, ULONG(length));
		sha256_done(&state, buffer);

		std::string hash(reinterpret_cast<char*>(buffer), sizeof(buffer));
		if (!hex) return hash;

		return string::dump_hex(hash, "");
	}

	std::string sha512::compute(const std::string& data, const bool hex)
	{
		return compute(reinterpret_cast<const uint8_t*>(data.data()), data.size(), hex);
	}

	std::string sha512::compute(const uint8_t* data, const size_t length, const bool hex)
	{
		uint8_t buffer[64] = {0};

		hash_state state;
		sha512_init(&state);
		sha512_process(&state, data, ULONG(length));
		sha512_done(&state, buffer);

		std::string hash(reinterpret_cast<char*>(buffer), sizeof(buffer));
		if (!hex) return hash;

		return string::dump_hex(hash, "");
	}

	std::string base64::encode(const uint8_t* data, const size_t len)
	{
		std::string result;
		result.resize((len + 2) * 2);

		auto out_len = static_cast<unsigned long>(result.size());
		if (base64_encode(data, static_cast<unsigned long>(len), result.data(), &out_len) != CRYPT_OK)
		{
			return {};
		}

		result.resize(out_len);
		return result;
	}

	std::string base64::encode(const std::string& data)
	{
		return base64::encode(reinterpret_cast<const unsigned char*>(data.data()), static_cast<unsigned>(data.size()));
	}

	std::string base64::decode(const std::string& data)
	{
		std::string result;
		result.resize((data.size() + 2) * 2);

		auto out_len = static_cast<unsigned long>(result.size());
		if (base64_decode(data.data(), static_cast<unsigned long>(data.size()),
		                  reinterpret_cast<uint8_t*>(result.data()), &out_len) != CRYPT_OK)
		{
			return {};
		}

		result.resize(out_len);
		return result;
	}

	unsigned int jenkins_one_at_a_time::compute(const std::string& data)
	{
		return compute(data.data(), data.size());
	}

	unsigned int jenkins_one_at_a_time::compute(const char* key, const size_t len)
	{
		unsigned int hash, i;
		for (hash = i = 0; i < len; ++i)
		{
			hash += key[i];
			hash += (hash << 10);
			hash ^= (hash >> 6);
		}
		hash += (hash << 3);
		hash ^= (hash >> 11);
		hash += (hash << 15);
		return hash;
	}

	namespace random
	{
		namespace
		{
			prng_state* get_prng_state()
			{
				static prng_state* state_ref = []()
				{
					static prng_state state;

					initialize_math();
					register_prng(&fortuna_desc);
					rng_make_prng(128, find_prng("fortuna"), &state, nullptr);

					int i[4]; // uninitialized data
					auto* i_ptr = &i;
					fortuna_add_entropy(reinterpret_cast<unsigned char*>(&i), sizeof(i), &state);
					fortuna_add_entropy(reinterpret_cast<unsigned char*>(&i_ptr), sizeof(i_ptr), &state);

					auto t = time(nullptr);
					fortuna_add_entropy(reinterpret_cast<unsigned char*>(&t), sizeof(t), &state);

					static const auto _ = gsl::finally([]()
					{
						fortuna_done(&state);
					});

					return &state;
				}();

				fortuna_ready(state_ref);
				return state_ref;
			}
		}
	}

	uint32_t random::get_integer()
	{
		uint32_t result;
		random::get_data(&result, sizeof(result));
		return result;
	}

	std::string random::get_challenge()
	{
		std::string result;
		result.resize(sizeof(uint32_t));
		random::get_data(result.data(), result.size());
		return string::dump_hex(result, "");
	}

	void random::get_data(void* data, const size_t size)
	{
		fortuna_read(static_cast<unsigned char*>(data), static_cast<unsigned long>(size), random::get_prng_state());
	}
}
