#ifndef CONSOLE_H
#define CONSOLE_H

#include <cctype>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

const bool KEEP_CURSOR_IN_PLACE = true;
const string underline = "\033[4m";
const string reset = "\033[0m";

inline void clearLine() {
    if (KEEP_CURSOR_IN_PLACE) cout << "\r\033[K" << flush;
}

inline void clearScreen() {
    cout << "\033[2J\033[H" << flush;
}

inline void stayOnPromptLine() {
    if (KEEP_CURSOR_IN_PLACE) cout << "\033[A\r\033[K" << flush;
}

inline string askInPlace(const string& label, const string& note) {
    clearLine();
    cout << note << label << flush;
    string line;
    getline(cin, line);
    stayOnPromptLine();
    return line;
}

inline void acceptInPlace(const string& label, const string& answer) {
    clearLine();
    cout << label << answer << endl;
}

inline int toIntOr(const string& text, int fallback) {
    size_t first = text.find_first_not_of(" \t\r\n");
    if (first == string::npos) return fallback;
    size_t last = text.find_last_not_of(" \t\r\n");
    string t = text.substr(first, last - first + 1);

    for (size_t i = 0; i < t.length(); i++) {
        if (i == 0 && (t[i] == '-' || t[i] == '+')) continue;
        if (!isdigit((unsigned char)t[i])) return fallback;
    }

    stringstream parse(t);
    int value = fallback;
    if (!(parse >> value)) return fallback;
    return value;
}

inline void pauseForKey() {
    cout << "\n  Press Enter to continue... " << flush;
    string discard;
    getline(cin, discard);
    stayOnPromptLine();
    clearLine();
}

inline int readMenuChoice(const string& label, int low, int high) {
    string note;
    while (true) {
        string line = askInPlace(label, note);
        if (!cin) { cout << endl; return low; }

        int value = 0;
        stringstream parse(line);
        if ((parse >> value) && value >= low && value <= high) {
            stringstream shown;
            shown << value;
            acceptInPlace(label, shown.str());
            return value;
        }

        stringstream message;
        message << "[enter " << low << "-" << high << "] ";
        note = message.str();
    }
}

#endif
