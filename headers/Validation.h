#ifndef VALIDATION_H
#define VALIDATION_H

#include <random>
#include <string>

using namespace std;

bool validateName(string input);
bool validatePatientAge(int input);
bool validateGender(char input);
bool validateNRIC(string input);
<<<<<<< HEAD
template<typename T>
bool validateEmail(const T& user);
=======
bool validateEmail(string input);
bool validatePassword(string input);
>>>>>>> a9154dd4f288ba911628396e9ad48080eba4c2be
bool validatePhoneNo(string input);

#endif
