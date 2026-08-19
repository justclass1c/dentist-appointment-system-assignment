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
    cout << "0. Logout" << endl;
}

void mainMenu(vector<Patient>& patients, const Session& current) {

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
        if (!(ss >> input) || input < 0 || input > 6) {
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

Patient inputPatientDetails(string id, vector<Patient>& patients) {
    Patient p;
    p.user.id = id;

    // name with 0 = go back, befitting Console-style prompts
    string nameNote;
    while (true) {
        string typed = trimInput(askInPlace("Name: ", nameNote));
        if (!cin) return p;

        if (typed == "0") { p.user.name = "0"; return p; }
        if (typed.empty()) { nameNote = "[cannot be blank] "; continue; }
        if (validateName(typed)) {
            p.user.name = typed;
            acceptInPlace("Name: ", typed);
            break;
        }
        nameNote = "[letters/spaces only] ";
    }

    p.user.age = askAge("Age (18-120): ");
    p.user.gender = askGender("Gender (M/F): ");

    string nricNote;
    while (true) {
        string typed = trimInput(askInPlace("NRIC (e.g. 000000-00-0000): ", nricNote));
        if (!cin) { p.user.nric = typed; break; }

        if (typed.empty()) { nricNote = "[cannot be blank] "; continue; }

        p.user.nric = typed;
        if (!validateNRIC(p.user.nric, patients)) {
            p.user.nric.clear();
            nricNote = "[use 000000-00-0000, and not already registered] ";
            continue;
        }
        acceptInPlace("NRIC (e.g. 000000-00-0000): ", typed);
        break;
    }

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

    // added: capture insurance status so Payment module can auto-apply the discount
    char insuranceAns;
    cout << "Do you have dental insurance coverage? (Y/N): ";
    cin >> insuranceAns;
    cin.ignore();
    p.hasInsurance = (toupper(insuranceAns) == 'Y');

    return p;
}

void createPatient(Patient patient, vector<Patient>& patients) {
    patients.push_back(patient);
    savePatients(patients);
}

void registerPatient(vector<Patient>& patients) {
    cout << "\nNew patient registration. (0 at Name to go back)" << endl;

    while (true) {
        Patient p = inputPatientDetails(nextPatientId(patients), patients);

        if (trimInput(p.user.name) == "0") {
            cout << "Registration cancelled." << endl;
            return;
        }

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
            createPatient(p, patients);
            cout << "\nRegistered. Your patient ID is " << p.user.id
                << " - log in with your ID and password." << endl;
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
    string id, password;

    cout << "Please login. (0 to go back)" << endl;

    string note;
    while (true) {
        clearLine();
        cout << note << "ID: " << flush;
        getline(cin, id);
        stayOnPromptLine();

        if (trimInput(id) == "0" || !cin) {
            clearLine();
            cout << "Login cancelled." << endl;
            return;
        }

        if (trimInput(id).empty()) { note = "[cannot be blank] "; continue; }
        if (verifyID(patients, id)) break;
        note = "[no account with that ID] ";
    }
    clearLine();
    cout << "ID: " << id << endl;

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

        Patient* account = findPatientByID(patients, id);
        if (account != nullptr && password == account->user.password) break;
        note = "[incorrect password] ";
    }
    clearLine();
    cout << "Password: " << string(password.length(), '*') << endl;

    assignCurrentUser(patients, id);

    Session current;
    current.role = PATIENT;
    for (const Patient& p : patients) {
        if (p.user.id == id) {
            current.userId   = p.user.id;
            current.name     = p.user.name;
            current.password = p.user.password;
            break;
        }
    }

    current.name = getUsername(patients, id);

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

void viewPatientProfile(vector<Patient>& patients, string currentUserID) {
    Patient* target = findPatientByID(patients, currentUserID);
    if (target == nullptr) return;

    cout << "\nYour Profile:" << endl;
    cout << "Patient ID: " << target->user.id << endl;
    cout << "Name: " << target->user.name << endl;
    cout << "Age: " << target->user.age << endl;
    char g = toupper(target->user.gender);
    cout << "Gender: " << (g == 'M' ? "Male" : g == 'F' ? "Female" : "Not recorded") << endl;
    cout << "Email: " << target->user.email << endl;
    cout << "NRIC: " << target->user.nric << endl;
    cout << "Contact: " << target->user.phoneNo << endl;
    cout << "Insurance: " << (target->hasInsurance ? "Yes" : "No") << endl;

    cout << "\nOptions: M = Modify, Q = Quit" << endl;

    char input;
    cin >> input;
    cin.ignore();

    if (toupper(input) == 'M') modifyPatient(patients, *target);
}

void modifyPatient(vector<Patient>& patients, Patient& patient) {
    int index = 0;
    string password, input;

    do {
        cout << "\nChoose a field to change." << endl;
        cout << "Your Profile:" << endl;
        cout << "1. Name   : " << patient.user.name << endl;
        cout << "2. Age    : " << patient.user.age << endl;
        cout << "3. Email  : " << patient.user.email << endl;
        cout << "4. Contact: " << patient.user.phoneNo << endl;
        cout << "0. Done" << endl;

        cin >> index;
        cin.ignore();

        switch (index) {
            case 1:
                cout << "Change your name: ";
                getline(cin, input);
                do {
                    cout << "Enter password: ";
                    getline(cin, password);
                } while (password != patient.user.password);
                patient.user.name = input;
                savePatients(patients);
                cout << "Saved." << endl;
                break;

            case 2:
                cout << "Change your age: ";
                cin >> input;
                cin.ignore();
                while (!validatePatientAge(stoi(input))) {
                    cout << "Invalid age (18-120). Try again: ";
                    cin >> input;
                    cin.ignore();
                }
                do {
                    cout << "Enter password: ";
                    getline(cin, password);
                } while (password != patient.user.password);
                patient.user.age = stoi(input);
                savePatients(patients);
                cout << "Saved." << endl;
                break;

            case 3:
                for (;;) {
                    cout << "Change your email: ";
                    getline(cin, input);

                    if (input.empty()) { cout << "[cannot be blank] "; continue; }

                    patient.user.email = input;
                    if (!validateEmail(patient, patients)) {
                        patient.user.email.clear();
                        cout << "[use name@example.com, and not already registered] ";
                        continue;
                    }
                    break;
                }
                do {
                    cout << "Enter password: ";
                    getline(cin, password);
                } while (password != patient.user.password);
                savePatients(patients);
                cout << "Saved." << endl;
                break;

            case 4:
                cout << "Change your contact: ";
                getline(cin, input);
                while (!validatePhoneNo(input)) {
                    cout << "Invalid phone (01x-xxx xxxx). Try again: ";
                    getline(cin, input);
                }
                do {
                    cout << "Enter password: ";
                    getline(cin, password);
                } while (password != patient.user.password);
                patient.user.phoneNo = input;
                savePatients(patients);
                cout << "Saved." << endl;
                break;

            case 0:
                break;

            default:
                cout << "Invalid input." << endl;
                break;
        }
    } while (index != 0);
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

void viewPatients(vector<Patient>& patients) {
    if (patients.empty()) {
        cout << "\nNo patients are registered yet.\n";
        return;
    }

    cout << "\n  " << left << setw(8) << "ID" << setw(24) << "Name"
        << setw(28) << "Email" << "Age\n";
    cout << "  " << string(64, '-') << "\n";
    for (size_t i = 0; i < patients.size(); i++) {
        const Patient& p = patients[i];
        cout << "  " << left << setw(8) << p.user.id
            << setw(24) << (p.user.name.empty() ? "(no name on record)" : p.user.name)
            << setw(28) << p.user.email << p.user.age << "\n";
    }
    cout << "  " << string(64, '-') << "\n";
    cout << "  " << patients.size() << " patient(s).\n";
    pauseForKey();
}