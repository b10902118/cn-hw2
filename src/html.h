#pragma once

#include <string>
#include <vector>

namespace Html {

void init();

std::string tagToList(const std::string rhtml, const std::string tag,
                      const std::string baseUri, const std::string dirName);

std::string toTableHerf(const std::string uriBase,
                        const std::vector<std::string> files);

std::string replaceTag(const std::string &rhtml, const std::string &tagName,
                       const std::string &content);

extern std::string index;
extern std::string uploadf;
extern std::string uploadv;
extern std::string listf;
extern std::string listv;
extern std::string player;

}; // namespace Html
