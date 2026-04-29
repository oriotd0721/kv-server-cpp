#pragma once
#include <optional>
#include <string>
#include <unordered_map>
#include <mutex>

class KVStore {
public:
    void set(const std::string& key, const std::string& value);
    std::optional<std::string> get(const std::string& key) const;
    bool del(const std::string& key);

private:
    // mutable so const methods (get) can still lock without a const_cast.
    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::string> store_;
};
