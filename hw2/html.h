#pragma once

#include <string>

namespace Html {

void init();

std::string toTableHerf(const std::string uriBase,
                        const std::vector<std::string> files);

std::string replaceTag(const std::string &rhtml, const std::string &tagName,
                       const std::string &content);

std::string index;
std::string uploadf;
std::string uploadv;
std::string listf;
std::string listv;
std::string player;

}; // namespace Html
