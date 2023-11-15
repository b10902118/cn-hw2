/*
static bool isConnectionClose(const char *header) {
std::regex pattern("Connection:\\s*close\\s*",
                   std::regex_constants::icase);
return std::regex_search(header, pattern);
}

static bool isContentType(const char *header) {
std::regex pattern("Content-Type:\\s*([^\\s]+)\\s*",
                   std::regex_constants::icase);
return std::regex_search(header, pattern);
}

static bool isAuthorizationBasic(const char *header) {
std::regex pattern("Authorization:\\s*Basic\\s.*",
                   std::regex_constants::icase);
return std::regex_search(header, pattern);
}
*/

/*
static std::string extractBasicCredentials(char *header) {
// Define a regex pattern to match Authorization: Basic followed by
// Base64 string
// std::regex
// pattern("Authorization:\\s*Basic\\s+([A-Za-z0-9+/]+={0,2})");
std::regex pattern("Authorization:\\s*Basic\\s+([^\\s]+)\\s*");

// Use std::smatch to capture the matched groups
std::smatch matches;

std::string input(header);
// Check if the pattern is found in the input
if (std::regex_search(input, matches, pattern)) {
    // Check if there's a captured group (Base64 string)
    if (matches.size() > 1) {
        // Return the Base64 string as a C++ string
        return matches[1].str();
    }
    else {
        return ""; // No Base64 credentials found
    }
}
else {
    return ""; // Authorization header not found or does not contain
               // Basic credentials
}
}
*/

/*
if (isConnectionClose(s)) connected = false;
else if (isAuthorizationBasic(s))
credential = extractBasicCredentials(s);
        */
// string header = string(s);
/*
if (caseInsensitiveIdentical(header, "Connection:")) {
// to lower case
transform(header.begin(), header.end(), header.begin(),
      [](unsigned char c) { return std::tolower(c); });
if (header.find("close") != string::npos) }
        */
