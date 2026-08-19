#include <string>
#include <vector>

using namespace std;

template <typename U>
bool verifyEmail(const vector<U>& users, string input) {
    for (auto user: users) {
        if (input == user.user.email) return true;
    }
    cout << "Email format invalid, please try again.\n";
    return false;
}

template <typename U>
bool verifyPassword(const vector<U>& users, string input) {
    for (auto user: users) {
        if (input == user.user.password) return true;
    }

    return false;
}