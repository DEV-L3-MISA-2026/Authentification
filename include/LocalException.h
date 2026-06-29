#include <iostream>
#include <exception>
#include <string>

class LocalException : public std::exception {
private:
    std::string message;
public:
    LocalException(const std::string& msg) : message(msg) {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};