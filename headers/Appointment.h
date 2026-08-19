#ifndef APPOINTMENT_H
#define APPOINTMENT_H

#include <string>
#include <vector>
#include "Session.h"

using namespace std;

enum Status { SCHEDULED, COMPLETED, CANCELLED };

struct Date {
    int day;
    int month;
    int year;
};

struct Appointment {
    string appointmentID;
    string patientID;
    string dentistID;
    Date   date;
    int    slotIndex;
    string reason;
    Status status;
};

void loadAppointments();
void saveAppointments();
void viewAppointments(const Session& current);

void appointmentMenu(const Session& current);

void scheduleAppointment(const Session& current);
void modifyAppointment(const Session& current);
void cancelAppointment(const Session& current);
void findNextAvailable();

bool hasActiveAppointment(string patientID);
int countAppointmentsForDentist(string dentistID);
bool getAppointmentInfo(string apptID, string& patientIDOut, string& dateOut);

#endif
