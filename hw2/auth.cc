#include "auth.h"
#include "fs.h"

namespace Auth {

// void init() { // secrets = Fs::readLines("./secret");}

const vector<string> secrets = Fs::readLines("./secret");
bool authorized(const string credential) {
    for (auto secret : secrets) {
        if (credential == secret) return true;
    }
    return false;
}

} // namespace Auth
