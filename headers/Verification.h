#ifndef VERIFICATION_H
#define VERIFICATION_H

#include <string>
#include <vector>

using namespace std;

template <typename U>
bool verifyEmail(const vector<U>& users, string input);

template <typename U>
bool verifyPassword(const vector<U>& users, string input);

#endif
