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

using namespace std;

static string trimInput(const string& text) {
    size_t first = text.find_first_not_of(" \t\r\n");
    if (first == string::npos) return "";
    size_t last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

static void printPatientMenu(const Session& current) {
    cout << "\nWelcome, " << current.name << "!" << endl;
    cout << "1. Schedule an appointment" << endl;
    cout << "2. View my appointments" << endl;
    cout << "3. Modify an appointment" << endl;
    cout << "4. Cancel an appointment" << endl;
    cout << "5. Find the next available slot" << endl;
    cout << "6. View Profile" << endl;
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
            case 6: viewPatientProfile(patients, currentUserID); break;
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
        note = "[1-120] ";
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

        stringstream idStream;
        idStream << "P" << setw(3) << setfill('0') << (patients.size() + 1);
        p.user.id = idStream.str();

        string note, name;
        while (true) {
            name = trimInput(askInPlace("Name: ", note));
            if (!cin || name == "0") {
                clearLine();
                cout << "Registration cancelled. Nothing was saved." << endl;
                return;
            }
            if (name.empty())          { note = "[cannot be blank] "; continue; }
            if (!validateName(name))   { note = "[not accepted] ";    continue; }
            acceptInPlace("Name: ", name);
            break;
        }
        p.user.name = name;

        p.user.age     = askAge("Age: ");
        p.user.gender  = askLetter("Gender (M/F): ", "MF", "[M or F] ");
        p.user.nric    = askField("NRIC: ", "[use xxxxxx-xx-xxxx] ", validateNRIC);
        p.user.email   = askField("Email: ", "[use name@example.com] ", validateEmail);
        p.user.password= askField("Password: ", "[not accepted] ", validatePassword);
        p.user.phoneNo = askField("Phone Number: ", "[use 01x-xxx xxxx] ", validatePhoneNo);

        p.allergies = trimInput(askInPlace("Allergies (or 'none'): ", ""));
        if (p.allergies.empty()) p.allergies = "none";
        acceptInPlace("Allergies (or 'none'): ", p.allergies);

        cout << "\nYour Profile (Patient ID: " << p.user.id << ")" << endl;
        cout << "Name: "      << p.user.name    << endl;
        cout << "Age: "       << p.user.age     << endl;
        cout << "Gender: "    << p.user.gender  << endl;
        cout << "NRIC: "      << p.user.nric    << endl;
        cout << "Email: "     << p.user.email   << endl;
        cout << "Phone No.: " << p.user.phoneNo << endl;
        cout << "Allergies: " << p.allergies    << endl;

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

        if (verifyPassword(patients, password)) break;
        note = "[incorrect password] ";
    }
    clearLine();
    cout << "Password: " << string(password.length(), '*') << endl;

    assignCurrentUser(PATIENT, patients, email);

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

        string ageStr, genderStr;
        getline(ss, id, ';');
        getline(ss, name, ';');
        getline(ss, ageStr, ';');    age = stoi(ageStr);
        getline(ss, genderStr, ';'); gender = genderStr[0];
        getline(ss, nric, ';');
        getline(ss, email, ';');
        getline(ss, password, ';');
        getline(ss, phoneNo, ';');
        getline(ss, allergies);

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

        p.push_back(patient);
    }

    inFile.close();

    return p;
}

void savePatients(vector<Patient> patients) {
    filesystem::create_directories("data");

    ofstream outFile("data/patients.txt");

    for (Patient patient : patients) {
        outFile << patient.user.id << ";" << patient.user.name << ";" << patient.user.age << ";" << patient.user.gender << ";" << patient.user.nric << ";" << patient.user.email << ";" << patient.user.password << ";" << patient.user.phoneNo << ";" << patient.allergies << endl;
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
            cout << "Gender: " << (patient.user.gender == 'M' ? "Male" : "Female") << endl;
            cout << "Email: " << patient.user.email << endl;
            cout << "NRIC: " << patient.user.nric << endl;
            cout << "Contact: " << patient.user.phoneNo << endl;

            break;
        }
    }
}
