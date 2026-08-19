#include <string>
#include <regex>

using namespace std;

bool validateName(string input) {
    for (unsigned char i : input) {
        if (isalpha(i)) {
            return true;
        }
    }
    return false;
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

bool validateEmail(string input) {
    const regex emailFormat(R"(^\w+@\w+.com)"); // upgrade to adapt to broader format
    return regex_match(input, emailFormat);
};

<<<<<<< Updated upstream
=======
        return true;
    } 
    return false;
}

bool validatePassword(string input) {
    // implement confirm password
    return true;
}
>>>>>>> Stashed changes
bool validatePhoneNo(string input) {
    const regex phoneNoFormat(R"(^01\d-\d{3,4} \d{4}$)");

    return regex_match(input, phoneNoFormat);
}

