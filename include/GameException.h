#pragma once
#include <exception>
#include <string>

class GameException : public std::exception {
public:
    explicit GameException(std::string message);
    [[nodiscard]] const char* what() const noexcept override;
protected:
    std::string m_message;
};

class InvalidInputException : public GameException {
public:
    explicit InvalidInputException(const std::string& message);
};

class InvalidStateException : public GameException {
public:
    explicit InvalidStateException(const std::string& message);
};

class ResourceLoadException : public GameException {
public:
    explicit ResourceLoadException(const std::string& message);
};
