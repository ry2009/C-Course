#pragma once

#include <string>
#include <functional>
#include <map>
#include <vector>
#include <mutex>
#include <memory>

class Message {
public:
    Message(const std::string& topic, const void* data);
    ~Message();
    
    template<typename T>
    T getData() const {
        return *static_cast<const T*>(data_);
    }
    
    std::string getTopic() const { return topic_; }
    
private:
    std::string topic_;
    const void* data_;
};

class MessageBus {
public:
    using MessageHandler = std::function<void(const Message&)>;
    
    MessageBus();
    ~MessageBus();
    
    // Message handling
    void publish(const Message& msg);
    void subscribe(const std::string& topic, MessageHandler handler);
    void unsubscribe(const std::string& topic, MessageHandler handler);
    
    // System management
    void start();
    void stop();
    bool isRunning() const;
    
private:
    std::map<std::string, std::vector<MessageHandler>> subscribers_;
    std::mutex mutex_;
    bool running_;
}; 