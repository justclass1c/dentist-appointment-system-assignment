#include <string>
#include <regex>

using namespace std;

bool validateName(string input) {
    // wip
    return true;
}

bool validatePatientAge(int input) {
    try {
        if (input >= 1 && input <= 120) {
            return true;
        } else return false;

    } catch(...) {
        return false;
    }
}

bool validateGender(char input) {
    if (toupper(input) == 'M' || toupper(input) == 'F') return true;
    else return false;
}

template<typename T>
bool validateNRIC(string input, const vector<T>& users) {
    const regex nricFormat(R"(^\d{6}-\d{2}-\d{4}$)"); // check for xxxxxx-xx-xxxx
    if (regex_match(input, nricFormat)) {
        for (auto u : users) {
            if (input == u.user.nric) return false;
        }

        return true;
    }

    return false;
}

template<typename T>
bool validateEmail(const T& targetUser, const vector<T>& users) {
    const regex emailFormat(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    if (regex_match(targetUser.user.email, emailFormat)) {
        for (auto u : users) {
            if (u.user.email == targetUser.user.email) return false;
        }

        return true;
    }

    return false;
}

bool validatePassword(string input) {
    // implement confirm password
    return true;
}
bool validatePhoneNo(string input) {
    const regex phoneNoFormat(R"(^01\d-\d{3,4} \d{4}$)");

    return regex_match(input, phoneNoFormat);
}

