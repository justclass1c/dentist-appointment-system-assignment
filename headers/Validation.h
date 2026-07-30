#ifndef VALIDATION_H
#define VALIDATION_H

#include <random>
#include <string>

using namespace std;

bool validateName(string input);
bool validatePatientAge(int input);
bool validateGender(char input);
bool validateNRIC(string input);
template<typename T>
bool validateEmail(const T& user);
bool validatePhoneNo(string input);

#endif
