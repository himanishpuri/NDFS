#include <iostream>
#include <fstream>
#include <cstdlib>
#include <thread>
#include <chrono>
#include "RedisUtil.h"
#include "Logger.h"
#include "GeneralFunctions.cpp"


RedisUtil::RedisUtil(std::string hostname, int port, int timeout) {
    this->hostname = hostname;
    this->port = port;
    this->timeout = timeout;
    this->redis = nullptr;

    load_env_file(".env");

    const char* env_host = std::getenv("REDIS_HOST");
    const char* env_port = std::getenv("REDIS_PORT");

    if (env_host) this->hostname = env_host;
    if (env_port) this->port = std::stoi(env_port);

    LOG_INFO("RedisUtil object created with hostname (%s) and port (%d)", this->hostname.c_str(), this->port);
    connect();
}

void RedisUtil::connect() {
    try {
        std::string redis_url = "tcp://" + this->hostname + ":" + std::to_string(this->port);
        this->redis = new sw::redis::Redis(redis_url);
        LOG_INFO("Connected to Redis at %s", redis_url.c_str());
    }
    catch (const std::exception& e) {
        LOG_ERROR("Redis connection failed: %s", e.what());
        exit(1);
    }
}

void RedisUtil::setKey(const std::string& key, const std::string& value) {
    try {
        this->redis->set(key, value);
        LOG_INFO("Key (%s) set to value (%s)", key.c_str(), value.c_str());
    }
    catch (const std::exception& e) {
        LOG_ERROR("Error setting key: %s", e.what());
    }
}

std::string RedisUtil::getKey(const std::string& key) {
    try {
        auto val = this->redis->get(key);
        if (val) {
            LOG_INFO("Key (%s) has value (%s)", key.c_str(), val->c_str());
            return *val;
        }
        else {
            LOG_WARN("Key (%s) not found", key.c_str());
            return "";
        }
    }
    catch (const std::exception& e) {
        LOG_ERROR("Error getting key: %s", e.what());
        return "";
    }
}

void RedisUtil::deleteKey(const std::string& key) {
    try {
        long long deleted = this->redis->del(key);
        if (deleted > 0) {
            LOG_INFO("Key (%s) deleted", key.c_str());
        }
        else {
            LOG_WARN("Key (%s) not found", key.c_str());
        }
    }
    catch (const std::exception& e) {
        LOG_ERROR("Error deleting key: %s", e.what());
    }
}

void RedisUtil::disconnect() {
    if (this->redis) {
        delete this->redis;
        this->redis = nullptr;
        LOG_INFO("Disconnected from Redis");
    }
}

RedisUtil::~RedisUtil() {
    disconnect();
}
