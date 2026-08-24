// HabitReminder.cs
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.Json;
using System.Text.Json.Serialization;
using System.Threading;

class Habit
{
    [JsonPropertyName("id")] public string Id { get; set; }
    [JsonPropertyName("name")] public string Name { get; set; }
    [JsonPropertyName("frequency_days")] public int FrequencyDays { get; set; }
    [JsonPropertyName("last_done")] public string LastDone { get; set; }

    public Habit() { }
    public Habit(string name, int frequencyDays)
    {
        Id = Guid.NewGuid().ToString().Substring(0,8);
        Name = name;
        FrequencyDays = frequencyDays;
        LastDone = null;
    }

    public DateTime NextDueDate()
    {
        if (string.IsNullOrEmpty(LastDone)) return DateTime.Today;
        return DateTime.Parse(LastDone).AddDays(FrequencyDays);
    }

    public bool IsDue() => NextDueDate() <= DateTime.Today;

    public void MarkDone() => LastDone = DateTime.Today.ToString("yyyy-MM-dd");
}

class Tracker
{
    private List<Habit> habits = new List<Habit>();
    private readonly string dataFile = "habits.json";
    private readonly JsonSerializerOptions options = new JsonSerializerOptions { WriteIndented = true };

    public Tracker() => Load();

    private void Load()
    {
        if (!File.Exists(dataFile)) return;
        string json = File.ReadAllText(dataFile);
        habits = JsonSerializer.Deserialize<List<Habit>>(json) ?? new List<Habit>();
    }

    private void Save()
    {
        string json = JsonSerializer.Serialize(habits, options);
        File.WriteAllText(dataFile, json);
    }

    public void Add(string name, int frequency)
    {
        var h = new Habit(name, frequency);
        habits.Add(h);
        Save();
        Console.WriteLine($"✅ Habit added: {h.Name} (ID: {h.Id}, frequency: {frequency} days)");
    }

    public void List()
    {
        if (!habits.Any())
        {
            Console.WriteLine("No habits.");
            return;
        }
        Console.WriteLine("\n📋 Your Habits:");
        foreach (var h in habits)
        {
            var due = h.NextDueDate();
            var daysUntil = (due - DateTime.Today).Days;
            string status;
            if (daysUntil < 0) status = "🔔 DUE!";
            else if (daysUntil == 0) status = "⏰ due today";
            else if (daysUntil == 1) status = "due tomorrow";
            else status = $"due in {daysUntil} days";
            string last = string.IsNullOrEmpty(h.LastDone) ? "never" : h.LastDone;
            Console.WriteLine($"  {h.Id}: {h.Name} (every {h.FrequencyDays} day{(h.FrequencyDays>1?"s":"")}) – last: {last}, next: {due:yyyy-MM-dd} ({status})");
        }
    }

    public void Check()
    {
        var due = habits.Where(h => h.IsDue()).ToList();
        if (!due.Any())
        {
            Console.WriteLine("✅ No habits due today. Well done!");
            return;
        }
        Console.WriteLine("\n🔔 Due habits:");
        foreach (var h in due)
        {
            Console.WriteLine($"  {h.Id}: {h.Name} (frequency: {h.FrequencyDays} days)");
            Console.Write($"  Mark '{h.Name}' as done? (y/n): ");
            var ans = Console.ReadLine()?.Trim().ToLower();
            if (ans == "y")
            {
                h.MarkDone();
                Console.WriteLine($"  ✅ Habit '{h.Name}' marked as done.");
            }
            else
            {
                Console.WriteLine($"  ⏳ Habit '{h.Name}' skipped.");
            }
        }
        Save();
    }

    public void Remove(string id)
    {
        var h = habits.FirstOrDefault(x => x.Id == id);
        if (h == null)
        {
            Console.WriteLine($"❌ Habit with ID {id} not found.");
            return;
        }
        habits.Remove(h);
        Save();
        Console.WriteLine($"✅ Habit '{h.Name}' removed.");
    }

    public void Watch(int interval)
    {
        Console.WriteLine($"🔔 Habit Reminder – checking every {interval} minute(s). Press Ctrl+C to stop.");
        while (true)
        {
            var due = habits.Where(h => h.IsDue()).ToList();
            if (due.Any())
            {
                Console.WriteLine($"\n🔔 Due habits at {DateTime.Now:HH:mm}:");
                foreach (var h in due) Console.WriteLine($"  {h.Id}: {h.Name}");
            }
            else
            {
                Console.Write(".");
            }
            Thread.Sleep(interval * 60 * 1000);
        }
    }
}

class Program
{
    static void Main(string[] args)
    {
        if (args.Length < 1)
        {
            Console.WriteLine("Usage: HabitReminder <command> [options]");
            return;
        }
        var tracker = new Tracker();
        var cmd = args[0];
        switch (cmd)
        {
            case "add":
                if (args.Length < 2) { Console.WriteLine("add <name> [--frequency N]"); return; }
                string name = args[1];
                int freq = 1;
                for (int i=2; i<args.Length; i++)
                {
                    if (args[i] == "--frequency" && i+1 < args.Length)
                        freq = int.Parse(args[++i]);
                }
                tracker.Add(name, freq);
                break;
            case "list":
                tracker.List();
                break;
            case "check":
                tracker.Check();
                break;
            case "remove":
                if (args.Length < 2) { Console.WriteLine("remove <id>"); return; }
                tracker.Remove(args[1]);
                break;
            case "watch":
                int interval = 5;
                for (int i=1; i<args.Length; i++)
                {
                    if (args[i] == "--interval" && i+1 < args.Length)
                        interval = int.Parse(args[++i]);
                }
                tracker.Watch(interval);
                break;
            default:
                Console.WriteLine("Unknown command.");
                break;
        }
    }
}
