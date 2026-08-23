#include "../headers/Payment.h"
#include "../headers/Console.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <ctime>
#include <cstdlib>

using namespace std;

static vector<Payment> paymentHistory;

// A small fixed service list to bill against (could later be loaded from file)
static vector<ServiceItem> availableServices = {
    {"Dental Check-up", 50.00},
    {"Tooth Extraction", 120.00},
    {"Scaling & Polishing", 90.00},
    {"Root Canal Treatment", 450.00},
    {"Dental Filling", 80.00},
    {"Teeth Whitening", 300.00}
};

static string todayString() {
    time_t rawTime = time(0);
    tm* localTime = localtime(&rawTime);
    stringstream ss;
    ss << setw(2) << setfill('0') << localTime->tm_mday << "/"
       << setw(2) << setfill('0') << (localTime->tm_mon + 1) << "/"
       << (localTime->tm_year + 1900);
    return ss.str();
}

// ---------------------------------------------------------
// File Processing
// ---------------------------------------------------------
void loadPaymentRecords() {
    paymentHistory.clear();

    ifstream inFile("data/payments.txt");
    if (!inFile.is_open()) return; // no file yet - not an error on first run

    string line;
    while (getline(inFile, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        string field;
        Payment p;

        getline(ss, p.paymentID, ',');
        getline(ss, p.patientID, ',');
        getline(ss, p.appointmentID, ',');
        getline(ss, field, ',');
        p.totalAmount = field.empty() ? 0.0 : stod(field);
        getline(ss, p.method, ',');
        getline(ss, p.date, ',');

        if (p.paymentID.empty()) continue;
        paymentHistory.push_back(p);
    }
    inFile.close();
}

static void savePaymentRecord(const Payment& p) {
    filesystem::create_directories("data");
    ofstream outFile("data/payments.txt", ios::app);
    if (!outFile) {
        cout << "  [!] Could not write to data/payments.txt\n";
        return;
    }
    outFile << p.paymentID << "," << p.patientID << "," << p.appointmentID << ","
            << fixed << setprecision(2) << p.totalAmount << "," << p.method << ","
            << p.date << "\n";
    outFile.close();
}

// ---------------------------------------------------------
// Lookup
// ---------------------------------------------------------
bool hasPaymentForAppointment(const string& appointmentID) {
    for (size_t i = 0; i < paymentHistory.size(); i++) {
        if (paymentHistory[i].appointmentID == appointmentID) return true;
    }
    return false;
}

static bool isDuplicatePaymentID(const string& id) {
    for (size_t i = 0; i < paymentHistory.size(); i++) {
        if (paymentHistory[i].paymentID == id) return true;
    }
    return false;
}

// Returned value: generates a new unique ID based on the highest existing number
static string generatePaymentID() {
    int highest = 0;
    for (size_t i = 0; i < paymentHistory.size(); i++) {
        string suffix = paymentHistory[i].paymentID.substr(3); // strip "PAY"
        int number = atoi(suffix.c_str());
        if (number > highest) highest = number;
    }

    string id;
    do {
        highest++;
        stringstream ss;
        ss << "PAY" << setw(4) << setfill('0') << highest;
        id = ss.str();
    } while (isDuplicatePaymentID(id)); // safety net

    return id;
}

// ---------------------------------------------------------
// Service Selection & Calculation
// ---------------------------------------------------------
static vector<ServiceItem> selectServices() {
    vector<ServiceItem> selected;

    cout << "\n  Services rendered\n";
    cout << "  " << string(40, '-') << "\n";
    for (int i = 0; i < (int)availableServices.size(); i++) {
        cout << "  " << (i + 1) << ". " << left << setw(22) << availableServices[i].serviceName
             << "RM " << fixed << setprecision(2) << availableServices[i].price << "\n";
    }
    cout << "  " << string(40, '-') << "\n";

    while (true) {
        int choice = readMenuChoice("  Add a service (0 to finish): ", 0, (int)availableServices.size());
        if (choice == 0) break;
        selected.push_back(availableServices[choice - 1]);
        cout << "  Added: " << availableServices[choice - 1].serviceName << "\n";
    }

    // Input validation: an invoice needs at least one billable item
    if (selected.empty()) {
        cout << "  [!] No services selected - defaulting to a Dental Check-up.\n";
        selected.push_back(availableServices[0]);
    }

    return selected;
}

// Pass-by-value: original service list is only read, not modified
static double calculateTotal(vector<ServiceItem> services) {
    double total = 0.0;
    for (int i = 0; i < (int)services.size(); i++) {
        total += services[i].price;
    }
    total += total * GST_RATE; // apply tax
    return total;
}

// Pass-by-reference: directly modifies the existing Payment record
static void applyDiscount(Payment& p, bool isSenior, bool hasInsurance) {
    if (isSenior && hasInsurance) {
        // Nested if: combined discount for senior + insured patients
        p.totalAmount -= p.totalAmount * (SENIOR_DISCOUNT + INSURANCE_COVERAGE);
    } else if (isSenior) {
        p.totalAmount -= p.totalAmount * SENIOR_DISCOUNT;
    } else if (hasInsurance) {
        p.totalAmount -= p.totalAmount * INSURANCE_COVERAGE;
    }

    if (p.totalAmount < 0) {
        p.totalAmount = 0;
    }
}

static char askPaymentMethod() {
    const string label = "  Payment method - 1) Cash  2) Card  3) Insurance: ";
    string note;
    while (true) {
        string line = askInPlace(label, note);
        if (!cin) return '1';

        if (line.length() == 1 && (line[0] == '1' || line[0] == '2' || line[0] == '3')) {
            acceptInPlace(label, line);
            return line[0];
        }
        note = "[enter 1, 2 or 3] ";
    }
}

// ---------------------------------------------------------
// Output
// ---------------------------------------------------------
static void generateReceipt(const Payment& p, const vector<ServiceItem>& services) {
    cout << "\n  ========== RECEIPT ==========\n";
    cout << "  Payment ID: " << p.paymentID << "\n";
    cout << "  Patient ID: " << p.patientID << "\n";
    cout << "  Appointment ID: " << p.appointmentID << "\n";
    cout << "  Date: " << p.date << "\n";
    cout << "  ------------------------------\n";

    // 2D-style itemized breakdown (service name + price)
    for (int i = 0; i < (int)services.size(); i++) {
        cout << "  " << left << setw(22) << services[i].serviceName
             << "RM " << right << setw(8) << fixed << setprecision(2)
             << services[i].price << "\n";
    }

    cout << "  ------------------------------\n";
    cout << "  Payment Method: " << p.method << "\n";
    cout << "  TOTAL PAID: RM " << fixed << setprecision(2) << p.totalAmount << "\n";
    cout << "  ==============================\n";
}

void displayAllPayments() {
    if (paymentHistory.empty()) {
        cout << "\n  No payment records found.\n";
        return;
    }

    cout << "\n  " << left << setw(10) << "ID" << setw(12) << "PatientID"
         << setw(15) << "AppointmentID" << setw(10) << "Method"
         << setw(12) << "Date" << "Amount (RM)\n";
    cout << "  " << string(70, '-') << "\n";

    for (int i = 0; i < (int)paymentHistory.size(); i++) {
        const Payment& p = paymentHistory[i];
        cout << "  " << left << setw(10) << p.paymentID << setw(12) << p.patientID
             << setw(15) << p.appointmentID << setw(10) << p.method
             << setw(12) << p.date << fixed << setprecision(2) << p.totalAmount << "\n";
    }
}

void generateSummaryReport() {
    double totalRevenue = 0.0;
    int cashCount = 0, cardCount = 0, insuranceCount = 0;

    for (int i = 0; i < (int)paymentHistory.size(); i++) {
        totalRevenue += paymentHistory[i].totalAmount;

        if (paymentHistory[i].method == "Cash") cashCount++;
        else if (paymentHistory[i].method == "Card") cardCount++;
        else if (paymentHistory[i].method == "Insurance") insuranceCount++;
    }

    cout << "\n  ===== PAYMENT SUMMARY REPORT =====\n";
    cout << "  Total Transactions: " << paymentHistory.size() << "\n";
    cout << "  Total Revenue: RM " << fixed << setprecision(2) << totalRevenue << "\n";
    cout << "  -----------------------------------\n";
    cout << "  Cash Payments:      " << cashCount << "\n";
    cout << "  Card Payments:      " << cardCount << "\n";
    cout << "  Insurance Payments: " << insuranceCount << "\n";

    if (!paymentHistory.empty()) {
        double average = totalRevenue / paymentHistory.size();
        cout << "  Average Transaction: RM " << fixed << setprecision(2) << average << "\n";
    }
    cout << "  ===================================\n";
}

// ---------------------------------------------------------
// Transaction entry point - called from appointment.cpp after a completed
// appointment has been selected, by either the patient (paying for
// themselves) or reception/admin (billing a patient on their behalf)
// ---------------------------------------------------------
void processPaymentTransaction(const string& appointmentID, const string& patientID) {
    Payment newPayment;
    newPayment.paymentID = generatePaymentID();
    newPayment.patientID = patientID;
    newPayment.appointmentID = appointmentID;
    newPayment.date = todayString();

    // Look up the patient's real record so discounts are automatic
    Patient* patient = findPatientByID(patients, patientID);

    bool isSenior = false;
    bool hasInsurance = false;

    if (patient == nullptr) {
        // Input validation: patientID doesn't match any registered patient
        cout << "  [!] Patient record not found - proceeding without discount eligibility.\n";
    } else {
        isSenior = (patient->user.age >= SENIOR_AGE_THRESHOLD);
        hasInsurance = patient->hasInsurance;
    }

    vector<ServiceItem> services = selectServices();
    newPayment.totalAmount = calculateTotal(services); // automatically calculated, no manual entry

    applyDiscount(newPayment, isSenior, hasInsurance); // automatically applied, no manual entry

    char methodChoice = askPaymentMethod();
    switch (methodChoice) {
        case '1': newPayment.method = "Cash"; break;
        case '2': newPayment.method = "Card"; break;
        case '3': newPayment.method = "Insurance"; break;
        default:  newPayment.method = "Cash"; break;
    }

    paymentHistory.push_back(newPayment);
    savePaymentRecord(newPayment);
    generateReceipt(newPayment, services);

    if (patient != nullptr) {
        cout << "  Discounts applied - Senior: " << (isSenior ? "Yes" : "No")
             << ", Insurance: " << (hasInsurance ? "Yes" : "No") << "\n";
    }
}
