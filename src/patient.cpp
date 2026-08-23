#include "../headers/Patient.h"
#include "../headers/Appointment.h"
#include "../headers/Console.h"
#include "../headers/Validation.h"
#include "../src/validation.cpp"
#include "../headers/Verification.h"
#include "../src/verification.cpp"
#include "../src/users.cpp"
#include <iostream>
#include <ostream>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstdlib>

using namespace std;

static string trimInput(const string& text) {
    size_t first = text.find_first_not_of(" \t\r\n");
    if (first == string::npos) return "";
    size_t last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

static string nextPatientId(const vector<Patient>& patients) {
    int highest = 0;
    for (size_t i = 0; i < patients.size(); i++) {
        const string& id = patients[i].user.id;
        if (id.length() < 2 || toupper(id[0]) != 'P') continue;
        int number = atoi(id.substr(1).c_str());
        if (number > highest) highest = number;
    }
    stringstream ss;
    ss << "P" << setw(3) << setfill('0') << (highest + 1);
    return ss.str();
}

static void printPatientMenu(const Session& current) {
    cout << "\nWelcome, " << current.name << "!" << endl;
    cout << "1. Schedule an appointment" << endl;
    cout << "2. View my appointments" << endl;
    cout << "3. Modify an appointment" << endl;
    cout << "4. Cancel an appointment" << endl;
    cout << "5. Find the next available slot" << endl;
    cout << "6. View profile" << endl;
    cout << "7. Make a payment" << endl;
    cout << "0. Logout" << endl;
}

void mainMenu(vector<Patient> patients, const Session& current) {

    string line, note;
    bool showMenu = true;

    while (true) {
        if (showMenu) {
            printPatientMenu(current);
            showMenu = false;
        }

        clearLine();
        cout << note << "Choice: " << flush;
        getline(cin, line);
        stayOnPromptLine();

        if (!cin) { cout << "\nLogging out." << endl; return; }

        int input = -1;
        stringstream ss(line);
        if (!(ss >> input) || input < 0 || input > 7) {
            note = "[invalid input, try again] ";
            continue;
        }

        clearLine();
        cout << "Choice: " << input << endl;
        note.clear();

        if (input == 0) {
            cout << "Logging out." << endl;
            return;
        }

        switch (input) {
            case 1: scheduleAppointment(current); break;
            case 2: viewAppointments(current);    break;
            case 3: modifyAppointment(current);   break;
            case 4: cancelAppointment(current);   break;
            case 5: findNextAvailable();          break;
            case 6:
                viewPatientProfile(patients, currentUserID);
                pauseForKey();
                break;
            case 7: payForAppointment(current);   break;
        }
        showMenu = true;
    }
}

static string askField(const string& label, const string& failMsg, bool (*isValid)(string)) {
    string note, answer;
    while (true) {
        answer = trimInput(askInPlace(label, note));
        if (!cin) return answer;

        if (answer.empty()) {
            note = "[cannot be blank] ";
        } else if (!isValid(answer)) {
            note = failMsg;
        } else {
            acceptInPlace(label, answer);
            return answer;
        }
    }
}

static int askAge(const string& label) {
    string note;
    while (true) {
        string line = trimInput(askInPlace(label, note));
        if (!cin) return 0;

        int value = 0;
        stringstream parse(line);
        if ((parse >> value) && validatePatientAge(value)) {
            stringstream shown;
            shown << value;
            acceptInPlace(label, shown.str());
            return value;
        }
        note = "[18-120] ";
    }
}

static char askGender(const string& label) {
    string note;
    while (true) {
        string line = trimInput(askInPlace(label, note));
        if (!cin) return 'M';

        if (line.length() == 1 && validateGender(line[0])) {
            char typed = toupper(line[0]);
            acceptInPlace(label, string(1, typed));
            return typed;
        }
        note = "[M or F] ";
    }
}

static char askLetter(const string& label, const string& allowed, const string& failMsg) {
    string note;
    while (true) {
        string line = trimInput(askInPlace(label, note));
        if (!cin) {

            size_t no = allowed.find('N');
            return (no == string::npos) ? allowed[0] : 'N';
        }

        if (line.length() == 1) {
            char typed = toupper(line[0]);
            if (allowed.find(typed) != string::npos) {
                acceptInPlace(label, string(1, typed));
                return typed;
            }
        }
        note = failMsg;
    }
}

void registerPatient(vector<Patient>& patients) {
    Patient p;

    cout << "\nNew patient registration. (0 at Name to go back)" << endl;

    while (true) {

        p.user.id = nextPatientId(patients);

        string note, name;
        while (true) {
            name = trimInput(askInPlace("Name: ", note));
            if (!cin || name == "0") {
                clearLine();
                cout << "Registration cancelled. Nothing was saved." << endl;
                return;
            }
            if (name.empty())          { note = "[cannot be blank] "; continue; }
            if (!validateName(name))   { note = "[letters, spaces, - and ' only] "; continue; }
            acceptInPlace("Name: ", name);
            break;
        }
        p.user.name = name;

        p.user.age     = askAge("Age (18-120): ");
        p.user.gender  = askGender("Gender (M/F): ");
        
        string nricNote;
        while (true) {
            string typed = trimInput(askInPlace("NRIC (e.g. 010101-01-0101): ", nricNote));
            if (!cin) { p.user.nric = typed; break; }

            if (typed.empty()) { nricNote = "[cannot be blank] "; continue; }
            if (!validateNRIC(typed, patients)) {
                nricNote = "[use xxxxxx-xx-xxxx, and not already registered] ";
                continue;
            }
            acceptInPlace("NRIC (e.g. 010101-01-0101): ", typed);
            p.user.nric = typed;
            break;
        }

        /* validateEmail() reads targetUser.user.email, so the typed value is
         * written into the record first and rolled back if it is rejected. */
        string emailNote;
        while (true) {
            string typed = trimInput(askInPlace("Email (e.g. name@example.com): ", emailNote));
            if (!cin) { p.user.email = typed; break; }

            if (typed.empty()) { emailNote = "[cannot be blank] "; continue; }

            p.user.email = typed;
            if (!validateEmail(p, patients)) {
                p.user.email.clear();
                emailNote = "[use name@example.com, and not already registered] ";
                continue;
            }
            acceptInPlace("Email (e.g. name@example.com): ", typed);
            break;
        }

        p.user.password = askField("Password: ", "[cannot be blank] ", validatePassword);
        p.user.phoneNo = askField("Phone Number (e.g. 012-345 6789): ", "[use 01x-xxx xxxx] ", validatePhoneNo);

        p.allergies = trimInput(askInPlace("Allergies (or 'none'): ", ""));
        if (p.allergies.empty()) p.allergies = "none";
        acceptInPlace("Allergies (or 'none'): ", p.allergies);

        // capture insurance status at registration so the Payment module can
        // auto-apply the insurance discount later without re-asking
        p.hasInsurance = (askLetter("Do you have dental insurance coverage? (Y/N): ",
                                    "YN", "[Y or N] ") == 'Y');

        cout << "\nYour Profile (Patient ID: " << p.user.id << ")" << endl;
        cout << "Name: "      << p.user.name    << endl;
        cout << "Age: "       << p.user.age     << endl;
        cout << "Gender: "    << p.user.gender  << endl;
        cout << "NRIC: "      << p.user.nric    << endl;
        cout << "Email: "     << p.user.email   << endl;
        cout << "Phone No.: " << p.user.phoneNo << endl;
        cout << "Allergies: " << p.allergies    << endl;
        cout << "Insurance: " << (p.hasInsurance ? "Yes" : "No") << endl;

        char answer = askLetter("Confirm registration? (Y = Yes / N = No / M = Modify): ",
                                "YNM", "[Y, N or M] ");

        if (answer == 'Y') {
            patients.push_back(p);
            savePatients(patients);
            cout << "\nRegistered. Your patient ID is " << p.user.id
                << " - log in with your email and password." << endl;
            return;
        }
        if (answer == 'N') {
            cout << "\nRegistration discarded. Nothing was saved." << endl;
            return;
        }
        cout << "\nRe-enter your details." << endl;
    }
}

void loginPatient(vector<Patient>& patients) {
    string email, password;

    cout << "Please login. (0 to go back)" << endl;

    string note;
    while (true) {
        clearLine();
        cout << note << "Email: " << flush;
        getline(cin, email);
        stayOnPromptLine();

        if (trimInput(email) == "0" || !cin) {
            clearLine();
            cout << "Login cancelled." << endl;
            return;
        }

        if (trimInput(email).empty()) { note = "[cannot be blank] "; continue; }
        if (verifyEmail(patients, email)) break;
        note = "[no account with that email] ";
    }
    clearLine();
    cout << "Email: " << email << endl;

    note.clear();
    while (true) {
        clearLine();
        cout << note << "Password: " << flush;
        getline(cin, password);
        stayOnPromptLine();

        if (trimInput(password) == "0" || !cin) {
            clearLine();
            cout << "Login cancelled." << endl;
            return;
        }

        if (password.empty()) { note = "[cannot be blank] "; continue; }
        if (verifyPassword(patients, email, password)) break;
        note = "[incorrect password] ";
    }
    clearLine();
    cout << "Password: " << string(password.length(), '*') << endl;

    assignCurrentUser(patients, email);

    Session current;
    current.role = PATIENT;
    for (const Patient& p : patients) {
        if (p.user.email == email) {
            current.userId   = p.user.id;
            current.name     = p.user.name;
            current.password = p.user.password;
            break;
        }
    }

    mainMenu(patients, current);
}

vector<Patient> loadPatients() {
    ifstream inFile("data/patients.txt");

    vector<Patient> p;

    if (!inFile.is_open()) return p;

    string id, name, nric, email, password, phoneNo, allergies;
    int age;
    char gender;

    string line;

    while(getline(inFile, line)) {
        stringstream ss(line);

        string ageStr, genderStr, insuranceStr;
        getline(ss, id, ';');
        getline(ss, name, ';');
        getline(ss, ageStr, ';');    age = toIntOr(ageStr, 0);
        getline(ss, genderStr, ';'); gender = genderStr.empty() ? '?' : genderStr[0];
        getline(ss, nric, ';');
        getline(ss, email, ';');
        getline(ss, password, ';');
        getline(ss, phoneNo, ';');
        getline(ss, allergies, ';');
        getline(ss, insuranceStr); // added: last field, no trailing delimiter

        if (trimInput(id).empty()) continue;
        if (trimInput(email).empty()) continue;

        Patient patient;
        patient.user.id = id;
        patient.user.name = name;
        patient.user.age = age;
        patient.user.gender = gender;
        patient.user.nric = nric;
        patient.user.email = email;
        patient.user.password = password;
        patient.user.phoneNo = phoneNo;
        patient.allergies = allergies;
        patient.hasInsurance = (!insuranceStr.empty() && insuranceStr[0] == '1'); // added

        p.push_back(patient);
    }

    inFile.close();

    return p;
}

void savePatients(vector<Patient> patients) {
    filesystem::create_directories("data");

    ofstream outFile("data/patients.txt");

    for (Patient patient : patients) {
        outFile << patient.user.id << ";" << patient.user.name << ";" << patient.user.age << ";" << patient.user.gender << ";" << patient.user.nric << ";" << patient.user.email << ";" << patient.user.password << ";" << patient.user.phoneNo << ";" << patient.allergies << ";" << (patient.hasInsurance ? "1" : "0") << endl;
    }

    outFile.close();
}

void viewPatientProfile(vector<Patient> patients, string currentUserID) {
    for (Patient patient : patients) {
        if (currentUserID == patient.user.id) {
            cout << "Your Profile:" << endl;
            cout << "Patient ID: " << patient.user.id << endl;
            cout << "Name: " << patient.user.name << endl;
            cout << "Age: " << patient.user.age << endl;
            char g = toupper(patient.user.gender);
            cout << "Gender: " << (g == 'M' ? "Male" : g == 'F' ? "Female" : "Not recorded") << endl;
            cout << "Email: " << patient.user.email << endl;
            cout << "NRIC: " << patient.user.nric << endl;
            cout << "Contact: " << patient.user.phoneNo << endl;
            cout << "Insurance: " << (patient.hasInsurance ? "Yes" : "No") << endl;

            break;
        }
    }
}

// added: lookup helper so the Payment module can pull a patient's age/insurance
// status directly by ID instead of asking the receptionist to re-enter it
Patient* findPatientByID(vector<Patient>& patients, const string& id) {
    for (Patient& patient : patients) {
        if (patient.user.id == id) {
            return &patient;
        }
    }
    return nullptr;
}
