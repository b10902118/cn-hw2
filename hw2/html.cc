#include "html.h"
#include "fs.h"
#include <iostream>

using namespace std;
namespace Html {

std::string index;
std::string uploadf;
std::string uploadv;
std::string listf;
std::string listv;
std::string player;

void init() {
    // Define the directory path
    std::string webDirectory = "./web/";

    // Read index.html
    index = Fs::readText(webDirectory + "index.html");

    // Read uploadf.html
    uploadf = Fs::readText(webDirectory + "uploadf.html");

    // Read uploadv.html
    uploadv = Fs::readText(webDirectory + "uploadv.html");

    // Read listf.rhtml
    listf = Fs::readText(webDirectory + "listf.rhtml");

    // Read listv.rhtml
    listv = Fs::readText(webDirectory + "listv.rhtml");

    // Read player.rhtml
    player = Fs::readText(webDirectory + "player.rhtml");
}

std::string tagToList(const std::string rhtml, const std::string tag, const std::string baseUri,
                      const std::string dirName) {
    vector<string> fileNames = Fs::listDir(dirName);
    string fileTags = toTableHerf(baseUri, fileNames);
    string replaced = replaceTag(rhtml, tag, fileTags);
    std::cout << replaced << endl;
    return replaced;
}

std::string toTableHerf(const std::string uriBase, const std::vector<std::string> files) {
    // Create an empty string to store the result
    std::string result;

    // Iterate over the files and construct the HTML table rows
    for (const auto &file : files) {
        result += "<tr><td><a href=\"" + uriBase + file + "\">" + file + "</a></td></tr>\n";
    }

    return result;
}

std::string replaceTag(const std::string &rhtml, const std::string &tagName,
                       const std::string &content) {
    std::string ret(rhtml), tag = "<?" + tagName + "?>";
    size_t pos = ret.find(tag);
    if (pos != std::string::npos) {
        ret.replace(pos, tag.length(), content);
    }
    else std::cerr << "replaceTag: no tag to replace";
    return ret;
}
} // namespace Html
