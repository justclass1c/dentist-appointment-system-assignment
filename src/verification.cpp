<<<<<<< HEAD
#include <regex>
#include <string>
#include "../headers/Patient.h"

using namespace std;

template<typename T>;

bool validateEmail(vector<T> users, string input) {
    for (const auto& user : users) {

    }
=======
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
bool verifyPassword(const vector<U>& users, string input) {
    for (auto user: users) {
        if (input == user.user.password) return true;
    }

>>>>>>> a9154dd4f288ba911628396e9ad48080eba4c2be
    return false;
}
