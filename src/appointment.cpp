#include "../headers/Appointment.h"
#include "../headers/Console.h"
#include "../headers/Dentist.h"
#include "../headers/Payment.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <cctype>
#include <ctime>
#include <filesystem>

using namespace std;

const int CLINIC_OPEN_HOUR   = 9;
const int CLINIC_CLOSE_HOUR  = 17;
const int SLOT_DURATION_HRS  = 1;
const int SLOTS_PER_DAY      = (CLINIC_CLOSE_HOUR - CLINIC_OPEN_HOUR) / SLOT_DURATION_HRS;

const int MAX_CHAIRS         = 3;
const int BOOKING_WINDOW     = 14;
const int MAX_PASSWORD_TRIES = 3;

const int EPOCH_YEAR         = 2000;
const int MAX_YEAR           = 2100;

const char   DELIM           = '|';
const string UNASSIGNED      = "UNASSIGNED";

const string APPOINTMENT_FILE = "data/appointments.txt";

const string DAY_NAMES[7] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

const int DAYS_IN_MONTH[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

const string STATUS_LABEL[3] = { "Scheduled", "Completed", "Cancelled" };

static vector<Appointment> appointments;

static string trimField(const string& text);
static vector<string> splitRecord(const string& line, char separator);

static bool   isLeapYear(int year);
static int    daysInMonth(int month, int year);
static bool   isValidDate(Date d);
static int    toDayNumber(Date d);
static int    daysBetween(Date from, Date to);
static int    dayOfWeek(Date d);
static Date   getToday();
static bool   sameDate(Date a, Date b);
static string formatDate(Date d);

static string to12Hour(int hour24);
static string toShortSlot(int slotIndex);
static string formatSlot(int slotIndex);
static void   buildScheduleGrid(int grid[][SLOTS_PER_DAY], const string& excludeID);
static bool   isSlotFull(int grid[][SLOTS_PER_DAY], Date date, int slotIndex);
static bool   isDentistBusy(string dentistID, Date date, int slotIndex);
static int    slotCapacity();
static bool   patientHasSlot(string patientID, Date date, int slotIndex, const string& excludeID);
static int    ageAppointments();

static string statusToString(Status s);
static Status stringToStatus(const string& text);
static string generateAppointmentID();

static void   splitPrompt(const string& prompt, string& prefix, string& label);
static int    readInt(const string& prompt, int low, int high);
static string readText(const string& prompt);
static bool   readDate(const string& prompt, Date& result);
static char   readChoice(const string& prompt, const string& allowed);
static bool   confirmPassword(const Session& current);

static void printModuleHeader(const string& title);
static void displayAvailableSlots(int grid[][SLOTS_PER_DAY], Date date);
static void displayAppointmentDetails(const Appointment& a);
static void displayAppointmentTable(const vector<int>& rows);

static void collectVisible(const Session& current, int statusFilter, bool unassignedOnly, vector<int>& rows);
static int  selectAppointment(const Session& current, int statusFilter, bool unassignedOnly, const string& title);
static int  chooseSlot(int grid[][SLOTS_PER_DAY], Date date);
static string chooseDentist(Date date, int slotIndex);

void scheduleAppointment(const Session& current);
static void editDraftField(int grid[][SLOTS_PER_DAY], Appointment& draft);
void modifyAppointment(const Session& current);
void cancelAppointment(const Session& current);
static void assignDentist(const Session& current);

static void displayScheduleGrid();
void findNextAvailable();

static void appointmentMenuReception(const Session& current);
static void appointmentMenuDentist(const Session& current);

static string lookupPatientName(string patientID);
static string lookupDentistName(string dentistID);
static bool   patientExists(string patientID);
static void   raiseInvoice(string apptID, string patientID);

static string lookupPatientName(string patientID) {
    Patient* p = findPatientByID(patients, patientID);
    if (p == nullptr || p->user.name.empty()) return "[" + patientID + "]";
    return p->user.name;
}

static string lookupDentistName(string dentistID) {
    if (dentistID == UNASSIGNED) return "-";
    Dentist* d = findDentistById(dentistID);
    if (d == NULL || d->user.name.empty()) return "(unknown)";
    return d->user.name;
}

static bool patientExists(string patientID) {
    if (patientID.length() != 4) return false;
    if (toupper(patientID[0]) != 'P') return false;
    for (int i = 1; i < 4; i++) {
        if (!isdigit(patientID[i])) return false;
    }
    return true;
}

static void raiseInvoice(string apptID, string patientID) {
    cout << "  -> Invoice request sent to Billing for " << apptID << " (patient " << patientID << ").\n";
}

// Patient flow: View Appointments -> select a completed appointment -> pay for it
void payForAppointment(const Session& current) {
    int index = selectAppointment(current, COMPLETED, false, "Pay for an Appointment");
    if (index < 0) return;

    const Appointment& target = appointments[index];

    if (hasPaymentForAppointment(target.appointmentID)) {
        cout << "\n  This appointment has already been paid for.\n";
        pauseForKey();
        return;
    }

    displayAppointmentDetails(target);
    processPaymentTransaction(target.appointmentID, target.patientID);
    pauseForKey();
}

// Reception/admin flow: View Appointments -> select a completed appointment -> assign the total payment/invoice
void assignPaymentForAppointment(const Session& current) {
    int index = selectAppointment(current, COMPLETED, false, "Assign Payment / Send Invoice");
    if (index < 0) return;

    const Appointment& target = appointments[index];

    if (hasPaymentForAppointment(target.appointmentID)) {
        cout << "\n  An invoice/payment already exists for this appointment.\n";
        pauseForKey();
        return;
    }

    displayAppointmentDetails(target);
    cout << "\n  Billing patient " << target.patientID << " " << lookupPatientName(target.patientID) << "\n";
    processPaymentTransaction(target.appointmentID, target.patientID);
    pauseForKey();
}

static string trimField(const string& text) {
    size_t first = text.find_first_not_of(" \t\r\n");
    if (first == string::npos) return "";
    size_t last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

static vector<string> splitRecord(const string& line, char separator) {
    vector<string> fields;
    size_t start = 0;

    while (true) {
        size_t position = line.find(separator, start);

        if (position == string::npos) {
            fields.push_back(trimField(line.substr(start)));
            break;
        }

        fields.push_back(trimField(line.substr(start, position - start)));
        start = position + 1;
    }
    return fields;
}

static bool isLeapYear(int year) {
    if (year % 400 == 0) return true;
    else if (year % 100 == 0) return false;
    else if (year % 4 == 0) return true;
    else return false;
}

static int daysInMonth(int month, int year) {
    if (month < 1 || month > 12) return 0;
    if (month == 2 && isLeapYear(year)) return 29;
    return DAYS_IN_MONTH[month - 1];
}

static bool isValidDate(Date d) {
    if (d.year < EPOCH_YEAR || d.year > MAX_YEAR) return false;
    if (d.month < 1 || d.month > 12) return false;
    if (d.day < 1 || d.day > daysInMonth(d.month, d.year)) return false;
    return true;
}

static int toDayNumber(Date d) {
    int days = 0;
    for (int year = EPOCH_YEAR; year < d.year; year++) {
        days += isLeapYear(year) ? 366 : 365;
    }
    for (int month = 1; month < d.month; month++) {
        days += daysInMonth(month, d.year);
    }
    return days + d.day;
}

static int daysBetween(Date from, Date to) {
    return toDayNumber(to) - toDayNumber(from);
}

static int dayOfWeek(Date d) {
    int month = d.month;
    int year  = d.year;
    if (month < 3) {
        month += 12;
        year  -= 1;
    }
    int k = year % 100;
    int j = year / 100;
    int h = (d.day + (13 * (month + 1)) / 5 + k + k / 4 + j / 4 + 5 * j) % 7;
    return (h + 6) % 7;
}

static Date getToday() {
    time_t rawTime = time(0);
    tm* localTime = localtime(&rawTime);
    Date today;
    today.day   = localTime->tm_mday;
    today.month = localTime->tm_mon + 1;
    today.year  = localTime->tm_year + 1900;
    return today;
}

static bool sameDate(Date a, Date b) {
    return a.day == b.day && a.month == b.month && a.year == b.year;
}

static string formatDate(Date d) {
    stringstream ss;
    ss << setw(2) << setfill('0') << d.day << "/"
    << setw(2) << setfill('0') << d.month << "/"
    << d.year;
    return ss.str();
}

static string to12Hour(int hour24) {
    int hour12 = hour24 % 12;
    if (hour12 == 0) hour12 = 12;
    stringstream ss;
    ss << setw(2) << setfill('0') << hour12 << ":00 " << (hour24 < 12 ? "AM" : "PM");
    return ss.str();
}

static string toShortSlot(int slotIndex) {
    int startHour = CLINIC_OPEN_HOUR + slotIndex * SLOT_DURATION_HRS;
    int endHour   = startHour + SLOT_DURATION_HRS;

    int start12 = startHour % 12;
    if (start12 == 0) start12 = 12;
    int end12 = endHour % 12;
    if (end12 == 0) end12 = 12;

    stringstream ss;
    ss << setw(2) << setfill('0') << start12 << "-"
    << setw(2) << setfill('0') << end12 << (endHour < 12 ? "am" : "pm");
    return ss.str();
}

static string formatSlot(int slotIndex) {
    int startHour = CLINIC_OPEN_HOUR + slotIndex * SLOT_DURATION_HRS;
    return to12Hour(startHour) + " - " + to12Hour(startHour + SLOT_DURATION_HRS);
}

static void buildScheduleGrid(int grid[][SLOTS_PER_DAY], const string& excludeID) {
    Date today = getToday();

    for (int day = 0; day < BOOKING_WINDOW; day++) {
        for (int slot = 0; slot < SLOTS_PER_DAY; slot++) {
            grid[day][slot] = 0;
        }
    }

    for (size_t i = 0; i < appointments.size(); i++) {
        const Appointment& a = appointments[i];

        if (a.status != SCHEDULED) continue;
        if (a.appointmentID == excludeID) continue;

        int offset = daysBetween(today, a.date);
        if (offset >= 0 && offset < BOOKING_WINDOW &&
            a.slotIndex >= 0 && a.slotIndex < SLOTS_PER_DAY) {
            grid[offset][a.slotIndex]++;
        }
    }
}

static bool isSlotFull(int grid[][SLOTS_PER_DAY], Date date, int slotIndex) {
    int offset = daysBetween(getToday(), date);
    if (offset < 0 || offset >= BOOKING_WINDOW) return true;
    if (slotIndex < 0 || slotIndex >= SLOTS_PER_DAY) return true;
    return grid[offset][slotIndex] >= slotCapacity();
}

static bool isDentistBusy(string dentistID, Date date, int slotIndex) {
    for (size_t i = 0; i < appointments.size(); i++) {
        const Appointment& a = appointments[i];
        if (a.status == SCHEDULED &&
            a.dentistID == dentistID &&
            a.slotIndex == slotIndex &&
            sameDate(a.date, date)) {
            return true;
        }
    }
    return false;
}

static int slotCapacity() {
    int onStaff = 0;
    for (size_t i = 0; i < dentists.size(); i++) {
        if (dentists[i].user.name.empty()) continue;
        onStaff++;
    }
    return (onStaff > MAX_CHAIRS) ? MAX_CHAIRS : onStaff;
}

static bool patientHasSlot(string patientID, Date date, int slotIndex, const string& excludeID) {
    for (size_t i = 0; i < appointments.size(); i++) {
        const Appointment& a = appointments[i];
        if (a.status == SCHEDULED &&
            a.patientID == patientID &&
            a.slotIndex == slotIndex &&
            a.appointmentID != excludeID &&
            sameDate(a.date, date)) {
            return true;
        }
    }
    return false;
}

static string statusToString(Status s) {
    switch (s) {
        case SCHEDULED: return "SCHEDULED";
        case COMPLETED: return "COMPLETED";
        case CANCELLED: return "CANCELLED";
        default:        return "SCHEDULED";
    }
}

static Status stringToStatus(const string& text) {
    if (text == "COMPLETED")      return COMPLETED;
    else if (text == "CANCELLED") return CANCELLED;
    else                          return SCHEDULED;
}

static string generateAppointmentID() {
    int highest = 0;
    for (size_t i = 0; i < appointments.size(); i++) {
        string suffix = appointments[i].appointmentID.substr(1);
        int number = atoi(suffix.c_str());
        if (number > highest) highest = number;
    }
    stringstream ss;
    ss << "A" << setw(3) << setfill('0') << (highest + 1);
    return ss.str();
}

static int ageAppointments() {
    Date today = getToday();
    int aged = 0;

    for (size_t i = 0; i < appointments.size(); i++) {
        if (appointments[i].status != SCHEDULED) continue;
        if (daysBetween(today, appointments[i].date) < 0) {
            appointments[i].status = COMPLETED;
            aged++;
        }
    }
    return aged;
}

void loadAppointments() {
    appointments.clear();

    ifstream inFile(APPOINTMENT_FILE.c_str());
    if (!inFile.is_open()) return;

    string line;
    while (getline(inFile, line)) {
        if (trimField(line).empty()) continue;

        vector<string> fields = splitRecord(line, DELIM);
        if (fields.size() < 9) continue;

        Appointment a;
        a.appointmentID = fields[0];
        a.patientID     = fields[1];
        a.dentistID     = fields[2];
        a.date.day      = atoi(fields[3].c_str());
        a.date.month    = atoi(fields[4].c_str());
        a.date.year     = atoi(fields[5].c_str());
        a.slotIndex     = atoi(fields[6].c_str());
        a.reason        = fields[7];
        a.status        = stringToStatus(fields[8]);

        if (a.appointmentID.empty()) continue;
        if (!isValidDate(a.date)) continue;
        if (a.slotIndex < 0 || a.slotIndex >= SLOTS_PER_DAY) continue;

        appointments.push_back(a);
    }
    inFile.close();

    if (ageAppointments() > 0) saveAppointments();
}

void saveAppointments() {
    filesystem::create_directories("data");
    ofstream outFile(APPOINTMENT_FILE.c_str());
    if (!outFile.is_open()) {
        cout << "  [!] Could not write to " << APPOINTMENT_FILE << "\n";
        return;
    }

    for (size_t i = 0; i < appointments.size(); i++) {
        const Appointment& a = appointments[i];
        outFile << a.appointmentID << DELIM
                << a.patientID     << DELIM
                << a.dentistID     << DELIM
                << a.date.day      << DELIM
                << a.date.month    << DELIM
                << a.date.year     << DELIM
                << a.slotIndex     << DELIM
                << a.reason        << DELIM
                << statusToString(a.status) << "\n";
    }
    outFile.close();
}

static void splitPrompt(const string& prompt, string& prefix, string& label) {
    size_t lastNewline = prompt.find_last_of('\n');
    if (lastNewline == string::npos) {
        prefix = "";
        label  = prompt;
    } else {
        prefix = prompt.substr(0, lastNewline + 1);
        label  = prompt.substr(lastNewline + 1);
    }
}

static int readInt(const string& prompt, int low, int high) {
    string prefix, label, line, note;
    splitPrompt(prompt, prefix, label);
    cout << prefix;

    while (true) {
        clearLine();
        cout << note << label << flush;
        getline(cin, line);
        stayOnPromptLine();

        if (!cin) { cout << endl; return low; }

        int value = 0;
        stringstream ss(line);
        if ((ss >> value) && value >= low && value <= high) {
            clearLine();
            cout << label << value << endl;
            return value;
        }

        stringstream message;
        message << "[enter " << low << "-" << high << "] ";
        note = message.str();
    }
}

static string readText(const string& prompt) {
    string prefix, label, line, note;
    splitPrompt(prompt, prefix, label);
    cout << prefix;

    while (true) {
        clearLine();
        cout << note << label << flush;
        getline(cin, line);
        stayOnPromptLine();

        if (!cin) { cout << endl; return ""; }
        line = trimField(line);

        if (line.empty()) {
            note = "[cannot be blank] ";
        } else if (line.find(DELIM) != string::npos) {
            note = string("[no '") + DELIM + "' allowed] ";
        } else {
            clearLine();
            cout << label << line << endl;
            return line;
        }
    }
}

static bool readDate(const string& prompt, Date& result) {
    string prefix, label, line, note;
    splitPrompt(prompt, prefix, label);
    cout << prefix;

    while (true) {
        clearLine();
        cout << note << label << flush;
        getline(cin, line);
        stayOnPromptLine();

        if (!cin) { cout << endl; return false; }
        line = trimField(line);

        if (line == "0") {
            clearLine();
            cout << label << "0" << endl;
            return false;
        }

        int day = 0, month = 0, year = 0;
        char firstSlash = ' ', secondSlash = ' ';
        stringstream ss(line);

        if (!(ss >> day >> firstSlash >> month >> secondSlash >> year) ||
            firstSlash != '/' || secondSlash != '/') {
            note = "[use DD/MM/YYYY] ";
            continue;
        }

        Date candidate;
        candidate.day   = day;
        candidate.month = month;
        candidate.year  = year;

        if (!isValidDate(candidate)) {
            note = "[no such date] ";
            continue;
        }

        int offset = daysBetween(getToday(), candidate);
        if (offset < 0) {
            note = "[that date has passed] ";
        } else if (offset >= BOOKING_WINDOW) {
            stringstream message;
            message << "[only " << BOOKING_WINDOW << " days ahead] ";
            note = message.str();
        } else {
            clearLine();
            cout << label << formatDate(candidate) << endl;
            result = candidate;
            return true;
        }
    }
}

static char readChoice(const string& prompt, const string& allowed) {
    string prefix, label, line, note;
    splitPrompt(prompt, prefix, label);
    cout << prefix;

    while (true) {
        clearLine();
        cout << note << label << flush;
        getline(cin, line);
        stayOnPromptLine();

        if (!cin) {

            cout << endl;
            size_t no = allowed.find('N');
            return (no == string::npos) ? allowed[0] : 'N';
        }
        line = trimField(line);

        if (line.length() == 1) {
            char typed = toupper(line[0]);
            if (allowed.find(typed) != string::npos) {
                clearLine();
                cout << label << typed << endl;
                return typed;
            }
        }

        string options;
        for (size_t i = 0; i < allowed.length(); i++) {
            if (i > 0) options += "/";
            options += allowed[i];
        }
        note = "[" + options + " only] ";
    }
}

static bool confirmPassword(const Session& current) {
    string entered, note;

    for (int attempt = 1; attempt <= MAX_PASSWORD_TRIES; attempt++) {
        stringstream label;
        label << "  Enter your password to confirm (" << attempt
            << " of " << MAX_PASSWORD_TRIES << "): ";

        clearLine();
        cout << note << label.str() << flush;
        getline(cin, entered);
        stayOnPromptLine();

        if (!cin) { cout << endl; return false; }

        if (entered == current.password) {
            clearLine();
            cout << "  Password confirmed." << endl;
            return true;
        }
        note = "[incorrect] ";
    }

    clearLine();
    cout << "  [!] Too many failed attempts. No changes were made." << endl;
    return false;
}

static void printModuleHeader(const string& title) {
    cout << "\n" << string(78, '=') << "\n";
    cout << "  " << title << "\n";
    cout << string(78, '=') << "\n";
}

static void displayAvailableSlots(int grid[][SLOTS_PER_DAY], Date date) {
    int offset = daysBetween(getToday(), date);

    cout << "\n  Availability for " << formatDate(date)
        << " (" << DAY_NAMES[dayOfWeek(date)] << ")\n";
    cout << "  " << string(52, '-') << "\n";
    cout << "  " << left << setw(6) << "No." << setw(24) << "Timeslot"
        << setw(22) << "Availability" << "\n";
    cout << "  " << string(52, '-') << "\n";

    for (int i = 0; i < SLOTS_PER_DAY; i++) {
        int booked = 0;
        if (offset >= 0 && offset < BOOKING_WINDOW) booked = grid[offset][i];
        int capacity = slotCapacity();
        int free = capacity - booked;

        stringstream status;
        if (capacity <= 0)  status << "No dentist available";
        else if (free <= 0) status << "Occupied (full)";
        else                status << "Available (" << free << " of " << capacity << ")";

        cout << "  " << left << setw(6) << (i + 1)
            << setw(24) << formatSlot(i)
            << setw(22) << status.str() << "\n";
    }
    cout << "  " << string(52, '-') << "\n";
}

static void displayAppointmentDetails(const Appointment& a) {
    cout << "\n  Appointment Details\n";
    cout << "  " << string(52, '-') << "\n";
    cout << "     Appointment ID : " << a.appointmentID << "\n";
    cout << "     Patient        : " << a.patientID << " " << lookupPatientName(a.patientID) << "\n";
    cout << "  1. Date           : " << formatDate(a.date) << " (" << DAY_NAMES[dayOfWeek(a.date)] << ")\n";
    cout << "  2. Timeslot       : " << formatSlot(a.slotIndex) << "\n";
    cout << "  3. Reason         : " << a.reason << "\n";
    cout << "  4. Dentist        : " << a.dentistID << " " << lookupDentistName(a.dentistID) << "\n";
    cout << "  5. Status         : " << STATUS_LABEL[a.status] << "\n";
    cout << "  " << string(52, '-') << "\n";
}

static void displayAppointmentTable(const vector<int>& rows) {
    cout << "\n  " << left
        << setw(5)  << "No."
        << setw(9)  << "ApptID"
        << setw(13) << "Date"
        << setw(21) << "Timeslot"
        << setw(9)  << "Patient"
        << setw(13) << "Dentist"
        << setw(11) << "Status" << "\n";
    cout << "  " << string(76, '-') << "\n";

    for (size_t i = 0; i < rows.size(); i++) {
        const Appointment& a = appointments[rows[i]];
        cout << "  " << left
            << setw(5)  << (i + 1)
            << setw(9)  << a.appointmentID
            << setw(13) << formatDate(a.date)
            << setw(21) << formatSlot(a.slotIndex)
            << setw(9)  << a.patientID
            << setw(13) << a.dentistID
            << setw(11) << STATUS_LABEL[a.status] << "\n";
    }
    cout << "  " << string(76, '-') << "\n";
    cout << "  " << rows.size() << " record(s) shown.\n";
}

static void collectVisible(const Session& current, int statusFilter, bool unassignedOnly, vector<int>& rows) {
    rows.clear();

    for (size_t i = 0; i < appointments.size(); i++) {
        const Appointment& a = appointments[i];

        if (current.role == PATIENT) {
            if (a.patientID != current.userId) continue;
        } else if (current.role == DENTIST) {
            if (a.dentistID != current.userId) continue;
        }

        if (statusFilter >= 0 && (int)a.status != statusFilter) continue;
        if (unassignedOnly && a.dentistID != UNASSIGNED) continue;

        rows.push_back((int)i);
    }
}

static int selectAppointment(const Session& current, int statusFilter, bool unassignedOnly, const string& title) {
    vector<int> rows;
    collectVisible(current, statusFilter, unassignedOnly, rows);

    if (rows.empty()) {
        cout << "\n  There are no appointments available for this action.\n";
        return -1;
    }

    printModuleHeader(title);
    displayAppointmentTable(rows);

    string note;
    while (true) {
        string typed = trimField(askInPlace("\n  Select by No. or Appointment ID (0 to cancel): ", note));
        if (!cin) { cout << endl; return -1; }
        if (typed == "0") { clearLine(); cout << "  Cancelled.\n"; return -1; }

        for (size_t i = 0; i < typed.length(); i++) typed[i] = toupper(typed[i]);

        for (size_t i = 0; i < rows.size(); i++) {
            if (appointments[rows[i]].appointmentID == typed) {
                clearLine();
                cout << "  Selected " << typed << "\n";
                return rows[i];
            }
        }

        int number = 0;
        stringstream parse(typed);
        if ((parse >> number) && number >= 1 && number <= (int)rows.size()) {
            clearLine();
            cout << "  Selected " << appointments[rows[number - 1]].appointmentID << "\n";
            return rows[number - 1];
        }

        note = "[no such row or appointment ID] ";
    }
}

static int chooseSlot(int grid[][SLOTS_PER_DAY], Date date) {
    while (true) {
        int choice = readInt("  Choose a timeslot number (0 to cancel): ", 0, SLOTS_PER_DAY);
        if (choice == 0) return -1;

        int slotIndex = choice - 1;
        if (isSlotFull(grid, date, slotIndex)) {
            if (slotCapacity() <= 0) {
                cout << "  [!] No dentist is on duty for " << formatSlot(slotIndex) << ". Please pick another hour.\n";
            } else {
                cout << "  [!] " << formatSlot(slotIndex) << " is fully booked. Please pick another hour.\n";
            }
            continue;
        }
        return slotIndex;
    }
}

static string chooseDentist(Date date, int slotIndex) {
    vector<string> ids;
    for (size_t i = 0; i < dentists.size(); i++) {
        if (dentists[i].user.name.empty()) continue;
        ids.push_back(dentists[i].id);
    }

    if (ids.empty()) {
        cout << "\n  [!] No dentists are registered. An admin adds them from Login Admin.\n";
        return "";
    }

    while (true) {
        cout << "\n  Dentists\n";
        cout << "  " << string(52, '-') << "\n";
        for (size_t i = 0; i < ids.size(); i++) {
            string note;
            if (isDentistBusy(ids[i], date, slotIndex)) note = "   [busy this hour]";

            cout << "  " << (i + 1) << ". " << ids[i] << " "
                << lookupDentistName(ids[i]) << note << "\n";
        }
        cout << "  " << string(52, '-') << "\n";

        int choice = readInt("  Choose a dentist (0 to cancel): ", 0, (int)ids.size());
        if (choice == 0) return "";

        string chosen = ids[choice - 1];

        if (isDentistBusy(chosen, date, slotIndex)) {
            cout << "  [!] " << lookupDentistName(chosen) << " already has an appointment at "
                << formatSlot(slotIndex) << " on " << formatDate(date) << ".\n";
            continue;
        }
        return chosen;
    }
}

static void editDraftField(int grid[][SLOTS_PER_DAY], Appointment& draft) {
    int field = readInt("  Change which field? 1=Date  2=Timeslot  3=Reason  (0 to cancel): ", 0, 3);

    switch (field) {
        case 1: {
            Date newDate;
            if (!readDate("  New date DD/MM/YYYY (0 to cancel): ", newDate)) return;
            buildScheduleGrid(grid, draft.appointmentID);
            displayAvailableSlots(grid, newDate);
            int newSlot;
            while (true) {
                newSlot = chooseSlot(grid, newDate);
                if (newSlot < 0) return;
                if (!patientHasSlot(draft.patientID, newDate, newSlot, draft.appointmentID)) break;
                cout << "  [!] " << draft.patientID << " already has an appointment at "
                    << formatSlot(newSlot) << " on " << formatDate(newDate) << ".\n";
            }
            draft.date      = newDate;
            draft.slotIndex = newSlot;
            break;
        }
        case 2: {
            buildScheduleGrid(grid, draft.appointmentID);
            displayAvailableSlots(grid, draft.date);
            int newSlot;
            while (true) {
                newSlot = chooseSlot(grid, draft.date);
                if (newSlot < 0) return;
                if (!patientHasSlot(draft.patientID, draft.date, newSlot, draft.appointmentID)) break;
                cout << "  [!] " << draft.patientID << " already has an appointment at "
                    << formatSlot(newSlot) << " on " << formatDate(draft.date) << ".\n";
            }
            draft.slotIndex = newSlot;
            break;
        }
        case 3:
            draft.reason = readText("  New reason for visit: ");
            break;
        default:
            break;
    }
}

void scheduleAppointment(const Session& current) {
    printModuleHeader("Schedule an Appointment");

    string patientID;
    if (current.role == PATIENT) {
        patientID = current.userId;
        cout << "\n  Booking for " << patientID << " " << lookupPatientName(patientID) << "\n";
    } else {
        while (true) {
            patientID = readText("\n  Patient ID (e.g. P001, or 0 to cancel): ");
            if (patientID == "0") return;
            if (patientExists(patientID)) break;
            cout << "  [!] No patient found with that ID.\n";
        }
    }

    Date date;
    if (!readDate("  Appointment date DD/MM/YYYY (0 to cancel): ", date)) return;

    int grid[BOOKING_WINDOW][SLOTS_PER_DAY];
    buildScheduleGrid(grid, "");
    displayAvailableSlots(grid, date);

    int slotIndex;
    while (true) {
        slotIndex = chooseSlot(grid, date);
        if (slotIndex < 0) return;
        if (!patientHasSlot(patientID, date, slotIndex, "")) break;
        cout << "  [!] " << patientID << " already has an appointment at "
            << formatSlot(slotIndex) << " on " << formatDate(date) << ".\n";
    }

    string reason = readText("  Reason for visit: ");

    Appointment draft;
    draft.appointmentID = generateAppointmentID();
    draft.patientID     = patientID;
    draft.dentistID     = UNASSIGNED;
    draft.date          = date;
    draft.slotIndex     = slotIndex;
    draft.reason        = reason;
    draft.status        = SCHEDULED;

    bool finished = false;
    while (!finished) {
        displayAppointmentDetails(draft);
        char answer = readChoice("  Y = confirm, N = discard, M = modify: ", "YNM");

        switch (answer) {
            case 'Y':

                buildScheduleGrid(grid, draft.appointmentID);
                if (patientHasSlot(draft.patientID, draft.date, draft.slotIndex, draft.appointmentID)) {
                    cout << "  [!] " << draft.patientID
                        << " was booked into that hour while you were deciding.\n";
                    displayAvailableSlots(grid, draft.date);
                    int retry = chooseSlot(grid, draft.date);
                    if (retry < 0) { finished = true; break; }
                    draft.slotIndex = retry;
                    break;
                }
                if (isSlotFull(grid, draft.date, draft.slotIndex)) {
                    cout << "  [!] That hour filled up while you were deciding.\n";
                    displayAvailableSlots(grid, draft.date);
                    int retry = chooseSlot(grid, draft.date);
                    if (retry < 0) { finished = true; break; }
                    draft.slotIndex = retry;
                    break;
                }
                appointments.push_back(draft);
                saveAppointments();
                cout << "\n  Appointment " << draft.appointmentID << " confirmed for " << formatDate(draft.date) << ", " << formatSlot(draft.slotIndex) << ".\n";
                finished = true;
                break;

            case 'N':
                cout << "\n  Booking discarded. Nothing was saved.\n";
                finished = true;
                break;

            case 'M':
                editDraftField(grid, draft);
                break;
        }
    }
}

void viewAppointments(const Session& current) {
    printModuleHeader("View Appointments");

    cout << "\n  1. All\n  2. Scheduled only\n  3. Completed only\n  4. Cancelled only\n";
    cout << "  0. Back\n";

    int filterChoice = readInt("\n  Show which records? ", 0, 4);
    if (filterChoice == 0) return;

    int statusFilter;
    switch (filterChoice) {
        case 2:  statusFilter = SCHEDULED; break;
        case 3:  statusFilter = COMPLETED; break;
        case 4:  statusFilter = CANCELLED; break;
        default: statusFilter = -1;        break;
    }

    vector<int> rows;
    collectVisible(current, statusFilter, false, rows);

    if (rows.empty()) {
        cout << "\n  No appointments match that filter.\n";
        pauseForKey();
        return;
    }
    displayAppointmentTable(rows);
    pauseForKey();
}

void modifyAppointment(const Session& current) {
    int index = selectAppointment(current, SCHEDULED, false, "Modify an Appointment");
    if (index < 0) return;

    Appointment& target = appointments[index];
    displayAppointmentDetails(target);

    bool isStaff = (current.role == RECEPTIONIST || current.role == ADMIN);

    cout << "\n  Editable fields\n";
    cout << "  1. Date\n  2. Timeslot\n  3. Reason\n";
    if (isStaff) {
        cout << "  4. Assigned dentist\n  5. Status\n";
    }
    cout << "  (Appointment ID and Patient ID are locked and cannot be changed.)\n";

    int highestField = isStaff ? 5 : 3;
    int field = readInt("\n  Field to change (0 to cancel): ", 0, highestField);
    if (field == 0) return;

    int    grid[BOOKING_WINDOW][SLOTS_PER_DAY];
    Date   newDate    = target.date;
    int    newSlot    = target.slotIndex;
    string newReason  = target.reason;
    string newDentist = target.dentistID;
    Status newStatus  = target.status;

    switch (field) {
        case 1:
            if (!readDate("  New date DD/MM/YYYY (0 to cancel): ", newDate)) return;
            buildScheduleGrid(grid, target.appointmentID);
            displayAvailableSlots(grid, newDate);
            while (true) {
                newSlot = chooseSlot(grid, newDate);
                if (newSlot < 0) return;
                if (!patientHasSlot(target.patientID, newDate, newSlot, target.appointmentID)) break;
                cout << "  [!] " << target.patientID << " already has an appointment at "
                    << formatSlot(newSlot) << " on " << formatDate(newDate) << ".\n";
            }
            break;

        case 2:
            buildScheduleGrid(grid, target.appointmentID);
            displayAvailableSlots(grid, newDate);
            while (true) {
                newSlot = chooseSlot(grid, newDate);
                if (newSlot < 0) return;
                if (!patientHasSlot(target.patientID, newDate, newSlot, target.appointmentID)) break;
                cout << "  [!] " << target.patientID << " already has an appointment at "
                    << formatSlot(newSlot) << " on " << formatDate(newDate) << ".\n";
            }
            break;

        case 3:
            newReason = readText("  New reason for visit: ");
            break;

        case 4:
            newDentist = chooseDentist(newDate, newSlot);
            if (newDentist.empty()) return;
            break;

        case 5: {
            cout << "\n  1. " << STATUS_LABEL[SCHEDULED] << "\n  2. " << STATUS_LABEL[COMPLETED] << "\n  3. " << STATUS_LABEL[CANCELLED] << "\n";
            int pick = readInt("  New status (0 to cancel): ", 0, 3);
            if (pick == 0) return;
            if (pick == 1)      newStatus = SCHEDULED;
            else if (pick == 2) newStatus = COMPLETED;
            else                newStatus = CANCELLED;
            break;
        }
    }

    if (!confirmPassword(current)) return;

    target.date      = newDate;
    target.slotIndex = newSlot;
    target.reason    = newReason;
    target.dentistID = newDentist;
    target.status    = newStatus;

    saveAppointments();
    cout << "\n  Appointment " << target.appointmentID << " updated.\n";

    if (field == 5 && newStatus == COMPLETED) {
        raiseInvoice(target.appointmentID, target.patientID);
    }
}

void cancelAppointment(const Session& current) {
    int index = selectAppointment(current, SCHEDULED, false, "Cancel an Appointment");
    if (index < 0) return;

    displayAppointmentDetails(appointments[index]);

    char answer = readChoice("\n  Cancel this appointment? (Y/N): ", "YN");
    if (answer == 'N') {
        cout << "\n  No changes made.\n";
        return;
    }

    if (!confirmPassword(current)) return;

    appointments[index].status = CANCELLED;
    saveAppointments();

    cout << "\n  Appointment " << appointments[index].appointmentID << " cancelled.\n";
}

static void assignDentist(const Session& current) {
    int index = selectAppointment(current, SCHEDULED, true, "Assign a Dentist");
    if (index < 0) return;

    Appointment& target = appointments[index];
    displayAppointmentDetails(target);

    string chosen = chooseDentist(target.date, target.slotIndex);
    if (chosen.empty()) return;

    if (!confirmPassword(current)) return;

    target.dentistID = chosen;
    saveAppointments();

    cout << "\n  " << chosen << " assigned to " << target.appointmentID << ".\n";
}

static void displayScheduleGrid() {
    printModuleHeader("Two-Week Schedule Grid");

    int grid[BOOKING_WINDOW][SLOTS_PER_DAY];
    buildScheduleGrid(grid, "");

    Date today = getToday();

    const int DATE_WIDTH = 16;
    const int SLOT_WIDTH = 8;
    const int TABLE_WIDTH = DATE_WIDTH + SLOT_WIDTH * SLOTS_PER_DAY;

    cout << "\n  Free appointments per hour (out of " << slotCapacity() << ")\n\n";
    cout << "  " << left << setw(DATE_WIDTH) << "Date";
    for (int slot = 0; slot < SLOTS_PER_DAY; slot++) {
        cout << setw(SLOT_WIDTH) << toShortSlot(slot);
    }
    cout << "\n  " << string(TABLE_WIDTH, '-') << "\n";

    for (int day = 0; day < BOOKING_WINDOW; day++) {
        Date d = today;
        d.day += day;
        while (d.day > daysInMonth(d.month, d.year)) {
            d.day -= daysInMonth(d.month, d.year);
            d.month++;
            if (d.month > 12) { d.month = 1; d.year++; }
        }

        stringstream label;
        label << formatDate(d) << " " << DAY_NAMES[dayOfWeek(d)].substr(0, 3);
        cout << "  " << left << setw(DATE_WIDTH) << label.str();

        for (int slot = 0; slot < SLOTS_PER_DAY; slot++) {
            int capacity = slotCapacity();
            int free = capacity - grid[day][slot];
            string cell;
            if (capacity <= 0) {
                cell = "closed";
            } else if (free <= 0) {
                cell = "FULL";
            } else {
                stringstream cellText;
                cellText << free << " free";
                cell = cellText.str();
            }
            cout << setw(SLOT_WIDTH) << cell;
        }
        cout << "\n";
    }
    cout << "  " << string(TABLE_WIDTH, '-') << "\n";
    pauseForKey();
}

void findNextAvailable() {
    printModuleHeader("Find the Next Available Slot");

    Date start;
    if (!readDate("\n  Search from which date? DD/MM/YYYY (0 to cancel): ", start)) return;

    int grid[BOOKING_WINDOW][SLOTS_PER_DAY];
    buildScheduleGrid(grid, "");
    Date today = getToday();
    int startOffset = daysBetween(today, start);
    int found = 0;
    const int WANTED = 5;

    cout << "\n  Earliest openings from " << formatDate(start) << "\n";
    cout << "  " << string(66, '-') << "\n";
    cout << "  " << left << setw(5) << "No." << setw(14) << "Date"
        << setw(12) << "Day" << setw(23) << "Timeslot" << "Availability\n";
    cout << "  " << string(66, '-') << "\n";

    for (int day = startOffset; day < BOOKING_WINDOW && found < WANTED; day++) {
        for (int slot = 0; slot < SLOTS_PER_DAY && found < WANTED; slot++) {
            if (grid[day][slot] < slotCapacity()) {
                Date d = today;
                d.day += day;
                while (d.day > daysInMonth(d.month, d.year)) {
                    d.day -= daysInMonth(d.month, d.year);
                    d.month++;
                    if (d.month > 12) { d.month = 1; d.year++; }
                }
                found++;
                stringstream freeText;
                freeText << (slotCapacity() - grid[day][slot]) << " free";
                cout << "  " << left << setw(5) << found
                    << setw(14) << formatDate(d)
                    << setw(12) << DAY_NAMES[dayOfWeek(d)]
                    << setw(23) << formatSlot(slot)
                    << freeText.str() << "\n";
            }
        }
    }

    if (found == 0) cout << "  No openings left inside the booking window.\n";
    cout << "  " << string(66, '-') << "\n";
    pauseForKey();
}

static void appointmentMenuReception(const Session& current) {
    int choice;
    do {
        printModuleHeader("Appointments - Reception (" + current.name + ")");
        cout << "\n  1. Schedule an appointment for a patient\n";
        cout << "  2. View all appointments\n";
        cout << "  3. Modify an appointment\n";
        cout << "  4. Cancel an appointment\n";
        cout << "  5. Assign a dentist\n";
        cout << "  6. Two-week schedule grid\n";
        cout << "  7. Find the next available slot\n";
        cout << "  8. Assign payment / send invoice\n";
        cout << "  0. Back\n";

        choice = readInt("\n  Choice: ", 0, 8);

        switch (choice) {
            case 1: scheduleAppointment(current); break;
            case 2: viewAppointments(current);    break;
            case 3: modifyAppointment(current);   break;
            case 4: cancelAppointment(current);   break;
            case 5: assignDentist(current);       break;
            case 6: displayScheduleGrid();        break;
            case 7: findNextAvailable();          break;
            case 8: assignPaymentForAppointment(current); break;
            case 0: break;
        }
    } while (choice != 0);
}

static void appointmentMenuDentist(const Session& current) {
    int choice;
    do {
        printModuleHeader("My Schedule - " + current.name);
        cout << "\n  1. View my appointments\n";
        cout << "  2. Two-week schedule grid\n";
        cout << "  0. Back\n";

        choice = readInt("\n  Choice: ", 0, 2);

        switch (choice) {
            case 1: viewAppointments(current); break;
            case 2: displayScheduleGrid();     break;
            case 0: break;
        }
    } while (choice != 0);
}

void appointmentMenu(const Session& current) {
    switch (current.role) {
        case DENTIST:
            appointmentMenuDentist(current);
            break;
        case RECEPTIONIST:
        case ADMIN:
            appointmentMenuReception(current);
            break;
        default:
            cout << "  [!] Patients reach these actions from their own menu.\n";
    }
}

bool hasActiveAppointment(string patientID) {
    for (size_t i = 0; i < appointments.size(); i++) {
        if (appointments[i].patientID == patientID &&
            appointments[i].status == SCHEDULED) {
            return true;
        }
    }
    return false;
}

int countAppointmentsForDentist(string dentistID) {
    int total = 0;
    for (size_t i = 0; i < appointments.size(); i++) {
        if (appointments[i].dentistID == dentistID &&
            appointments[i].status == SCHEDULED) {
            total++;
        }
    }
    return total;
}

bool getAppointmentInfo(string apptID, string& patientIDOut, string& dateOut) {
    for (size_t i = 0; i < appointments.size(); i++) {
        if (appointments[i].appointmentID == apptID) {
            patientIDOut = appointments[i].patientID;
            dateOut      = formatDate(appointments[i].date);
            return true;
        }
    }
    return false;
}
