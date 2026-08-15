#ifndef CONSOLE_H
#define CONSOLE_H

#include <iostream>
#include <sstream>
#include <string>

using namespace std;

const bool KEEP_CURSOR_IN_PLACE = true;

inline void clearLine() {
    if (KEEP_CURSOR_IN_PLACE) cout << "\r\033[K" << flush;
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
