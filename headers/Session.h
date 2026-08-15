#ifndef SESSION_H
#define SESSION_H

#include <string>
#include "User.h"

using namespace std;

struct Session {
    string userId;
    string name;
    string password;
    Roles  role;
};

#endif
