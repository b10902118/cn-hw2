#include "auth.h"
#include "fs.h"

namespace Auth {

const vector<string> secrets = Fs::readLines("./secret");
bool authorized(const string credential) {
    for (auto secret : secrets) {
        if (credential == secret) return true;
    }
    return false;
}

} // namespace Auth
