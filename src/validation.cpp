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

<<<<<<< HEAD
template<typename T>
bool validateEmail(const T& user) {
    const regex emailFormat(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    return regex_match(user.email, emailFormat);
=======
bool validateEmail(string input) {
    const regex emailFormat(R"(^\w+@\w+.com)"); // upgrade to adapt to broader format
    return regex_match(input, emailFormat);
>>>>>>> a9154dd4f288ba911628396e9ad48080eba4c2be
}

bool validatePassword(string input) {
    // implement confirm password
    return true;
}
bool validatePhoneNo(string input) {
    const regex phoneNoFormat(R"(^01\d-\d{3,4} \d{4}$)");

    return regex_match(input, phoneNoFormat);
}

