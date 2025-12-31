#pragma once
#include <vector>
#include <string>
#include <iostream>

class CommandSequence {
public:
    CommandSequence();
    explicit CommandSequence(const std::vector<std::string>& initialCommands);

    void push(const std::string &cmd);
    [[nodiscard]] bool matchesMove() const;
    [[nodiscard]] bool matchesAttack() const;
    void clear();
    [[nodiscard]] const std::vector<std::string>& getCommands() const;

    friend std::ostream& operator<<(std::ostream &os, const CommandSequence &cs);

private:
    std::vector<std::string> m_seq;
    static constexpr std::size_t m_maxHistory = 4;
    [[nodiscard]] bool endsWithPattern(const std::vector<std::string> &pattern) const;
};
