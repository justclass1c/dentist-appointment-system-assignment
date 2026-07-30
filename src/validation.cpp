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

bool validateNRIC(string input) {
    const regex nricFormat(R"(^\d{6}-\d{2}-\d{4}$)"); // check for xxxxxx-xx-xxxx
    return regex_match(input, nricFormat);
}

template<typename T>
bool validateEmail(const T& user) {
    const regex emailFormat(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    return regex_match(user.email, emailFormat);
}

bool validatePhoneNo(string input) {
    const regex phoneNoFormat(R"(^01\d-\d{3,4} \d{4}$)");

    return regex_match(input, phoneNoFormat);
}

