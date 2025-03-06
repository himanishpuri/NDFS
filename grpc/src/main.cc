#include <iostream>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <chrono>
#include <grpcpp/grpcpp.h>
#include <sw/redis++/redis++.h>
#include "proto/storage.grpc.pb.h"
#include "proto/tracking.grpc.pb.h"
#include "proto/client.grpc.pb.h"

sw::redis::Redis redis("tcp://127.0.0.1:6379");

// ======================== STORAGE SERVICE ========================
class StorageServiceImpl final : public StorageService::Service {
public:
    ::grpc::Status StoreChunk(::grpc::ServerContext* context, const ::StoreChunkRequest* request, ::StoreChunkResponse* response) override {
        std::string file_key = "file:" + request->chunk_hash();
        redis.hset(file_key, request->chunk_hash(), request->data());

        response->set_success(true);
        std::cout << "[Storage] Stored Chunk: " << request->chunk_hash() << std::endl;
        return grpc::Status::OK;
    }

    ::grpc::Status RetrieveChunk(::grpc::ServerContext* context, const ::RetrieveChunkRequest* request, ::RetrieveChunkResponse* response) override {
        std::string file_key = "file:" + request->chunk_hash();
        auto data = redis.hget(file_key, request->chunk_hash());

        if (!data) {
            return grpc::Status(::grpc::StatusCode::NOT_FOUND, "Chunk not found");
        }

        response->set_data(*data);
        std::cout << "[Storage] Retrieved Chunk: " << request->chunk_hash() << std::endl;
        return grpc::Status::OK;
    }
};

// ======================== TRACKING SERVICE ========================
std::unordered_map<std::string, std::string> chunk_locations;
std::unordered_map<std::string, std::chrono::steady_clock::time_point> node_heartbeats;
std::mutex tracking_mutex;

class TrackingServiceImpl final : public TrackingService::Service {
public:
    ::grpc::Status RegisterNode(::grpc::ServerContext* context, const ::NodeInfo* request, ::RegisterResponse* response) override {
        std::lock_guard<std::mutex> lock(tracking_mutex);
        node_heartbeats[request->node_id()] = std::chrono::steady_clock::now();
        response->set_success(true);
        std::cout << "[Tracking] Node Registered: " << request->node_id() << std::endl;
        return grpc::Status::OK;
    }

    ::grpc::Status Heartbeat(::grpc::ServerContext* context, const ::NodeInfo* request, ::HeartbeatResponse* response) override {
        std::lock_guard<std::mutex> lock(tracking_mutex);
        node_heartbeats[request->node_id()] = std::chrono::steady_clock::now();
        response->set_success(true);
        std::cout << "[Tracking] Heartbeat received from Node: " << request->node_id() << std::endl;
        return grpc::Status::OK;
    }

    ::grpc::Status StoreChunk(::grpc::ServerContext* context, const ::ChunkInfo* request, ::StoreResponse* response) override {
        std::lock_guard<std::mutex> lock(tracking_mutex);
        chunk_locations[request->chunk_hash()] = request->node_ids(0);
        response->set_success(true);
        std::cout << "[Tracking] Stored Chunk Location: " << request->chunk_hash() << " -> " << request->node_ids(0) << std::endl;
        return grpc::Status::OK;
    }

    ::grpc::Status GetChunkLocation(::grpc::ServerContext* context, const ::ChunkRequest* request, ::ChunkResponse* response) override {
        std::lock_guard<std::mutex> lock(tracking_mutex);
        if (chunk_locations.find(request->chunk_hash()) == chunk_locations.end()) {
            return grpc::Status(::grpc::StatusCode::NOT_FOUND, "Chunk location not found");
        }

        response->add_node_ids(chunk_locations[request->chunk_hash()]);
        std::cout << "[Tracking] Retrieved Chunk Location: " << request->chunk_hash() << " -> " << chunk_locations[request->chunk_hash()] << std::endl;
        return grpc::Status::OK;
    }
};

// Background thread to monitor node heartbeats
void MonitorNodes() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        auto now = std::chrono::steady_clock::now();
        std::lock_guard<std::mutex> lock(tracking_mutex);

        std::vector<std::string> nodes_to_remove;
        for (const auto& [node_id, last_heartbeat] : node_heartbeats) {
            if (std::chrono::duration_cast<std::chrono::seconds>(now - last_heartbeat).count() > 15) {
                std::cout << "[Tracking] Node " << node_id << " marked offline!" << std::endl;
                nodes_to_remove.push_back(node_id);
            }
        }

        for (const auto& node_id : nodes_to_remove) {
            node_heartbeats.erase(node_id);
        }
    }
}

// ======================== CLIENT SERVICE ========================
class ClientServiceImpl final : public ClientService::Service {
public:
    ::grpc::Status UploadFile(::grpc::ServerContext* context, const ::UploadRequest* request, ::UploadResponse* response) override {
        for (const auto& chunk : request->chunks()) {
            std::string chunk_hash = std::to_string(std::hash<std::string>{}(chunk));
            redis.hset("file:" + request->filename(), chunk_hash, chunk);
            std::cout << "[Client] Stored chunk: " << chunk_hash << std::endl;
        }

        response->set_success(true);
        std::cout << "[Client] File uploaded: " << request->filename() << std::endl;
        return grpc::Status::OK;
    }

    ::grpc::Status DownloadFile(::grpc::ServerContext* context, const ::DownloadRequest* request, ::DownloadResponse* response) override {
        std::unordered_map<std::string, std::string> chunks;
        redis.hgetall("file:" + request->filename(), std::inserter(chunks, chunks.end()));
        if (chunks.empty()) {
            return grpc::Status(::grpc::StatusCode::NOT_FOUND, "File not found");
        }

        for (const auto& chunk : chunks) {
            response->add_chunks(chunk.second);
        }

        std::cout << "[Client] File downloaded: " << request->filename() << std::endl;
        return grpc::Status::OK;
    }
};

// ======================== RUN SERVERS ========================
void RunStorageServer() {
    std::string address("0.0.0.0:50052");
    StorageServiceImpl service;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Storage Server running on " << address << std::endl;
    server->Wait();
}

void RunTrackingServer() {
    std::string address("0.0.0.0:50053");
    TrackingServiceImpl service;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Tracking Server running on " << address << std::endl;
    std::thread monitor_thread(MonitorNodes);
    server->Wait();
}

void RunClientServer() {
    std::string address("0.0.0.0:50054");
    ClientServiceImpl service;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Client Server running on " << address << std::endl;
    server->Wait();
}

// ======================== MAIN FUNCTION ========================
int main() {
    std::thread storage_thread(RunStorageServer);
    std::thread tracking_thread(RunTrackingServer);
    std::thread client_thread(RunClientServer);

    storage_thread.join();
    tracking_thread.join();
    client_thread.join();

    return 0;
}
