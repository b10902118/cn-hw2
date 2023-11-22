#include <vector>
#include <string>

using namespace std;
namespace Auth {
extern const vector<string> secrets;
// void init();
bool authorized(const string credential);
} // namespace Auth
