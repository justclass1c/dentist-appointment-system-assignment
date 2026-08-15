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
#include "User.h"

using namespace std;

const string adminName = "admin";
const string adminPassword = "pass123";

struct Dentist {
    User user;
    string id;
};

struct TimeSlot {
    string dentistId;
    string start;
    string end;
    bool available;
};

extern vector<Dentist> dentists;
extern vector<TimeSlot> slots;

void loadDentists();
void saveDentists();
void loadSlots();
void saveSlots();

Dentist* findDentistById(const string& id);
Dentist* findDentistByName(const string& name);
vector<TimeSlot> getSlotsForDentist(const string& dentistId);
void addOrUpdateSlot(const TimeSlot& slot);
bool removeSlot(const string& dentistId, const string& start, const string& end);

void displayDentistInfo(const Dentist& d);
void displaySlots(const vector<TimeSlot>& slotList);

void adminRegisterDentist();
void adminModifyDentist();
void adminPanel();

void receptionViewAllSchedules();
void receptionMenu();

void dentistViewSchedule(Dentist* d);
void dentistAddSlot(Dentist* d);
void dentistRemoveSlot(Dentist* d);
void dentistLockSlot(Dentist* d);
void dentistUnlockSlot(Dentist* d);
void dentistMenu(Dentist* d);

void loginDentist();
void loginReception();
void loginAdmin();

#endif
