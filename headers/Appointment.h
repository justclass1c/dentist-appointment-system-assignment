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

// added: Reception's invoicing entry point, reusing the existing appointment
// table/selection UI so it doesn't need its own copy of it. Patient payment
// (payForAppointment) now lives in Payment.h/payment.cpp instead, since the
// patient picks from their own pending invoices, not from appointments.
void assignPaymentForAppointment(const Session& current); // Reception/Admin -> View Appointments -> select completed -> Issue Invoice

#endif
