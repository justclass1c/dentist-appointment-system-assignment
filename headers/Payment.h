#ifndef PAYMENT_H
#define PAYMENT_H

#include <string>
#include <vector>
#include "Patient.h" // needed to look up a patient's age/insurance for discounts
#include "Session.h"

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

// Exposed so other domains (e.g. Loyalty) can read real catalog names/prices
// instead of duplicating them - same pattern as `extern vector<Patient> patients;`
extern vector<ServiceItem> availableServices;

// Represents one billing record, from invoice through to payment.
// Reception creates it as PENDING (picks the treatment, sets the amount);
// only the patient can change it to PAID (picks how they're paying).
struct Payment {
    string paymentID;
    string patientID;      // Reference only - no duplication of patient data
    string appointmentID;  // Reference only - no duplication of appointment data
    string itemsSummary;   // treatment rendered, as fixed by reception at invoice time
    double totalAmount;    // fixed by reception at invoice time - the patient cannot change this
    string method;         // "" until paid, then "Cash" / "Card" / "Insurance"
    string invoiceDate;    // date reception issued the invoice
    string paymentDate;    // date the patient paid; "" until paid
    string status;         // "PENDING" or "PAID"
};

// File Processing
void loadPaymentRecords();

// Reception only: creates the invoice for a completed appointment.
// Reception selects the treatment actually rendered and the total is
// computed and fixed here (with discount auto-applied) - the patient never
// gets to choose or change what they're billed for.
void issueInvoice(const string& appointmentID, const string& patientID);

// Blocks reception from invoicing the same appointment twice.
bool hasInvoiceForAppointment(const string& appointmentID);

// Patient only: pays off one of their own PENDING invoices. The patient
// only ever picks a payment method - the amount and treatment were already
// fixed by reception and cannot be changed here.
void payForAppointment(const Session& current);

// Output
void displayAllPayments();
void generateSummaryReport();

// Reception/Admin menu: view all payments / revenue summary
void paymentReportsMenu();

#endif
