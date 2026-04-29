#pragma once
#include <string>

enum class CommandType { SET, GET, DEL, EXIT, UNKNOWN };

struct Command {
    CommandType type = CommandType::UNKNOWN;
    std::string key;
    std::string value;
    // Non-empty only when type == UNKNOWN; contains a human-readable reason.
    std::string error;
};

// Parses one text line into a Command. Never throws.
// On any parse failure, returns a Command with type UNKNOWN and error set.
Command parse(const std::string& line);
