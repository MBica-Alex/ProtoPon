#include "CommandSequence.h"

CommandSequence::CommandSequence() {}

CommandSequence::CommandSequence(const std::vector<std::string>& initialCommands)
    : m_seq(initialCommands) {}

void CommandSequence::push(const std::string &cmd) {
    m_seq.push_back(cmd);
    if (m_seq.size() > m_maxHistory) m_seq.erase(m_seq.begin());
}

bool CommandSequence::matchesMove() const { 
    return endsWithPattern({ "pa", "pa", "pa", "po" }); 
}

bool CommandSequence::matchesAttack() const { 
    return endsWithPattern({ "po", "po", "pa", "po" }); 
}

bool CommandSequence::matchesRetreat() const {
    return endsWithPattern({ "po", "pa", "po", "pa" });
}

void CommandSequence::clear() { m_seq.clear(); }

const std::vector<std::string>& CommandSequence::getCommands() const { return m_seq; }

bool CommandSequence::endsWithPattern(const std::vector<std::string> &pattern) const {
    if (m_seq.size() < pattern.size()) return false;
    size_t n = m_seq.size(), m = pattern.size();
    for (size_t i = 0; i < m; ++i)
        if (m_seq[n - m + i] != pattern[i]) return false;
    return true;
}


