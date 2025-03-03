#ifndef REDIS_H
#define REDIS_H
#pragma once

#include <sw/redis++/redis++.h>
#include <string>

class RedisUtil {
private:
    sw::redis::Redis* redis;
    std::string hostname;
    int port;
    int timeout;

public:
    RedisUtil(std::string hostname = "localhost", int port = 6379, int timeout = -1);
    ~RedisUtil();

    void connect();
    void setKey(const std::string& key, const std::string& value);
    std::string getKey(const std::string& key);
    void deleteKey(const std::string& key);
    void disconnect();
};

#endif // REDIS_H
