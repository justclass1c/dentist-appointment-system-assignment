#include "../headers/Dentist.h"
#include "../headers/Appointment.h"
#include "../headers/Console.h"

const string DENTIST_FILE = "dentists.txt";
const string SLOT_FILE = "slots.txt";

const string RECEPTION_USERNAME = "reception";
const string RECEPTION_PASSWORD = "pass123";

vector<Dentist> dentists;
vector<TimeSlot> slots;

string trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

vector<string> split(const string& s, char delim) {
    vector<string> tokens;
    stringstream ss(s);
    string token;
    while (getline(ss, token, delim)) {
        tokens.push_back(trim(token));
    }
    return tokens;
}

static string askNonBlank(const string& label) {
    string note, answer;
    while (true) {
        answer = trim(askInPlace(label, note));
        if (!cin) return answer;

        if (answer.empty()) {
            note = "[cannot be blank] ";
        } else if (answer.find(',') != string::npos) {
            note = "[no commas - they break the file] ";
        } else {
            acceptInPlace(label, answer);
            return answer;
        }
    }
}

void loadDentists() {
    ifstream file(DENTIST_FILE);
    if (!file.is_open()) return;
    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        vector<string> parts = split(line, ',');
        if (parts.size() < 5) continue;
        Dentist d;
        d.user.id = parts[0];
        d.id = parts[0];
        d.user.name = parts[1];
        d.user.age = stoi(parts[2]);
        d.user.email = parts[3];
        d.user.password = parts[4];
        dentists.push_back(d);
    }
    file.close();
}

void saveDentists() {
    ofstream file(DENTIST_FILE);
    for (const auto& d : dentists) {
        file << d.user.id << "," << d.user.name << "," << d.user.age << ","
             << d.user.email << "," << d.user.password << "\n";
    }
    file.close();
}

void loadSlots() {
    ifstream file(SLOT_FILE);
    if (!file.is_open()) return;
    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        vector<string> parts = split(line, ',');
        if (parts.size() < 4) continue;
        TimeSlot s;
        s.dentistId = parts[0];
        s.start = parts[1];
        s.end = parts[2];
        s.available = (parts[3] == "1");
        slots.push_back(s);
    }
    file.close();
}

void saveSlots() {
    ofstream file(SLOT_FILE);
    for (const auto& s : slots) {
        file << s.dentistId << "," << s.start << "," << s.end << ","
             << (s.available ? "1" : "0") << "\n";
    }
    file.close();
}

Dentist* findDentistById(const string& id) {
    for (auto& d : dentists) {
        if (d.id == id) return &d;
    }
    return nullptr;
}

Dentist* findDentistByName(const string& name) {
    for (auto& d : dentists) {
        if (d.user.name == name) return &d;
    }
    return nullptr;
}

vector<TimeSlot> getSlotsForDentist(const string& dentistId) {
    vector<TimeSlot> result;
    for (const auto& s : slots) {
        if (s.dentistId == dentistId) result.push_back(s);
    }
    return result;
}

void addOrUpdateSlot(const TimeSlot& slot) {
    for (auto& s : slots) {
        if (s.dentistId == slot.dentistId && s.start == slot.start && s.end == slot.end) {
            s.available = slot.available;
            return;
        }
    }
    slots.push_back(slot);
}

bool removeSlot(const string& dentistId, const string& start, const string& end) {
    for (auto it = slots.begin(); it != slots.end(); ++it) {
        if (it->dentistId == dentistId && it->start == start && it->end == end) {
            slots.erase(it);
            return true;
        }
    }
    return false;
}

void displayDentistInfo(const Dentist& d) {
    cout << "ID: " << d.id << "\n";
    cout << "Name: " << d.user.name << "\n";
    cout << "Age: " << d.user.age << "\n";
    cout << "Email: " << d.user.email << "\n";
    cout << "Password: " << d.user.password << "\n";
}

void displaySlots(const vector<TimeSlot>& slotList) {
    if (slotList.empty()) {
        return;
    }
    cout << "Dentist ID\tStart\tEnd\tStatus\n";
    for (const auto& s : slotList) {
        cout << s.dentistId << "\t\t" << s.start << "\t" << s.end << "\t"
             << (s.available ? "Available" : "Locked") << "\n";
    }
}

void adminRegisterDentist() {
    Dentist d;

    if (dentists.empty()) {
        d.id = "D001";
    } else {
        string lastId = dentists.back().id;
        string numStr = lastId.substr(1);
        int num = stoi(numStr);
        num++;
        stringstream ss;
        ss << "D" << setw(3) << setfill('0') << num;
        d.id = ss.str();
    }
    d.user.id = d.id;

    cout << "Generated Dentist ID: " << d.id << endl;

    d.user.name     = askNonBlank("Enter name: ");
    d.user.age      = readMenuChoice("Enter age: ", 1, 120);
    d.user.email    = askNonBlank("Enter email: ");
    d.user.password = askNonBlank("Enter password: ");

    dentists.push_back(d);
    saveDentists();
    cout << "Dentist registered successfully.\n";
}

void adminModifyDentist() {
    string id;
    cout << "Enter dentist ID to modify: ";
    cin >> id;

    Dentist* d = findDentistById(id);
    if (d == nullptr) {
        cout << "Syntax Error: Dentist not found.\n";
        return;
    }

    cout << "Current information:\n";
    displayDentistInfo(*d);
    cout << "\nEnter new values (press Enter to keep current):\n";

    string input;
    cin.ignore();

    cout << "Name [" << d->user.name << "]: ";
    getline(cin, input);
    if (!input.empty()) d->user.name = input;

    cout << "Age [" << d->user.age << "]: ";
    getline(cin, input);
    if (!input.empty()) d->user.age = stoi(input);

    cout << "Email [" << d->user.email << "]: ";
    getline(cin, input);
    if (!input.empty()) d->user.email = input;

    cout << "Password [" << d->user.password << "]: ";
    getline(cin, input);
    if (!input.empty()) d->user.password = input;

    saveDentists();
    cout << "Dentist information updated.\n";
}

void adminPanel() {
    while (true) {
        cout << "\n--- Admin Panel ---\n";
        cout << "1. Register dentist\n";
        cout << "2. Modify dentist information\n";
        cout << "3. Manage all appointments\n";
        cout << "0. Log out\n";

        int choice = readMenuChoice("Choose: ", 0, 3);

        switch (choice) {
            case 1:
                adminRegisterDentist();
                break;
            case 2:
                adminModifyDentist();
                break;
            case 3: {

                Session current;
                current.userId   = "ADM001";
                current.name     = "Administrator";
                current.password = adminPassword;
                current.role     = ADMIN;
                appointmentMenu(current);
                break;
            }
            case 0:
                cout << "Logging out from admin.\n";
                return;
        }
    }
}

void receptionViewAllSchedules() {
    if (dentists.empty()) {
        cout << "\nNo dentists are registered yet.\n"
             << "An admin can add one from the main menu: Login Admin > Register dentist.\n";
        return;
    }

    for (const Dentist& d : dentists) {
        cout << "\n" << d.id << " - "
             << (d.user.name.empty() ? "(no name on record)" : d.user.name) << "\n";

        vector<TimeSlot> ds = getSlotsForDentist(d.id);
        if (ds.empty()) {
            cout << "  No timeslots set. The dentist adds these from"
                 << " Manage my time slots.\n";
        } else {
            displaySlots(ds);
        }
    }
}

void receptionMenu() {
    while (true) {
        cout << "\n--- Reception Menu ---\n";
        cout << "1. View all dentists' schedules\n";
        cout << "2. Manage appointments\n";
        cout << "0. Logout\n";

        int choice = readMenuChoice("Choose: ", 0, 2);

        switch (choice) {
            case 1:
                receptionViewAllSchedules();
                break;
            case 2: {

                Session current;
                current.userId   = "R001";
                current.name     = "Reception";
                current.password = RECEPTION_PASSWORD;
                current.role     = RECEPTIONIST;
                appointmentMenu(current);
                break;
            }
            case 0:
                cout << "Logging out from reception.\n";
                return;
        }
    }
}

void dentistViewSchedule(Dentist* d) {
    vector<TimeSlot> ds = getSlotsForDentist(d->id);

    if (ds.empty()) {
        cout << "\nNo timeslots set yet. Add one from Manage my time slots.\n";
        return;
    }
    cout << "\n";
    displaySlots(ds);
}

void dentistAddSlot(Dentist* d) {
    TimeSlot s;
    s.dentistId = d->id;
    cout << "Enter start time (e.g., 10:00): ";
    cin >> s.start;
    cout << "Enter end time (e.g., 12:00): ";
    cin >> s.end;
    cout << "Available? (1 for yes, 0 for locked): ";
    int avail;
    cin >> avail;
    s.available = (avail == 1);
    addOrUpdateSlot(s);
    saveSlots();
    cout << "Slot added/updated.\n";
}

void dentistRemoveSlot(Dentist* d) {
    string start, end;
    cout << "Enter start time: ";
    cin >> start;
    cout << "Enter end time: ";
    cin >> end;
    if (removeSlot(d->id, start, end)) {
        saveSlots();
        cout << "Slot removed.\n";
    } else {
        cout << "Slot not found.\n";
    }
}

void dentistLockSlot(Dentist* d) {
    string start, end;
    cout << "Enter start time: ";
    cin >> start;
    cout << "Enter end time: ";
    cin >> end;
    for (auto& s : slots) {
        if (s.dentistId == d->id && s.start == start && s.end == end) {
            s.available = false;
            saveSlots();
            cout << "Slot locked (unavailable).\n";
            return;
        }
    }
    cout << "Slot not found. Do you want to create it as locked? (y/n): ";
    char resp;
    cin >> resp;
    if (tolower(resp) == 'y') {
        TimeSlot ns;
        ns.dentistId = d->id;
        ns.start = start;
        ns.end = end;
        ns.available = false;
        slots.push_back(ns);
        saveSlots();
        cout << "Slot created and locked.\n";
    } else {
        cout << "Operation cancelled.\n";
    }
}

void dentistUnlockSlot(Dentist* d) {
    string start, end;
    cout << "Enter start time: ";
    cin >> start;
    cout << "Enter end time: ";
    cin >> end;
    for (auto& s : slots) {
        if (s.dentistId == d->id && s.start == start && s.end == end) {
            s.available = true;
            saveSlots();
            cout << "Slot unlocked (available).\n";
            return;
        }
    }
    cout << "Slot not found.\n";
}

void dentistMenu(Dentist* d) {
    while (true) {
        cout << "\n--- Dentist Menu (" << d->user.name << ") ---\n";
        cout << "1. View my schedule\n";
        cout << "2. Manage my time slots\n";
        cout << "3. My appointments\n";
        cout << "4. Logout\n";
        cout << "Choose: ";
        int choice;
        cin >> choice;
        cin.ignore(1000, '\n');

        if (choice == 1) {
            dentistViewSchedule(d);
        } else if (choice == 2) {
            while (true) {
                cout << "\n--- Manage Slots ---\n";
                cout << "1. View schedule\n";
                cout << "2. Add new slot\n";
                cout << "3. Remove slot\n";
                cout << "4. Lock a slot (set unavailable)\n";
                cout << "5. Unlock a slot (set available)\n";
                cout << "6. Back\n";
                cout << "Choose: ";
                int sub;
                cin >> sub;
                switch (sub) {
                    case 1: dentistViewSchedule(d); break;
                    case 2: dentistAddSlot(d); break;
                    case 3: dentistRemoveSlot(d); break;
                    case 4: dentistLockSlot(d); break;
                    case 5: dentistUnlockSlot(d); break;
                    case 6: goto back;
                    default: cout << "Invalid choice.\n";
                }
            }
            back: ;
        } else if (choice == 3) {

            Session current;
            current.userId   = d->id;
            current.name     = d->user.name;
            current.password = d->user.password;
            current.role     = DENTIST;
            appointmentMenu(current);
        } else if (choice == 4) {
            cout << "Logging out.\n";
            return;
        } else {
            cout << "Invalid choice.\n";
        }
    }
}

void loginDentist() {
    string name, pass, note;

    if (dentists.empty()) {
        cout << "No dentists are registered yet.\n"
             << "An admin can add one from the main menu: Login Admin > Register dentist.\n";
        return;
    }

    cout << "Dentist login. (0 to go back)\n";

    Dentist* d = nullptr;
    while (true) {
        name = trim(askInPlace("Dentist name: ", note));
        if (!cin || name == "0") { clearLine(); cout << "Login cancelled.\n"; return; }

        if (name.empty()) { note = "[cannot be blank] "; continue; }

        d = findDentistByName(name);
        if (d != nullptr) break;
        note = "[no dentist with that name] ";
    }
    acceptInPlace("Dentist name: ", name);

    note.clear();
    while (true) {
        pass = trim(askInPlace("Password: ", note));
        if (!cin || pass == "0") { clearLine(); cout << "Login cancelled.\n"; return; }
        if (pass == d->user.password) break;
        note = "[incorrect password] ";
    }
    acceptInPlace("Password: ", string(pass.length(), '*'));

    cout << "Login successful. Welcome, " << d->user.name << "!\n";
    dentistMenu(d);
}

void loginReception() {
    string username, pass, note;

    cout << "Reception login. (0 to go back)\n";

    while (true) {
        username = trim(askInPlace("Username: ", note));
        if (!cin || username == "0") { clearLine(); cout << "Login cancelled.\n"; return; }
        if (username == RECEPTION_USERNAME) break;
        note = "[no such user] ";
    }
    acceptInPlace("Username: ", username);

    note.clear();
    while (true) {
        pass = trim(askInPlace("Password: ", note));
        if (!cin || pass == "0") { clearLine(); cout << "Login cancelled.\n"; return; }
        if (pass == RECEPTION_PASSWORD) break;
        note = "[incorrect password] ";
    }
    acceptInPlace("Password: ", string(pass.length(), '*'));

    cout << "Reception login successful.\n";
    receptionMenu();
}

void loginAdmin() {
    string name, pass, note;

    cout << "Admin login. (0 to go back)\n";

    while (true) {
        name = trim(askInPlace("Admin name: ", note));
        if (!cin || name == "0") { clearLine(); cout << "Login cancelled.\n"; return; }
        if (name == adminName) break;
        note = "[no such admin] ";
    }
    acceptInPlace("Admin name: ", name);

    note.clear();
    while (true) {
        pass = trim(askInPlace("Admin password: ", note));
        if (!cin || pass == "0") { clearLine(); cout << "Login cancelled.\n"; return; }
        if (pass == adminPassword) break;
        note = "[incorrect password] ";
    }
    acceptInPlace("Admin password: ", string(pass.length(), '*'));

    cout << "Admin login successful.\n";
    adminPanel();
}
