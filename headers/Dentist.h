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

// ========================== Data Structures ==========================

string adminNameInput;
string adminPasswordInput;
const string adminName = "admin";
const string adminPassword = "pass123";

struct Dentist {
    User user;
    string id;          // unique ID (e.g., D001)
};

struct TimeSlot {
    string dentistId;
    string start;       // e.g., "10:00"
    string end;         // e.g., "12:00"
    bool available;     // true = free, false = locked
};

// ========================== Global Variables (extern) ==========================

extern vector<Dentist> dentists;
extern vector<TimeSlot> slots;

// ========================== File I/O Functions ==========================

void loadDentists();
void saveDentists();
void loadSlots();
void saveSlots();

// ========================== Find / Helper Functions ==========================

Dentist* findDentistById(const string& id);
Dentist* findDentistByName(const string& name);
vector<TimeSlot> getSlotsForDentist(const string& dentistId);
void addOrUpdateSlot(const TimeSlot& slot);
bool removeSlot(const string& dentistId, const string& start, const string& end);

void displayDentistInfo(const Dentist& d);
void displaySlots(const vector<TimeSlot>& slotList);

// ========================== Admin Panel ==========================

void adminRegisterDentist();
void adminModifyDentist();
void adminPanel();

// ========================== Reception Menu ==========================

void receptionViewAllSchedules();
void receptionMenu();

// ========================== Dentist Menu ==========================

void dentistViewSchedule(Dentist* d);
void dentistAddSlot(Dentist* d);
void dentistRemoveSlot(Dentist* d);
void dentistLockSlot(Dentist* d);
void dentistUnlockSlot(Dentist* d);
void dentistMenu(Dentist* d);

// ========================== Login Functions ==========================

void loginDentist();
void loginReception();
void registerPatientPlaceholder();   // placeholder

#endif
