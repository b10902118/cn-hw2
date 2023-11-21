#include "html.h"
#include "fs.h"

namespace Html {
void init() {
    // Define the directory path
    std::string webDirectory = "./web/";

    // Read index.html
    index = Fs::readFile(webDirectory + "index.html");

    // Read uploadf.html
    uploadf = Fs::readFile(webDirectory + "uploadf.html");

    // Read uploadv.html
    uploadv = Fs::readFile(webDirectory + "uploadv.html");

    // Read listf.rhtml
    listf = Fs::readFile(webDirectory + "listf.rhtml");

    // Read listv.rhtml
    listv = Fs::readFile(webDirectory + "listv.rhtml");

    // Read player.rhtml
    player = Fs::readFile(webDirectory + "player.rhtml");
}

std::string toTableHerf(const std::string uriBase,
                        const std::vector<std::string> files) {
    // Create an empty string to store the result
    std::string result;

    // Iterate over the files and construct the HTML table rows
    for (const auto &file : files) {
        result += "<tr><td><a href=\"" + uriBase + file + "\">" + file +
                  "</a></td></tr>\n";
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
    else cerr << "replaceTag: no tag to replace";

    return ret;
}
} // namespace Html
