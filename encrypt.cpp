#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <algorithm>
#include <stdexcept>
#include <numeric>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <windows.h>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <cmath>
#include <ctime>
#include <cstring>
#include <direct.h>
#include <io.h>

// ============================================================
// 高速加密解密一体机 v6.1 - 库版本
// ============================================================

// --- 安全参数 ---
namespace SecuritySpecs {
	constexpr int KEY_SIZE = 32;              // 256位密钥
	constexpr int SALT_SIZE = 32;             // 256位盐
	constexpr int IV_SIZE = 16;               // 128位IV
	constexpr int PBKDF2_ITERATIONS = 1000;   // 1000次迭代（更快）
	constexpr int ENCRYPTION_ROUNDS = 8;      // 8轮加密
	constexpr int WHITENING_SIZE = 16;        
	constexpr int MAC_SIZE = 32;              
	constexpr int MIN_PASSWORD_LENGTH = 6;    
	constexpr int MAX_PASSWORD_LENGTH = 128;  
}

// --- 硬件检测 ---
class HardwareDetector {
private:
	int cpu_cores;
	
public:
	HardwareDetector() {
		SYSTEM_INFO sysInfo;
		GetSystemInfo(&sysInfo);
		cpu_cores = sysInfo.dwNumberOfProcessors;
		if (cpu_cores < 1) cpu_cores = 1;
	}
	
	int getCores() const { return cpu_cores; }
};

// --- 快速随机数生成器 ---
class FastRandom {
private:
	std::mt19937_64 gen;
	std::uniform_int_distribution<uint8_t> dis8;
	std::mutex mutex;
	
	uint64_t getEntropy() {
		uint64_t entropy = 0;
		entropy ^= std::chrono::high_resolution_clock::now().time_since_epoch().count();
		entropy ^= GetTickCount();
		entropy ^= GetCurrentProcessId();
		entropy ^= GetCurrentThreadId();
		LARGE_INTEGER perfCounter;
		QueryPerformanceCounter(&perfCounter);
		entropy ^= perfCounter.QuadPart;
		return entropy;
	}
	
public:
	FastRandom() {
		uint64_t seed = getEntropy() ^ 0x9e3779b97f4a7c15ULL;
		gen.seed(seed);
	}
	
	uint8_t nextByte() {
		std::lock_guard<std::mutex> lock(mutex);
		return dis8(gen);
	}
	
	void fillBytes(std::vector<uint8_t>& buffer) {
		std::lock_guard<std::mutex> lock(mutex);
		for (auto& byte : buffer) {
			byte = dis8(gen);
		}
	}
};

// --- 快速哈希函数 ---
class FastHash {
private:
	uint64_t rotl64(uint64_t x, int r) {
		return (x << r) | (x >> (64 - r));
	}
	
public:
	std::vector<uint8_t> fastHash(const std::vector<uint8_t>& input) {
		std::vector<uint8_t> digest(32, 0);
		
		uint64_t h1 = 0x9e3779b97f4a7c15ULL;
		uint64_t h2 = 0xbf58476d1ce4e5b9ULL;
		
		for (size_t i = 0; i < input.size(); i += 8) {
			uint64_t block = 0;
			for (size_t j = 0; j < 8 && i + j < input.size(); j++) {
				block |= static_cast<uint64_t>(input[i + j]) << (j * 8);
			}
			
			h1 ^= block;
			h2 ^= rotl64(block, 17);
			
			h1 = rotl64(h1, 13) ^ h2;
			h2 = rotl64(h2, 19) ^ h1;
		}
		
		for (int i = 0; i < 8; i++) {
			uint64_t mix = h1 ^ h2;
			mix = rotl64(mix, i * 8);
			for (int j = 0; j < 4; j++) {
				digest[i * 4 + j] = static_cast<uint8_t>((mix >> (j * 8)) & 0xFF);
			}
			h1 = rotl64(h1, 7) ^ h2;
			h2 = rotl64(h2, 11) ^ h1;
		}
		
		return digest;
	}
};

// --- 快速密钥派生 ---
std::vector<uint8_t> fastKDF(const std::string& password, 
							 const std::vector<uint8_t>& salt, 
							 int iterations) {
	FastHash fh;
	std::vector<uint8_t> key(SecuritySpecs::KEY_SIZE, 0);
	
	if (password.length() < SecuritySpecs::MIN_PASSWORD_LENGTH) {
		throw std::runtime_error("密码长度至少为6个字符");
	}
	
	std::vector<uint8_t> combined;
	combined.reserve(password.length() + salt.size());
	combined.insert(combined.end(), password.begin(), password.end());
	combined.insert(combined.end(), salt.begin(), salt.end());
	
	std::vector<uint8_t> current = fh.fastHash(combined);
	
	for (int i = 0; i < iterations; i++) {
		std::vector<uint8_t> temp;
		temp.reserve(current.size() + salt.size() + password.length());
		temp.insert(temp.end(), current.begin(), current.end());
		temp.insert(temp.end(), salt.begin(), salt.end());
		temp.insert(temp.end(), password.begin(), password.end());
		current = fh.fastHash(temp);
		
		for (size_t j = 0; j < SecuritySpecs::KEY_SIZE && j < current.size(); j++) {
			key[j] ^= current[j];
		}
	}
	
	return key;
}

// --- 快速加密核心 ---
class FastCipher {
private:
	std::vector<uint8_t> round_keys[SecuritySpecs::ENCRYPTION_ROUNDS];
	std::vector<uint8_t> sboxes[SecuritySpecs::ENCRYPTION_ROUNDS];
	std::vector<uint8_t> inv_sboxes[SecuritySpecs::ENCRYPTION_ROUNDS];
	
	std::vector<uint8_t> generateSBox(const std::vector<uint8_t>& key) {
		std::vector<uint8_t> sbox(256);
		std::iota(sbox.begin(), sbox.end(), 0);
		
		unsigned int seed = 0;
		for (size_t i = 0; i < key.size() && i < 32; i++) {
			seed = seed * 31 + key[i];
		}
		std::mt19937 rng(seed);
		
		for (int i = 255; i > 0; i--) {
			std::uniform_int_distribution<int> dist(0, i);
			int j = dist(rng);
			std::swap(sbox[i], sbox[j]);
		}
		
		return sbox;
	}
	
public:
	FastCipher(const std::vector<uint8_t>& master_key) {
		FastHash fh;
		std::vector<uint8_t> current_key = master_key;
		
		for (int round = 0; round < SecuritySpecs::ENCRYPTION_ROUNDS; round++) {
			current_key = fh.fastHash(current_key);
			round_keys[round] = current_key;
			
			sboxes[round] = generateSBox(current_key);
			
			std::vector<uint8_t> inv_sbox(256);
			for (int i = 0; i < 256; i++) {
				inv_sbox[sboxes[round][i]] = i;
			}
			inv_sboxes[round] = inv_sbox;
		}
	}
	
	void encryptBlock(std::vector<uint8_t>& data) {
		if (data.empty()) return;
		
		std::vector<uint8_t> state = data;
		size_t block_size = data.size();
		
		for (int round = 0; round < SecuritySpecs::ENCRYPTION_ROUNDS; round++) {
			const auto& rkey = round_keys[round];
			const auto& sbox = sboxes[round];
			size_t key_size = rkey.size();
			
			// 轮密钥加
			for (size_t i = 0; i < block_size; i++) {
				state[i] ^= rkey[i % key_size];
			}
			
			// S盒替换
			for (size_t i = 0; i < block_size; i++) {
				state[i] = sbox[state[i]];
			}
			
			// 简单混淆
			for (size_t i = 0; i < block_size; i++) {
				state[i] ^= static_cast<uint8_t>((round * 0x9E + i * 0x37) & 0xFF);
				if (i > 0) {
					state[i] ^= state[i-1];
				}
			}
		}
		
		data = state;
	}
	
	void decryptBlock(std::vector<uint8_t>& data) {
		if (data.empty()) return;
		
		std::vector<uint8_t> state = data;
		size_t block_size = data.size();
		
		for (int round = SecuritySpecs::ENCRYPTION_ROUNDS - 1; round >= 0; round--) {
			const auto& rkey = round_keys[round];
			const auto& inv_sbox = inv_sboxes[round];
			size_t key_size = rkey.size();
			
			// 逆混淆
			for (size_t i = block_size - 1; i > 0; i--) {
				state[i] ^= state[i-1];
			}
			for (size_t i = 0; i < block_size; i++) {
				state[i] ^= static_cast<uint8_t>((round * 0x9E + i * 0x37) & 0xFF);
			}
			
			// 逆S盒
			for (size_t i = 0; i < block_size; i++) {
				state[i] = inv_sbox[state[i]];
			}
			
			// 逆轮密钥加
			for (size_t i = 0; i < block_size; i++) {
				state[i] ^= rkey[i % key_size];
			}
		}
		
		data = state;
	}
};

// --- 填充 ---
void padData(std::vector<uint8_t>& data) {
	if (data.empty()) {
		data.push_back(16);
		for (int i = 1; i < 16; i++) data.push_back(16);
		return;
	}
	size_t padding_size = 16 - (data.size() % 16);
	if (padding_size == 0) padding_size = 16;
	data.insert(data.end(), padding_size, static_cast<uint8_t>(padding_size));
}

void unpadData(std::vector<uint8_t>& data) {
	if (data.empty()) throw std::runtime_error("数据为空");
	uint8_t padding_size = data.back();
	if (padding_size > 16 || padding_size == 0) {
		throw std::runtime_error("无效的填充");
	}
	if (data.size() < padding_size) {
		throw std::runtime_error("无效的填充");
	}
	for (size_t i = 0; i < padding_size; i++) {
		if (data[data.size() - 1 - i] != padding_size) {
			throw std::runtime_error("无效的填充");
		}
	}
	data.resize(data.size() - padding_size);
}

// ============================================================
// 对外暴露的加密解密函数（C风格接口，便于在f系统中调用）
// ============================================================

extern "C" {
	
// 加密配置数据（用于用户名、密码等短字符串）
	__declspec(dllexport) char* EncryptConfigString(const char* plaintext, const char* password) {
		try {
			if (!plaintext || !password || strlen(password) < 6) {
				return NULL;
			}
			
			std::string text(plaintext);
			std::string pwd(password);
			
			FastRandom fr;
			FastHash fh;
			
			std::vector<uint8_t> salt(SecuritySpecs::SALT_SIZE);
			fr.fillBytes(salt);
			
			std::vector<uint8_t> iv(SecuritySpecs::IV_SIZE);
			fr.fillBytes(iv);
			
			std::vector<uint8_t> key = fastKDF(pwd, salt, SecuritySpecs::PBKDF2_ITERATIONS);
			
			std::vector<uint8_t> data(text.begin(), text.end());
			padData(data);
			
			std::vector<uint8_t> header(16);
			fr.fillBytes(header);
			data.insert(data.begin(), header.begin(), header.end());
			
			std::vector<uint8_t> footer(16);
			fr.fillBytes(footer);
			data.insert(data.end(), footer.begin(), footer.end());
			
			std::vector<uint8_t> prev_block = iv;
			FastCipher cipher(key);
			
			for (size_t i = 0; i < data.size(); i += 16) {
				size_t block_size = (i + 16 < data.size()) ? 16 : (data.size() - i);
				std::vector<uint8_t> block(data.begin() + i, data.begin() + i + block_size);
				
				for (size_t j = 0; j < block.size(); j++) {
					block[j] ^= prev_block[j % prev_block.size()];
				}
				
				cipher.encryptBlock(block);
				
				for (size_t j = 0; j < block.size(); j++) {
					data[i + j] = block[j];
				}
				
				prev_block = block;
			}
			
			std::vector<uint8_t> mac(32);
			std::vector<uint8_t> mac_input = data;
			mac_input.insert(mac_input.end(), key.begin(), key.end());
			std::vector<uint8_t> mac_hash = fh.fastHash(mac_input);
			for (int i = 0; i < 32 && i < (int)mac_hash.size(); i++) {
				mac[i] = mac_hash[i] ^ mac_hash[(i + 16) % 32];
			}
			
			std::vector<uint8_t> ciphertext;
			ciphertext.reserve(SecuritySpecs::SALT_SIZE + SecuritySpecs::IV_SIZE + SecuritySpecs::MAC_SIZE + data.size());
			ciphertext.insert(ciphertext.end(), salt.begin(), salt.end());
			ciphertext.insert(ciphertext.end(), iv.begin(), iv.end());
			ciphertext.insert(ciphertext.end(), mac.begin(), mac.end());
			ciphertext.insert(ciphertext.end(), data.begin(), data.end());
			
			// 转换为十六进制字符串
			std::stringstream ss;
			ss << std::hex << std::setfill('0');
			for (uint8_t byte : ciphertext) {
				ss << std::setw(2) << static_cast<int>(byte);
			}
			std::string result = ss.str();
			
			char* output = (char*)malloc(result.length() + 1);
			if (output) {
				strcpy(output, result.c_str());
			}
			return output;
			
		} catch (...) {
			return NULL;
		}
	}
	
// 解密配置数据
	__declspec(dllexport) char* DecryptConfigString(const char* ciphertext_hex, const char* password) {
		try {
			if (!ciphertext_hex || !password || strlen(password) < 6) {
				return NULL;
			}
			
			std::string hex(ciphertext_hex);
			if (hex.length() % 2 != 0) {
				return NULL;
			}
			
			std::vector<uint8_t> ciphertext;
			for (size_t i = 0; i < hex.length(); i += 2) {
				std::string byteStr = hex.substr(i, 2);
				uint8_t byte = static_cast<uint8_t>(std::stoul(byteStr, nullptr, 16));
				ciphertext.push_back(byte);
			}
			
			const size_t header_size = SecuritySpecs::SALT_SIZE + 
			SecuritySpecs::IV_SIZE + 
			SecuritySpecs::MAC_SIZE;
			
			if (ciphertext.size() < header_size + 16) {
				throw std::runtime_error("无效的密文格式");
			}
			
			FastHash fh;
			
			std::vector<uint8_t> salt(ciphertext.begin(), 
									  ciphertext.begin() + SecuritySpecs::SALT_SIZE);
			std::vector<uint8_t> iv(ciphertext.begin() + SecuritySpecs::SALT_SIZE,
									ciphertext.begin() + SecuritySpecs::SALT_SIZE + SecuritySpecs::IV_SIZE);
			std::vector<uint8_t> mac(ciphertext.begin() + SecuritySpecs::SALT_SIZE + SecuritySpecs::IV_SIZE,
									 ciphertext.begin() + SecuritySpecs::SALT_SIZE + SecuritySpecs::IV_SIZE + SecuritySpecs::MAC_SIZE);
			std::vector<uint8_t> data(ciphertext.begin() + header_size, 
									  ciphertext.end());
			
			std::vector<uint8_t> key = fastKDF(password, salt, SecuritySpecs::PBKDF2_ITERATIONS);
			
			std::vector<uint8_t> mac_input = data;
			mac_input.insert(mac_input.end(), key.begin(), key.end());
			std::vector<uint8_t> mac_hash = fh.fastHash(mac_input);
			std::vector<uint8_t> calc_mac(32, 0);
			for (int i = 0; i < 32 && i < (int)mac_hash.size(); i++) {
				calc_mac[i] = mac_hash[i] ^ mac_hash[(i + 16) % 32];
			}
			
			if (calc_mac != mac) {
				throw std::runtime_error("MAC验证失败！密码错误或数据被篡改");
			}
			
			std::vector<uint8_t> prev_block = iv;
			FastCipher cipher(key);
			
			for (size_t i = 0; i < data.size(); i += 16) {
				size_t block_size = (i + 16 < data.size()) ? 16 : (data.size() - i);
				std::vector<uint8_t> block(data.begin() + i, data.begin() + i + block_size);
				std::vector<uint8_t> encrypted_block = block;
				
				cipher.decryptBlock(block);
				
				for (size_t j = 0; j < block.size(); j++) {
					block[j] ^= prev_block[j % prev_block.size()];
				}
				
				for (size_t j = 0; j < block.size(); j++) {
					data[i + j] = block[j];
				}
				
				prev_block = encrypted_block;
			}
			
			if (data.size() < 32) {
				throw std::runtime_error("数据损坏");
			}
			data.erase(data.begin(), data.begin() + 16);
			data.resize(data.size() - 16);
			
			unpadData(data);
			
			std::string plaintext(data.begin(), data.end());
			char* output = (char*)malloc(plaintext.length() + 1);
			if (output) {
				strcpy(output, plaintext.c_str());
			}
			return output;
			
		} catch (...) {
			return NULL;
		}
	}
	
// 释放内存
	__declspec(dllexport) void FreeEncryptMemory(char* ptr) {
		if (ptr) {
			free(ptr);
		}
	}
	
} // extern "C"

// 注意：不再有 main 函数
