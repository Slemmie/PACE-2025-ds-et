#pragma once

#ifdef HASH_TABLE_PD_DS
#include <ext/pb_ds/assoc_container.hpp>
#include <cstdint>
#include <chrono>

struct hash_table_hash {
	static inline uint64_t splitmix64(uint64_t x) {
		x += 0x9e3779b97f4a7c15;
		x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
		x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
		return x ^ (x >> 31);
	}

	inline size_t operator()(uint64_t x) const {
		static const uint64_t FIXED_RANDOM = std::chrono::steady_clock::now().time_since_epoch().count();
		return splitmix64(x + FIXED_RANDOM);
	}
};

template <typename K> using hash_set = __gnu_pbds::gp_hash_table <K, __gnu_pbds::null_type, hash_table_hash>;
template <typename K, typename V> using hash_map = __gnu_pbds::gp_hash_table <K, V, hash_table_hash>;

template <typename K> static constexpr bool contains(const hash_set <K>& hs, const K& key) { return hs.find(key) != hs.end(); }
template <typename K, typename V> static constexpr bool contains(const hash_map <K, V>& ht, const K& key) { return ht.find(key) != ht.end(); }
#else
#ifdef HASH_TABLE_ROBIN_HOOD
#include "../../3rdparty/robin_hood/robin_hood.h"

template <typename K> using hash_set = robin_hood::unordered_set <K>;
template <typename K, typename V> using hash_map = robin_hood::unordered_map <K, V>;

// template <typename K> static constexpr bool contains(const hash_set <K>& hs, const K& key) { return hs.contains(key); }
// template <typename K, typename V> static constexpr bool contains(const hash_map <K, V>& ht, const K& key) { return ht.contains(key); }
template <typename T, typename K> static constexpr bool contains(const T& ht, const K& key) { return ht.contains(key); }
#else
#include <unordered_set>
#include <unordered_map>

template <typename K> using hash_set = std::unordered_set <K>;
template <typename K, typename V> using hash_map = std::unordered_map <K, V>;

template <typename K> static constexpr bool contains(const hash_set <K>& hs, const K& key) { return hs.contains(key); }
template <typename K, typename V> static constexpr bool contains(const hash_map <K, V>& ht, const K& key) { return ht.contains(key); }

template <typename K> static constexpr void reserve(hash_set <K>& hs, size_t capacity) { hs.reserve(capacity); }
template <typename K, typename V> static constexpr void reserve(hash_map <K, V>& ht, size_t capacity) { ht.reserve(capacity); }
#endif
#endif
