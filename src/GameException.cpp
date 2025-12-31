#include "GameException.h"

GameException::GameException(std::string message)
    : m_message(std::move(message)) {}

const char* GameException::what() const noexcept {
    return m_message.c_str();
}

InvalidInputException::InvalidInputException(const std::string& message)
    : GameException("Invalid input: " + message) {}

InvalidStateException::InvalidStateException(const std::string& message)
    : GameException("Invalid state: " + message) {}

ResourceLoadException::ResourceLoadException(const std::string& message)
    : GameException("Resource load failed: " + message) {}
