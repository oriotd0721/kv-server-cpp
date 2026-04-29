#include "parser.hpp"
#include <algorithm>
#include <sstream>

static std::string toUpper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::toupper);
    return s;
}

Command parse(const std::string& line) {
    Command cmd;
    std::istringstream iss(line);
    std::string token;

    if (!(iss >> token)) {
        // iss >> fails on an all-whitespace or empty line.
        cmd.error = "empty command";
        return cmd;
    }

    std::string verb = toUpper(token);

    if (verb == "SET") {
        std::string key, value;
        if (!(iss >> key)) {
            cmd.error = "SET requires: SET <key> <value>";
            return cmd;
        }
        if (!(iss >> value)) {
            cmd.error = "SET requires: SET <key> <value>";
            return cmd;
        }
        cmd.type = CommandType::SET;
        cmd.key = key;
        cmd.value = value;
    } else if (verb == "GET") {
        std::string key;
        if (!(iss >> key)) {
            cmd.error = "GET requires: GET <key>";
            return cmd;
        }
        cmd.type = CommandType::GET;
        cmd.key = key;
    } else if (verb == "DEL") {
        std::string key;
        if (!(iss >> key)) {
            cmd.error = "DEL requires: DEL <key>";
            return cmd;
        }
        cmd.type = CommandType::DEL;
        cmd.key = key;
    } else if (verb == "EXIT") {
        cmd.type = CommandType::EXIT;
    } else {
        cmd.error = "unknown command '" + verb + "'. Valid: SET GET DEL EXIT";
    }

    return cmd;
}
