#ifndef VALIDATION_H
#define VALIDATION_H

#include <random>
#include <string>

using namespace std;

bool validateName(string input);
bool validatePatientAge(int input);
bool validateGender(char input);
bool validateNRIC(string input);
bool validateEmail(string input);
bool validatePassword(string input);
bool validatePhoneNo(string input);

#endif
