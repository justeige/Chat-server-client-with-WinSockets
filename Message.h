#pragma once

#include <string>
#include <vector>

// text message:
// [length]        = 32 bit * 1
// [message-text]  = 32 bit * length

class Message {
public:
    Message(std::string const& msg)
    {
        m_size = msg.size();
        m_buffer = std::vector<char>(std::begin(msg), std::end(msg));
    }

    const char* head() const
    {
        return (char*)&m_size;
    }

    int32_t length() const
    {
        return m_size;
    }

    const char* body() const
    {
        return &m_buffer[0];
    }


private:
    int32_t m_size;
    std::vector<char> m_buffer;
};