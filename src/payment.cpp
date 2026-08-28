#include "../headers/Payment.h"
#include "../headers/Loyalty.h"
#include "../headers/Console.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <ctime>
#include <cstdlib>
#include <cstring>

using namespace std;

static vector<Payment> paymentHistory;

// A small fixed service list to bill against (could later be loaded from file)
// Not `static` - Loyalty.h declares this `extern` so it can price service-credit prizes
// from the real catalog instead of duplicating names/prices that could drift out of sync.
vector<ServiceItem> availableServices = {
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
// Record format: paymentID,patientID,appointmentID,itemsSummary,totalAmount,method,invoiceDate,paymentDate,status
// itemsSummary uses "; " internally (never a bare comma) so it can't be confused with the field separator.
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
        getline(ss, p.itemsSummary, ',');
        getline(ss, field, ',');
        p.totalAmount = field.empty() ? 0.0 : stod(field);
        getline(ss, p.method, ',');
        getline(ss, p.invoiceDate, ',');
        getline(ss, p.paymentDate, ',');
        getline(ss, p.status, ',');

        // Input validation: skip malformed/incomplete lines (e.g. from an
        // older file format) instead of crashing on a bad record
        if (p.paymentID.empty() || p.status.empty()) continue;

        paymentHistory.push_back(p);
    }
    inFile.close();
}

static void saveAllPaymentRecords() {
    filesystem::create_directories("data");
    ofstream outFile("data/payments.txt"); // rewrite in full - records can change status after creation
    if (!outFile) {
        cout << "  [!] Could not write to data/payments.txt\n";
        return;
    }
    for (size_t i = 0; i < paymentHistory.size(); i++) {
        const Payment& p = paymentHistory[i];
        outFile << p.paymentID << "," << p.patientID << "," << p.appointmentID << ","
                << p.itemsSummary << "," << fixed << setprecision(2) << p.totalAmount << ","
                << p.method << "," << p.invoiceDate << "," << p.paymentDate << ","
                << p.status << "\n";
    }
    outFile.close();
}

// ---------------------------------------------------------
// Lookup
// ---------------------------------------------------------
bool hasInvoiceForAppointment(const string& appointmentID) {
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

static void printItemsSummary(const string& itemsSummary) {
    size_t start = 0;
    while (start < itemsSummary.size()) {
        size_t pos = itemsSummary.find("; ", start);
        string item = (pos == string::npos) ? itemsSummary.substr(start) : itemsSummary.substr(start, pos - start);
        cout << "  - " << item << "\n";
        if (pos == string::npos) break;
        start = pos + 2;
    }
}

// ---------------------------------------------------------
// Service Selection & Calculation (reception only, at invoice time)
// ---------------------------------------------------------
static void printSelectedServices(const vector<ServiceItem>& selected) {
    if (selected.empty()) {
        cout << "  (no services added yet)\n";
        return;
    }
    double runningTotal = 0.0;
    for (size_t i = 0; i < selected.size(); i++) {
        cout << "  " << (i + 1) << ". " << left << setw(22) << selected[i].serviceName
             << "RM " << fixed << setprecision(2) << selected[i].price << "\n";
        runningTotal += selected[i].price;
    }
    cout << "  Subtotal so far: RM " << fixed << setprecision(2) << runningTotal << " (before GST/discount)\n";
}

static vector<ServiceItem> selectServicesForInvoice() {
    vector<ServiceItem> selected;

    while (true) {
        cout << "\n  Services rendered\n";
        cout << "  " << string(40, '-') << "\n";
        for (int i = 0; i < (int)availableServices.size(); i++) {
            cout << "  " << (i + 1) << ". " << left << setw(22) << availableServices[i].serviceName
                 << "RM " << fixed << setprecision(2) << availableServices[i].price << "\n";
        }
        cout << "  " << string(40, '-') << "\n";

        cout << "\n  Currently added to this invoice:\n";
        printSelectedServices(selected);

        cout << "\n  A) Add a service\n";
        cout << "  R) Remove a service\n";
        cout << "  F) Finish (0 added = cancel)\n";

        char action = 'A';
        string note;
        while (true) {
            string line = askInPlace("  Choice (A/R/F): ", note);
            if (!cin) return selected;

            if (line.length() == 1 && strchr("ARFarf", line[0]) != nullptr) {
                action = toupper(line[0]);
                acceptInPlace("  Choice (A/R/F): ", string(1, action));
                break;
            }
            note = "[enter A, R or F] ";
        }

        if (action == 'F') {
            break;
        }

        if (action == 'A') {
            int choice = readMenuChoice("  Add which service (0 to go back): ",
                                         0, (int)availableServices.size());
            if (choice == 0) continue;
            selected.push_back(availableServices[choice - 1]);
            cout << "  Added: " << availableServices[choice - 1].serviceName << "\n";
            continue;
        }

        // action == 'R'
        if (selected.empty()) {
            cout << "  [!] Nothing to remove yet.\n";
            continue;
        }
        cout << "\n  Currently added:\n";
        printSelectedServices(selected);
        int removeChoice = readMenuChoice("  Remove which line (0 to go back): ", 0, (int)selected.size());
        if (removeChoice == 0) continue;
        cout << "  Removed: " << selected[removeChoice - 1].serviceName << "\n";
        selected.erase(selected.begin() + (removeChoice - 1));
    }

    return selected; // empty means "cancel" - the caller decides what that means
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

// ---------------------------------------------------------
// Reception: issue the invoice
// ---------------------------------------------------------
void issueInvoice(const Session& current, const string& appointmentID, const string& patientID) {
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

    vector<ServiceItem> services = selectServicesForInvoice();
    if (services.empty()) {
        cout << "\n  No services selected - invoice cancelled.\n";
        return;
    }

    Payment invoice;
    invoice.paymentID = generatePaymentID();
    invoice.patientID = patientID;
    invoice.appointmentID = appointmentID;
    invoice.totalAmount = calculateTotal(services); // automatically calculated, no manual entry
    applyDiscount(invoice, isSenior, hasInsurance);  // automatically applied, no manual entry
    vector<string> loyaltyLines = applyUnredeemedWin(current, invoice, patientID); // reception picks which reward (if any) to apply, confirmed before it commits
    invoice.method = "";       // not chosen yet - that's the patient's step
    invoice.invoiceDate = todayString();
    invoice.paymentDate = "";  // not paid yet
    invoice.status = "PENDING";

    string itemsSummary;
    for (size_t i = 0; i < services.size(); i++) {
        if (i > 0) itemsSummary += "; ";
        stringstream item;
        item << services[i].serviceName << " (RM" << fixed << setprecision(2) << services[i].price << ")";
        itemsSummary += item.str();
    }
    invoice.itemsSummary = itemsSummary;

    paymentHistory.push_back(invoice);
    saveAllPaymentRecords();

    cout << "\n  ========== INVOICE ISSUED ==========\n";
    cout << "  Invoice ID: " << invoice.paymentID << "\n";
    cout << "  Patient ID: " << invoice.patientID << "\n";
    cout << "  Appointment ID: " << invoice.appointmentID << "\n";
    cout << "  ------------------------------------\n";
    printItemsSummary(invoice.itemsSummary);
    cout << "  ------------------------------------\n";
    if (patient != nullptr) {
        cout << "  Discounts applied - Senior: " << (isSenior ? "Yes" : "No")
             << ", Insurance: " << (hasInsurance ? "Yes" : "No") << "\n";
    }
    for (size_t i = 0; i < loyaltyLines.size(); i++) {
        cout << "  Loyalty reward applied - " << loyaltyLines[i] << "\n";
    }
    cout << "  AMOUNT DUE: RM " << fixed << setprecision(2) << invoice.totalAmount << "\n";
    cout << "  Status: PENDING - the patient can now pay this amount from their own login.\n";
    cout << "  =====================================\n";
}

// ---------------------------------------------------------
// Patient: pay off a pending invoice
// ---------------------------------------------------------
static char askPaymentMethod() {
    const string label = "  Confirm payment - 1) Cash  2) Card  3) Insurance  (0 to cancel): ";
    string note;
    while (true) {
        string line = askInPlace(label, note);
        if (!cin) return '0';

        if (line.length() == 1 && (line[0] == '0' || line[0] == '1' || line[0] == '2' || line[0] == '3')) {
            acceptInPlace(label, line);
            return line[0];
        }
        note = "[enter 0, 1, 2 or 3] ";
    }
}

static void generateReceipt(const Payment& p) {
    cout << "\n  ========== RECEIPT ==========\n";
    cout << "  Payment ID: " << p.paymentID << "\n";
    cout << "  Patient ID: " << p.patientID << "\n";
    cout << "  Appointment ID: " << p.appointmentID << "\n";
    cout << "  ------------------------------\n";
    printItemsSummary(p.itemsSummary);
    cout << "  ------------------------------\n";
    cout << "  Payment Method: " << p.method << "\n";
    cout << "  Invoiced: " << p.invoiceDate << "   Paid: " << p.paymentDate << "\n";
    cout << "  TOTAL PAID: RM " << fixed << setprecision(2) << p.totalAmount << "\n";
    cout << "  ==============================\n";
}

void payForAppointment(const Session& current) {
    vector<int> pendingIndices;
    for (size_t i = 0; i < paymentHistory.size(); i++) {
        if (paymentHistory[i].patientID == current.userId && paymentHistory[i].status == "PENDING") {
            pendingIndices.push_back((int)i);
        }
    }

    if (pendingIndices.empty()) {
        cout << "\n  You have no pending invoices to pay right now.\n";
        cout << "  (Reception issues an invoice once your appointment is marked completed.)\n";
        pauseForKey();
        return;
    }

    cout << "\n  Pending Invoices\n";
    cout << "  " << left << setw(6) << "No." << setw(16) << "Appointment"
         << setw(14) << "Invoiced" << "Amount Due (RM)\n";
    cout << "  " << string(50, '-') << "\n";
    for (size_t i = 0; i < pendingIndices.size(); i++) {
        const Payment& inv = paymentHistory[pendingIndices[i]];
        cout << "  " << left << setw(6) << (i + 1) << setw(16) << inv.appointmentID
             << setw(14) << inv.invoiceDate << fixed << setprecision(2) << inv.totalAmount << "\n";
    }
    cout << "  " << string(50, '-') << "\n";

    int choice = readMenuChoice("\n  Select an invoice to pay by No. (0 to cancel): ",
                                 0, (int)pendingIndices.size());
    if (choice == 0) {
        cout << "  Cancelled.\n";
        return;
    }

    Payment& invoice = paymentHistory[pendingIndices[choice - 1]];

    cout << "\n  Invoice " << invoice.paymentID << " for appointment " << invoice.appointmentID << "\n";
    printItemsSummary(invoice.itemsSummary);
    cout << "  Amount Due: RM " << fixed << setprecision(2) << invoice.totalAmount << "\n";

    char methodChoice = askPaymentMethod();
    if (methodChoice == '0') {
        cout << "  Payment cancelled. Your invoice is still pending.\n";
        pauseForKey();
        return;
    }

    switch (methodChoice) {
        case '1': invoice.method = "Cash"; break;
        case '2': invoice.method = "Card"; break;
        case '3': invoice.method = "Insurance"; break;
    }
    invoice.status = "PAID";
    invoice.paymentDate = todayString();

    saveAllPaymentRecords();
    grantLoyaltyEntryForPayment(invoice.paymentID, invoice.patientID); // 1 raffle ticket per paid invoice
    generateReceipt(invoice);
    pauseForKey();
}

// ---------------------------------------------------------
// Output
// ---------------------------------------------------------
void displayAllPayments() {
    if (paymentHistory.empty()) {
        cout << "\n  No payment records found.\n";
        return;
    }

    cout << "\n  " << left << setw(10) << "ID" << setw(12) << "PatientID"
         << setw(15) << "AppointmentID" << setw(9) << "Status" << setw(10) << "Method"
         << setw(12) << "Invoiced" << "Amount (RM)\n";
    cout << "  " << string(80, '-') << "\n";

    for (int i = 0; i < (int)paymentHistory.size(); i++) {
        const Payment& p = paymentHistory[i];
        cout << "  " << left << setw(10) << p.paymentID << setw(12) << p.patientID
             << setw(15) << p.appointmentID << setw(9) << p.status
             << setw(10) << (p.method.empty() ? "-" : p.method)
             << setw(12) << p.invoiceDate << fixed << setprecision(2) << p.totalAmount << "\n";
    }
}

void generateSummaryReport() {
    double collectedRevenue = 0.0;
    double pendingRevenue = 0.0;
    int cashCount = 0, cardCount = 0, insuranceCount = 0, pendingCount = 0;

    for (int i = 0; i < (int)paymentHistory.size(); i++) {
        const Payment& p = paymentHistory[i];

        if (p.status == "PAID") {
            collectedRevenue += p.totalAmount;
            if (p.method == "Cash") cashCount++;
            else if (p.method == "Card") cardCount++;
            else if (p.method == "Insurance") insuranceCount++;
        } else {
            pendingRevenue += p.totalAmount;
            pendingCount++;
        }
    }

    cout << "\n  ===== PAYMENT SUMMARY REPORT =====\n";
    cout << "  Total Invoices: " << paymentHistory.size() << "\n";
    cout << "  Collected Revenue: RM " << fixed << setprecision(2) << collectedRevenue << "\n";
    cout << "  Pending (Unpaid) : RM " << fixed << setprecision(2) << pendingRevenue
         << " across " << pendingCount << " invoice(s)\n";
    cout << "  -----------------------------------\n";
    cout << "  Cash Payments:      " << cashCount << "\n";
    cout << "  Card Payments:      " << cardCount << "\n";
    cout << "  Insurance Payments: " << insuranceCount << "\n";
    cout << "  ===================================\n";
}

void paymentReportsMenu() {
    int choice;
    do {
        cout << "\n--- Payment Reports ---\n";
        cout << "1. View all payments\n";
        cout << "2. Revenue summary\n";
        cout << "0. Back\n";

        choice = readMenuChoice("Choose: ", 0, 2);

        switch (choice) {
            case 1: displayAllPayments();     pauseForKey(); break;
            case 2: generateSummaryReport();  pauseForKey(); break;
            case 0: break;
        }
    } while (choice != 0);
}
