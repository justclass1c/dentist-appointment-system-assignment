#ifndef PAYMENT_H
#define PAYMENT_H

#include <string>
#include <vector>
#include "Patient.h" // needed to look up a patient's age/insurance for discounts

using namespace std;

// Named Constants
const double GST_RATE = 0.06;          // 6% government service tax
const double SENIOR_DISCOUNT = 0.10;   // 10% discount for senior citizens
const double INSURANCE_COVERAGE = 0.50; // Insurance covers 50% of total
const int SENIOR_AGE_THRESHOLD = 60;   // age at which senior discount applies

// Structures

// Represents a single billable service item (e.g. "Tooth Extraction", RM120)
struct ServiceItem {
    string serviceName;
    double price;
};

// Represents one finalized payment transaction/record
struct Payment {
    string paymentID;
    string patientID;      // Reference only - no duplication of patient data
    string appointmentID;  // Reference only - no duplication of appointment data
    double totalAmount;
    string method;         // "Cash", "Card", or "Insurance"
    string date;
};

// File Processing
void loadPaymentRecords();

// Lookup - lets the Appointment module check before offering to bill twice
bool hasPaymentForAppointment(const string& appointmentID);

// Single entry point used by both flows:
//   Patient   -> View Appointments -> select completed appointment -> Payment
//   Reception -> View Appointments -> select completed appointment -> Assign Payment
// The appointment-selection UI lives in appointment.cpp; this function does
// the actual billing (services, discount, method, receipt, save).
void processPaymentTransaction(const string& appointmentID, const string& patientID);

// Output
void displayAllPayments();
void generateSummaryReport();

#endif
