// habit_reminder.go
package main

import (
	"bufio"
	"encoding/json"
	"flag"
	"fmt"
	"os"
	"strings"
	"time"
	"github.com/google/uuid"
)

type Habit struct {
	ID           string `json:"id"`
	Name         string `json:"name"`
	FrequencyDays int   `json:"frequency_days"`
	LastDone     string `json:"last_done"`
}

func (h *Habit) NextDueDate() time.Time {
	if h.LastDone == "" {
		return time.Now()
	}
	last, _ := time.Parse(time.RFC3339, h.LastDone)
	return last.AddDate(0, 0, h.FrequencyDays)
}

func (h *Habit) IsDue() bool {
	due := h.NextDueDate()
	return due.Before(time.Now()) || due.Equal(time.Now())
}

func (h *Habit) MarkDone() {
	h.LastDone = time.Now().Format(time.RFC3339)
}

type Tracker struct {
	Habits []Habit `json:"habits"`
}

var dataFile = "habits.json"

func (t *Tracker) load() {
	data, err := os.ReadFile(dataFile)
	if err != nil {
		return
	}
	json.Unmarshal(data, t)
}

func (t *Tracker) save() {
	data, _ := json.MarshalIndent(t, "", "  ")
	os.WriteFile(dataFile, data, 0644)
}

func (t *Tracker) add(name string, freq int) {
	habit := Habit{
		ID:           uuid.New().String()[:8],
		Name:         name,
		FrequencyDays: freq,
	}
	t.Habits = append(t.Habits, habit)
	t.save()
	fmt.Printf("✅ Habit added: %s (ID: %s, frequency: %d days)\n", name, habit.ID, freq)
}

func (t *Tracker) list() {
	if len(t.Habits) == 0 {
		fmt.Println("No habits.")
		return
	}
	fmt.Println("\n📋 Your Habits:")
	for _, h := range t.Habits {
		due := h.NextDueDate()
		daysUntil := int(due.Sub(time.Now()).Hours() / 24)
		status := ""
		if daysUntil < 0 {
			status = "🔔 DUE!"
		} else if daysUntil == 0 {
			status = "⏰ due today"
		} else if daysUntil == 1 {
			status = "due tomorrow"
		} else {
			status = fmt.Sprintf("due in %d days", daysUntil)
		}
		last := h.LastDone
		if last == "" {
			last = "never"
		} else {
			last = last[:10]
		}
		fmt.Printf("  %s: %s (every %d day%s) – last: %s, next: %s (%s)\n",
			h.ID, h.Name, h.FrequencyDays, func() string {
				if h.FrequencyDays > 1 { return "s" } else { return "" }
			}(), last, due.Format("2006-01-02"), status)
	}
}

func (t *Tracker) check() {
	due := []Habit{}
	for _, h := range t.Habits {
		if h.IsDue() {
			due = append(due, h)
		}
	}
	if len(due) == 0 {
		fmt.Println("✅ No habits due today. Well done!")
		return
	}
	fmt.Println("\n🔔 Due habits:")
	reader := bufio.NewReader(os.Stdin)
	for _, h := range due {
		fmt.Printf("  %s: %s (frequency: %d days)\n", h.ID, h.Name, h.FrequencyDays)
		fmt.Printf("  Mark '%s' as done? (y/n): ", h.Name)
		input, _ := reader.ReadString('\n')
		input = strings.TrimSpace(strings.ToLower(input))
		if input == "y" {
			for i := range t.Habits {
				if t.Habits[i].ID == h.ID {
					t.Habits[i].MarkDone()
					break
				}
			}
			fmt.Printf("  ✅ Habit '%s' marked as done.\n", h.Name)
		} else {
			fmt.Printf("  ⏳ Habit '%s' skipped.\n", h.Name)
		}
	}
	t.save()
}

func (t *Tracker) remove(id string) {
	for i, h := range t.Habits {
		if h.ID == id {
			name := h.Name
			t.Habits = append(t.Habits[:i], t.Habits[i+1:]...)
			t.save()
			fmt.Printf("✅ Habit '%s' removed.\n", name)
			return
		}
	}
	fmt.Printf("❌ Habit with ID %s not found.\n", id)
}

func (t *Tracker) watch(interval int) {
	fmt.Printf("🔔 Habit Reminder – checking every %d minute(s). Press Ctrl+C to stop.\n", interval)
	ticker := time.NewTicker(time.Duration(interval) * time.Minute)
	defer ticker.Stop()
	for {
		select {
		case <-ticker.C:
			due := []Habit{}
			for _, h := range t.Habits {
				if h.IsDue() {
					due = append(due, h)
				}
			}
			if len(due) > 0 {
				fmt.Printf("\n🔔 Due habits at %s:\n", time.Now().Format("15:04"))
				for _, h := range due {
					fmt.Printf("  %s: %s\n", h.ID, h.Name)
				}
			} else {
				fmt.Print(".")
			}
		}
	}
}

func main() {
	if len(os.Args) < 2 {
		fmt.Println("Usage: habit_reminder <command> [options]")
		return
	}
	tracker := &Tracker{}
	tracker.load()
	cmd := os.Args[1]
	switch cmd {
	case "add":
		addCmd := flag.NewFlagSet("add", flag.ExitOnError)
		name := addCmd.String("name", "", "")
		freq := addCmd.Int("frequency", 1, "")
		addCmd.Parse(os.Args[2:])
		if *name == "" && len(addCmd.Args()) > 0 {
			*name = addCmd.Args()[0]
		}
		if *name == "" {
			fmt.Println("add requires a name")
			return
		}
		tracker.add(*name, *freq)
	case "list":
		tracker.list()
	case "check":
		tracker.check()
	case "remove":
		if len(os.Args) < 3 {
			fmt.Println("remove <id>")
			return
		}
		tracker.remove(os.Args[2])
	case "watch":
		watchCmd := flag.NewFlagSet("watch", flag.ExitOnError)
		interval := watchCmd.Int("interval", 5, "")
		watchCmd.Parse(os.Args[2:])
		tracker.watch(*interval)
	default:
		fmt.Println("Unknown command. Use add, list, check, remove, watch.")
	}
}
