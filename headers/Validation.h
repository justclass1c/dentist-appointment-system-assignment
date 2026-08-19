#ifndef VALIDATION_H
#define VALIDATION_H

#include <random>
#include <string>

using namespace std;

bool validateName(string input);
bool validatePatientAge(int input);
bool validateGender(char input);

template<typename T>
bool validateNRIC(string input, const vector<T>& users);

template<typename T>
bool validateEmail(const T& user, const vector<T>& users);
bool validatePassword(string input);
bool validatePhoneNo(string input);

#endif
