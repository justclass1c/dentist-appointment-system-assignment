#include "../headers/Payment.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

using namespace std;

static vector<ServiceItem> availableServices = {
    {"Dental Check-up", 50.00},
    {"Tooth Extraction", 120.00},
    {"Scaling & Polishing", 90.00},
    {"Root Canal Treatment", 450.00},
    {"Dental Filling", 80.00},
    {"Teeth Whitening", 300.00}
};

// Menu / Program Flow
void displayPaymentMenu() {
    cout << "\n===== PAYMENT MODULE =====\n";
    cout << "1. Process New Payment\n";
    cout << "2. View All Payment Records\n";
    cout << "3. View Summary Report\n";
    cout << "4. Exit\n";
    cout << "Enter your choice: ";
}

void runPaymentModule(vector<Payment>& paymentHistory, vector<Patient>& patients) {
    loadPaymentRecords(paymentHistory);

    int choice;
    do {
        displayPaymentMenu();
        cin >> choice;

        // Input validation: invalid menu choice
        if (cin.fail()) {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "Invalid input. Please enter a number.\n";
            continue;
        }

        switch (choice) {
            case 1: {
                string patientID, appointmentID;
                cout << "Enter Patient ID: ";
                cin >> patientID;
                cout << "Enter Appointment ID: ";
                cin >> appointmentID;
                processPayment(paymentHistory, patients, patientID, appointmentID);
                break;
            }
            case 2:
                displayAllPayments(paymentHistory);
                break;
            case 3:
                generateSummaryReport(paymentHistory);
                break;
            case 4:
                cout << "Exiting Payment Module...\n";
                break;
            default:
                cout << "Invalid menu choice. Please select 1-4.\n";
                break;
        }
    } while (choice != 4);
}

// Service Selection & Calculation
vector<ServiceItem> selectServices() {
    vector<ServiceItem> selected;
    int choice;

    cout << "\nAvailable Services:\n";
    for (int i = 0; i < (int)availableServices.size(); i++) {
        cout << i + 1 << ". " << left << setw(22) << availableServices[i].serviceName
             << "RM " << fixed << setprecision(2) << availableServices[i].price << "\n";
    }

    do {
        cout << "Select a service (1-" << availableServices.size() << "), or 0 to finish: ";
        cin >> choice;

        // Input validation: invalid service selection
        while (cin.fail() || choice < 0 || choice > (int)availableServices.size()) {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "Invalid service selection. Try again: ";
            cin >> choice;
        }

        if (choice != 0) {
            selected.push_back(availableServices[choice - 1]);
            cout << availableServices[choice - 1].serviceName << " added.\n";
        }
    } while (choice != 0);

    return selected;
}


double calculateTotal(vector<ServiceItem> services) {
    double total = 0.0;
    for (int i = 0; i < (int)services.size(); i++) {
        total += services[i].price;
    }
    total += total * GST_RATE; // apply tax
    return total;
}


void applyDiscount(Payment& p, bool isSenior, bool hasInsurance) {
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

// Payment Processing

Payment processPayment(vector<Payment>& paymentHistory, vector<Patient>& patients,
                        const string& patientID, const string& appointmentID) {
    Payment newPayment;
    newPayment.paymentID = generatePaymentID(paymentHistory);
    newPayment.patientID = patientID;
    newPayment.appointmentID = appointmentID;
    newPayment.date = "10-08-2026"; // placeholder - could pull from system clock

    // Look up the patient's real record so discounts are automatic -
    // no more manually asking "are you a senior?" every time
    Patient* patient = findPatientByID(patients, patientID);

    bool isSenior = false;
    bool hasInsurance = false;

    if (patient == nullptr) {
        // Input validation: patientID doesn't match any registered patient.
        // Payment can still proceed (e.g. walk-in), just without discount eligibility.
        cout << "Warning: Patient ID not found. Proceeding without discount eligibility.\n";
    } else {
        isSenior = (patient->user.age >= SENIOR_AGE_THRESHOLD);
        hasInsurance = patient->hasInsurance;
    }

    vector<ServiceItem> services = selectServices();
    newPayment.totalAmount = calculateTotal(services); 

    applyDiscount(newPayment, isSenior, hasInsurance); 

    int methodChoice;
    cout << "\nSelect Payment Method:\n1. Cash\n2. Card\n3. Insurance\nChoice: ";
    cin >> methodChoice;

    switch (methodChoice) {
        case 1: newPayment.method = "Cash"; break;
        case 2: newPayment.method = "Card"; break;
        case 3: newPayment.method = "Insurance"; break;
        default:
            cout << "Invalid method selected. Defaulting to Cash.\n";
            newPayment.method = "Cash";
            break;
    }

    // Duplicate ID check before saving (safety net; IDs are auto-generated)
    while (isDuplicatePaymentID(paymentHistory, newPayment.paymentID)) {
        newPayment.paymentID = generatePaymentID(paymentHistory);
    }

    paymentHistory.push_back(newPayment);
    savePaymentRecord(newPayment);
    generateReceipt(newPayment, services);

    // Let the receptionist/patient see why the discount was (or wasn't) applied
    if (patient != nullptr) {
        cout << "Discounts applied - Senior: " << (isSenior ? "Yes" : "No")
             << ", Insurance: " << (hasInsurance ? "Yes" : "No") << "\n";
    }

    return newPayment;
}

// Returned value: generates a new unique ID based on current record count
string generatePaymentID(const vector<Payment>& paymentHistory) {
    int nextNumber = (int)paymentHistory.size() + 1;
    stringstream ss;
    ss << "PAY" << setw(4) << setfill('0') << nextNumber;
    return ss.str();
}

bool isDuplicatePaymentID(const vector<Payment>& paymentHistory, const string& id) {
    for (int i = 0; i < (int)paymentHistory.size(); i++) {
        if (paymentHistory[i].paymentID == id) {
            return true;
        }
    }
    return false;
}

// Output
void generateReceipt(const Payment& p, const vector<ServiceItem>& services) {
    cout << "\n========== RECEIPT ==========\n";
    cout << "Payment ID: " << p.paymentID << "\n";
    cout << "Patient ID: " << p.patientID << "\n";
    cout << "Appointment ID: " << p.appointmentID << "\n";
    cout << "Date: " << p.date << "\n";
    cout << "------------------------------\n";

    // 2D-style itemized breakdown (service name + price)
    for (int i = 0; i < (int)services.size(); i++) {
        cout << left << setw(22) << services[i].serviceName
             << "RM " << right << setw(8) << fixed << setprecision(2)
             << services[i].price << "\n";
    }

    cout << "------------------------------\n";
    cout << "Payment Method: " << p.method << "\n";
    cout << "TOTAL PAID: RM " << fixed << setprecision(2) << p.totalAmount << "\n";
    cout << "==============================\n";
}

void displayAllPayments(const vector<Payment>& paymentHistory) {
    if (paymentHistory.empty()) {
        cout << "\nNo payment records found.\n";
        return;
    }

    cout << "\n" << left << setw(10) << "ID" << setw(12) << "PatientID"
         << setw(15) << "AppointmentID" << setw(10) << "Method"
         << setw(12) << "Date" << "Amount (RM)\n";
    cout << string(70, '-') << "\n";

    for (int i = 0; i < (int)paymentHistory.size(); i++) {
        const Payment& p = paymentHistory[i];
        cout << left << setw(10) << p.paymentID << setw(12) << p.patientID
             << setw(15) << p.appointmentID << setw(10) << p.method
             << setw(12) << p.date << fixed << setprecision(2) << p.totalAmount << "\n";
    }
}

void generateSummaryReport(const vector<Payment>& paymentHistory) {
    double totalRevenue = 0.0;
    int cashCount = 0, cardCount = 0, insuranceCount = 0;

    for (int i = 0; i < (int)paymentHistory.size(); i++) {
        totalRevenue += paymentHistory[i].totalAmount;

        if (paymentHistory[i].method == "Cash") cashCount++;
        else if (paymentHistory[i].method == "Card") cardCount++;
        else if (paymentHistory[i].method == "Insurance") insuranceCount++;
    }

    cout << "\n===== PAYMENT SUMMARY REPORT =====\n";
    cout << "Total Transactions: " << paymentHistory.size() << "\n";
    cout << "Total Revenue: RM " << fixed << setprecision(2) << totalRevenue << "\n";
    cout << "-----------------------------------\n";
    cout << "Cash Payments:      " << cashCount << "\n";
    cout << "Card Payments:      " << cardCount << "\n";
    cout << "Insurance Payments: " << insuranceCount << "\n";

    if (!paymentHistory.empty()) {
        double average = totalRevenue / paymentHistory.size();
        cout << "Average Transaction: RM " << fixed << setprecision(2) << average << "\n";
    }
    cout << "===================================\n";
}

// File Processing
void savePaymentRecord(const Payment& p) {
    ofstream outFile("data/payments.txt", ios::app);
    if (!outFile) {
        cout << "Error: Could not open payments.txt for writing.\n";
        return;
    }
    outFile << p.paymentID << "," << p.patientID << "," << p.appointmentID << ","
            << fixed << setprecision(2) << p.totalAmount << "," << p.method << ","
            << p.date << "\n";
    outFile.close();
}

void loadPaymentRecords(vector<Payment>& paymentHistory) {
    ifstream inFile("data/payments.txt");
    if (!inFile) {
        return; 
    }

    string line;
    while (getline(inFile, line)) {
        stringstream ss(line);
        string field;
        Payment p;

        getline(ss, p.paymentID, ',');
        getline(ss, p.patientID, ',');
        getline(ss, p.appointmentID, ',');
        getline(ss, field, ',');
        p.totalAmount = stod(field);
        getline(ss, p.method, ',');
        getline(ss, p.date, ',');

        paymentHistory.push_back(p);
    }
    inFile.close();
}
