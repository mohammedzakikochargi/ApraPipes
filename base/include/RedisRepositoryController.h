#include <sw/redis++/redis++.h>
// #include <thread>
// #include <vector>
// #include <string_view>
// #include <string>
// #include <memory>
// #include <unordered_map>
// #include <iostream>

class RedisRepositoryController
{
public:
    static sw::redis::Redis& getInstance()
    {   sw::redis::ConnectionOptions connection_options;
        connection_options.type = sw::redis::ConnectionType::UNIX;
        connection_options.path = "/run/redis/redis.sock";
        connection_options.db = 1;
        connection_options.socket_timeout = std::chrono::milliseconds(1000);
        static sw::redis::Redis redis = sw::redis::Redis(connection_options);
        return redis;
    }
    RedisRepositoryController()
    {}
};