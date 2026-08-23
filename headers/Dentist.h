#ifndef DENTIST_H
#define DENTIST_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <cstdlib>
#include <filesystem>
#include "User.h"

using namespace std;

const string adminName = "admin";
const string adminPassword = "pass123";

struct Dentist {
    User user;
    string id;
};

extern vector<Dentist> dentists;

void loadDentists();
void saveDentists();

Dentist* findDentistById(const string& id);
Dentist* findDentistByEmail(const string email);
Dentist* findDentistByName(const string& name);

void displayDentistInfo(const Dentist& d);

void adminRegisterDentist();
void adminModifyDentist();
void adminPanel();

void receptionViewAllDentists();
void receptionMenu();

void dentistMenu(Dentist* d);

void loginDentist();
void loginReception();
void loginAdmin();

#endif
