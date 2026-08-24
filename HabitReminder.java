// HabitReminder.java
import java.io.*;
import java.nio.file.*;
import java.time.*;
import java.time.format.*;
import java.util.*;
import com.google.gson.*;

class Habit {
    String id;
    String name;
    int frequencyDays;
    String lastDone;

    Habit() {}
    Habit(String name, int frequencyDays) {
        this.id = UUID.randomUUID().toString().substring(0,8);
        this.name = name;
        this.frequencyDays = frequencyDays;
        this.lastDone = null;
    }

    LocalDate nextDueDate() {
        if (lastDone == null) return LocalDate.now();
        LocalDate last = LocalDate.parse(lastDone);
        return last.plusDays(frequencyDays);
    }

    boolean isDue() {
        return nextDueDate().compareTo(LocalDate.now()) <= 0;
    }

    void markDone() {
        lastDone = LocalDate.now().toString();
    }
}

class Tracker {
    List<Habit> habits = new ArrayList<>();
    final String dataFile = "habits.json";
    final Gson gson = new GsonBuilder().setPrettyPrinting().create();

    public Tracker() { load(); }

    void load() {
        try {
            Path path = Paths.get(dataFile);
            if (Files.exists(path)) {
                String json = new String(Files.readAllBytes(path));
                Habit[] arr = gson.fromJson(json, Habit[].class);
                habits = Arrays.asList(arr);
            }
        } catch (Exception e) {}
    }

    void save() {
        try {
            Files.write(Paths.get(dataFile), gson.toJson(habits).getBytes());
        } catch (Exception e) {}
    }

    void add(String name, int freq) {
        Habit h = new Habit(name, freq);
        habits.add(h);
        save();
        System.out.printf("✅ Habit added: %s (ID: %s, frequency: %d days)%n", name, h.id, freq);
    }

    void list() {
        if (habits.isEmpty()) {
            System.out.println("No habits.");
            return;
        }
        System.out.println("\n📋 Your Habits:");
        for (Habit h : habits) {
            LocalDate due = h.nextDueDate();
            long daysUntil = LocalDate.now().until(due).getDays();
            String status;
            if (daysUntil < 0) status = "🔔 DUE!";
            else if (daysUntil == 0) status = "⏰ due today";
            else if (daysUntil == 1) status = "due tomorrow";
            else status = "due in " + daysUntil + " days";
            String last = h.lastDone != null ? h.lastDone : "never";
            System.out.printf("  %s: %s (every %d day%s) – last: %s, next: %s (%s)%n",
                h.id, h.name, h.frequencyDays, h.frequencyDays>1 ? "s" : "",
                last, due, status);
        }
    }

    void check() throws IOException {
        List<Habit> due = new ArrayList<>();
        for (Habit h : habits) if (h.isDue()) due.add(h);
        if (due.isEmpty()) {
            System.out.println("✅ No habits due today. Well done!");
            return;
        }
        System.out.println("\n🔔 Due habits:");
        BufferedReader reader = new BufferedReader(new InputStreamReader(System.in));
        for (Habit h : due) {
            System.out.printf("  %s: %s (frequency: %d days)%n", h.id, h.name, h.frequencyDays);
            System.out.printf("  Mark '%s' as done? (y/n): ", h.name);
            String ans = reader.readLine().trim().toLowerCase();
            if (ans.equals("y")) {
                h.markDone();
                System.out.printf("  ✅ Habit '%s' marked as done.%n", h.name);
            } else {
                System.out.printf("  ⏳ Habit '%s' skipped.%n", h.name);
            }
        }
        save();
    }

    void remove(String id) {
        Iterator<Habit> it = habits.iterator();
        while (it.hasNext()) {
            Habit h = it.next();
            if (h.id.equals(id)) {
                it.remove();
                save();
                System.out.printf("✅ Habit '%s' removed.%n", h.name);
                return;
            }
        }
        System.out.printf("❌ Habit with ID %s not found.%n", id);
    }

    void watch(int interval) throws InterruptedException {
        System.out.printf("🔔 Habit Reminder – checking every %d minute(s). Press Ctrl+C to stop.%n", interval);
        while (true) {
            List<Habit> due = new ArrayList<>();
            for (Habit h : habits) if (h.isDue()) due.add(h);
            if (!due.isEmpty()) {
                System.out.printf("%n🔔 Due habits at %s:%n", LocalTime.now().format(DateTimeFormatter.ofPattern("HH:mm")));
                for (Habit h : due) System.out.printf("  %s: %s%n", h.id, h.name);
            } else {
                System.out.print(".");
            }
            Thread.sleep(interval * 60 * 1000L);
        }
    }

    public static void main(String[] args) throws Exception {
        if (args.length < 1) {
            System.out.println("Usage: HabitReminder <command> [options]");
            return;
        }
        Tracker tracker = new Tracker();
        String cmd = args[0];
        switch (cmd) {
            case "add": {
                if (args.length < 2) { System.out.println("add <name> [--frequency N]"); return; }
                String name = args[1];
                int freq = 1;
                for (int i=2; i<args.length; i++) {
                    if (args[i].equals("--frequency") && i+1 < args.length) {
                        freq = Integer.parseInt(args[++i]);
                    }
                }
                tracker.add(name, freq);
                break;
            }
            case "list":
                tracker.list();
                break;
            case "check":
                tracker.check();
                break;
            case "remove": {
                if (args.length < 2) { System.out.println("remove <id>"); return; }
                tracker.remove(args[1]);
                break;
            }
            case "watch": {
                int interval = 5;
                for (int i=1; i<args.length; i++) {
                    if (args[i].equals("--interval") && i+1 < args.length) {
                        interval = Integer.parseInt(args[++i]);
                    }
                }
                tracker.watch(interval);
                break;
            }
            default:
                System.out.println("Unknown command.");
        }
    }
}
