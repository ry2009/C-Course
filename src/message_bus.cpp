#include "message_bus.h"
#include <iostream>

Message::Message(const std::string& topic, const void* data)
    : topic_(topic), data_(data) {}

Message::~Message() {
    // Note: We don't delete data_ here as it's managed by the publisher
}

MessageBus::MessageBus() : running_(false) {}

MessageBus::~MessageBus() {
    stop();
}

void MessageBus::publish(const Message& msg) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = subscribers_.find(msg.getTopic());
    if (it != subscribers_.end()) {
        for (const auto& handler : it->second) {
            handler(msg);
        }
    }
}

void MessageBus::subscribe(const std::string& topic, MessageHandler handler) {
    std::lock_guard<std::mutex> lock(mutex_);
    subscribers_[topic].push_back(handler);
}

void MessageBus::unsubscribe(const std::string& topic, MessageHandler handler) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = subscribers_.find(topic);
    if (it != subscribers_.end()) {
        auto& handlers = it->second;
        handlers.erase(
            std::remove(handlers.begin(), handlers.end(), handler),
            handlers.end()
        );
    }
}

void MessageBus::start() {
    std::lock_guard<std::mutex> lock(mutex_);
    running_ = true;
    std::cout << "Message bus started" << std::endl;
}

void MessageBus::stop() {
    std::lock_guard<std::mutex> lock(mutex_);
    running_ = false;
    std::cout << "Message bus stopped" << std::endl;
}

bool MessageBus::isRunning() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return running_;
} 