// habit_reminder.cpp
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <iomanip>
#include <random>
#include <nlohmann/json.hpp>
#include <getopt.h>

using namespace std;
using json = nlohmann::json;

string generateId() {
    const char* hex = "0123456789abcdef";
    string id;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, 15);
    for (int i=0; i<8; i++) id += hex[dis(gen)];
    return id;
}

string currentDate() {
    time_t t = time(nullptr);
    char buf[11];
    strftime(buf, sizeof(buf), "%Y-%m-%d", localtime(&t));
    return string(buf);
}

struct Habit {
    string id;
    string name;
    int frequencyDays;
    string lastDone;

    Habit() : frequencyDays(1) {}
    Habit(const string& n, int freq) : id(generateId()), name(n), frequencyDays(freq) {}

    string nextDueDate() const {
        if (lastDone.empty()) return currentDate();
        struct tm tm = {};
        strptime(lastDone.c_str(), "%Y-%m-%d", &tm);
        time_t t = mktime(&tm);
        t += frequencyDays * 24 * 3600;
        tm = *localtime(&t);
        char buf[11];
        strftime(buf, sizeof(buf), "%Y-%m-%d", &tm);
        return string(buf);
    }

    bool isDue() const {
        return nextDueDate() <= currentDate();
    }

    void markDone() {
        lastDone = currentDate();
    }
};

class Tracker {
private:
    vector<Habit> habits;
    string dataFile = "habits.json";

    void load() {
        ifstream f(dataFile);
        if (!f.is_open()) return;
        json j;
        f >> j;
        for (auto& item : j) {
            Habit h;
            h.id = item["id"];
            h.name = item["name"];
            h.frequencyDays = item["frequency_days"];
            h.lastDone = item["last_done"];
            habits.push_back(h);
        }
    }

    void save() {
        json j = json::array();
        for (auto& h : habits) {
            j.push_back({
                {"id", h.id},
                {"name", h.name},
                {"frequency_days", h.frequencyDays},
                {"last_done", h.lastDone}
            });
        }
        ofstream f(dataFile);
        f << setw(2) << j << endl;
    }

public:
    Tracker() { load(); }

    void add(const string& name, int freq) {
        Habit h(name, freq);
        habits.push_back(h);
        save();
        cout << "✅ Habit added: " << h.name << " (ID: " << h.id << ", frequency: " << freq << " days)\n";
    }

    void list() {
        if (habits.empty()) {
            cout << "No habits.\n";
            return;
        }
        cout << "\n📋 Your Habits:\n";
        for (auto& h : habits) {
            string due = h.nextDueDate();
            struct tm tmDue = {}, tmNow = {};
            strptime(due.c_str(), "%Y-%m-%d", &tmDue);
            strptime(currentDate().c_str(), "%Y-%m-%d", &tmNow);
            int daysUntil = (int)difftime(mktime(&tmDue), mktime(&tmNow)) / (24*3600);
            string status;
            if (daysUntil < 0) status = "🔔 DUE!";
            else if (daysUntil == 0) status = "⏰ due today";
            else if (daysUntil == 1) status = "due tomorrow";
            else status = "due in " + to_string(daysUntil) + " days";
            string last = h.lastDone.empty() ? "never" : h.lastDone;
            cout << "  " << h.id << ": " << h.name << " (every " << h.frequencyDays << " day" << (h.frequencyDays>1 ? "s" : "") << ") – last: " << last << ", next: " << due << " (" << status << ")\n";
        }
    }

    void check() {
        vector<Habit*> due;
        for (auto& h : habits) if (h.isDue()) due.push_back(&h);
        if (due.empty()) {
            cout << "✅ No habits due today. Well done!\n";
            return;
        }
        cout << "\n🔔 Due habits:\n";
        for (auto* h : due) {
            cout << "  " << h->id << ": " << h->name << " (frequency: " << h->frequencyDays << " days)\n";
            cout << "  Mark '" << h->name << "' as done? (y/n): ";
            string ans;
            getline(cin, ans);
            if (ans == "y" || ans == "Y") {
                h->markDone();
                cout << "  ✅ Habit '" << h->name << "' marked as done.\n";
            } else {
                cout << "  ⏳ Habit '" << h->name << "' skipped.\n";
            }
        }
        save();
    }

    void remove(const string& id) {
        for (auto it = habits.begin(); it != habits.end(); ++it) {
            if (it->id == id) {
                string name = it->name;
                habits.erase(it);
                save();
                cout << "✅ Habit '" << name << "' removed.\n";
                return;
            }
        }
        cout << "❌ Habit with ID " << id << " not found.\n";
    }

    void watch(int interval) {
        cout << "🔔 Habit Reminder – checking every " << interval << " minute(s). Press Ctrl+C to stop.\n";
        while (true) {
            vector<Habit*> due;
            for (auto& h : habits) if (h.isDue()) due.push_back(&h);
            if (!due.empty()) {
                time_t t = time(nullptr);
                tm* now = localtime(&t);
                char buf[6];
                strftime(buf, sizeof(buf), "%H:%M", now);
                cout << "\n🔔 Due habits at " << buf << ":\n";
                for (auto* h : due) cout << "  " << h->id << ": " << h->name << "\n";
            } else {
                cout << ".";
                cout.flush();
            }
            this_thread::sleep_for(chrono::minutes(interval));
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: habit_reminder <command> [options]\n";
        return 1;
    }
    Tracker tracker;
    string cmd = argv[1];

    if (cmd == "add") {
        if (argc < 3) { cerr << "add <name> [--frequency N]\n"; return 1; }
        string name = argv[2];
        int freq = 1;
        for (int i=3; i<argc; i++) {
            if (string(argv[i]) == "--frequency" && i+1 < argc) {
                freq = stoi(argv[++i]);
            }
        }
        tracker.add(name, freq);
    } else if (cmd == "list") {
        tracker.list();
    } else if (cmd == "check") {
        tracker.check();
    } else if (cmd == "remove") {
        if (argc < 3) { cerr << "remove <id>\n"; return 1; }
        tracker.remove(argv[2]);
    } else if (cmd == "watch") {
        int interval = 5;
        for (int i=2; i<argc; i++) {
            if (string(argv[i]) == "--interval" && i+1 < argc) {
                interval = stoi(argv[++i]);
            }
        }
        tracker.watch(interval);
    } else {
        cerr << "Unknown command. Use add, list, check, remove, watch.\n";
        return 1;
    }
    return 0;
}
