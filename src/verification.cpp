#include <string>
#include <vector>

using namespace std;

template <typename U>
bool verifyEmail(const vector<U>& users, string input) {
    for (auto user: users) {
        if (input == user.user.email) return true;
    }

    return false;
}

template <typename U>
bool verifyPassword(const vector<U>& users, string email, string input) {
    for (auto user: users) {
        if (email == user.user.email) return input == user.user.password;
    }

    return false;
}
