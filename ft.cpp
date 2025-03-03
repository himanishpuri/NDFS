#include <bits/stdc++.h>
#include <sw/redis++/redis++.h>
using namespace std;
using namespace sw::redis;

void load_env_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        setenv(key.c_str(), value.c_str(), 1);  // Set environment variable
    }
    file.close();
}

int main() {

    load_env_file(".env");

    const char* redis_url = getenv("REDIS_URL");
    if (redis_url == nullptr) {
        cout << "REDIS_URL not found" << endl;
        return 1;
    }

    Redis redis(redis_url);
    redis.set("key", "kanishka");
    OptionalString val = redis.get("key");
    if (val) {
        cout << *val << endl;
    }
    else {
        cout << "key not found" << endl;
    }
    return 0;
}