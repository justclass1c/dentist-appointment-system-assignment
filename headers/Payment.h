#ifndef PAYMENT_H
#define PAYMENT_H

#include <string>
#include <vector>
#include "Patient.h" // added: needed to look up a patient's age/insurance for discounts

using namespace std;

// Named Constants
const double GST_RATE = 0.06;          // 6% government service tax
const double SENIOR_DISCOUNT = 0.10;   // 10% discount for senior citizens
const double INSURANCE_COVERAGE = 0.50; // Insurance covers 50% of total
const int SENIOR_AGE_THRESHOLD = 60;   // added: age at which senior discount applies

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

// Function Declarations

// Program Design / Menu
void runPaymentModule(vector<Payment>& paymentHistory, vector<Patient>& patients);
void displayPaymentMenu();

// Service selection & calculation
vector<ServiceItem> selectServices();
double calculateTotal(vector<ServiceItem> services);          // pass-by-value
void applyDiscount(Payment& p, bool isSenior, bool hasInsurance); // pass-by-reference

// Payment processing
Payment processPayment(vector<Payment>& paymentHistory, vector<Patient>& patients,
                        const string& patientID, const string& appointmentID);
string generatePaymentID(const vector<Payment>& paymentHistory);  // returned value
bool isDuplicatePaymentID(const vector<Payment>& paymentHistory, const string& id);

// Output
void generateReceipt(const Payment& p, const vector<ServiceItem>& services);
void displayAllPayments(const vector<Payment>& paymentHistory);
void generateSummaryReport(const vector<Payment>& paymentHistory);

// File Processing
void savePaymentRecord(const Payment& p);
void loadPaymentRecords(vector<Payment>& paymentHistory);

#endif
