🔔 Habit Tracker (Reminders) — Multi‑Language Habit Reminder System
8 languages, one reliable habit reminder – keep track of your daily habits and get notified when it's time to take action – right from your terminal.

✨ Features
➕ Add habits – create habits with a name and frequency (in days)

📋 List habits – view all habits with their last completion date and next due date

🔔 Check due habits – see which habits are due today and mark them as done

⏱️ Watch mode – automatically check for due habits every N minutes (configurable)

💾 Persistent storage – all data saved in habits.json

🖥️ Cross‑platform – works on Windows, macOS, and Linux

🧰 Supported Languages & Files
Language	File	Dependencies
Python	habit_reminder.py	none (stdlib)
Go	habit_reminder.go	none (stdlib)
JavaScript (Node)	habit_reminder.js	commander (optional)
Ruby	habit_reminder.rb	json, date
PHP	habit_reminder.php	none (extensions)
Java	HabitReminder.java	Java 8+
C#	HabitReminder.cs	.NET Core 3.1+
C++	habit_reminder.cpp	nlohmann/json
🚀 Quick Start
All implementations follow the same CLI pattern:

bash
# Add a daily habit
<command> add "Exercise" --frequency 1

# Add a weekly habit
<command> add "Read a book" --frequency 7

# List all habits
<command> list

# Check for due habits (and optionally mark them as done)
<command> check

# Watch for due habits every 5 minutes
<command> watch --interval 5
Commands:

add <name> [--frequency N] – add a habit (frequency in days, default: 1)

list – show all habits with due status

check – show due habits and mark them as done

remove <id> – remove a habit by its ID

watch [--interval MINUTES] – continuously check for due habits

📸 Example Output
text
📋 Your Habits:
1. Exercise (daily) – last done: 2026-08-23, next: 2026-08-24 (due tomorrow)
2. Read a book (weekly) – last done: 2026-08-20, next: 2026-08-27 (due in 3 days)
3. Drink water (daily) – last done: 2026-08-24, next: 2026-08-25 (due today!) 🔔

🔔 Due habits:
3. Drink water (daily) – due today! Mark as done? (y/n): y
✅ Habit "Drink water" marked as done.
📁 Repository Structure
text
.
├── README.md
├── python/
│   └── habit_reminder.py
├── go/
│   └── habit_reminder.go
├── javascript/
│   └── habit_reminder.js
├── ruby/
│   └── habit_reminder.rb
├── php/
│   └── habit_reminder.php
├── java/
│   └── HabitReminder.java
├── csharp/
│   └── HabitReminder.cs
└── cpp/
    └── habit_reminder.cpp
