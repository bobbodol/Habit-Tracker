# habit_reminder.php
#!/usr/bin/env php
<?php

define('DATA_FILE', 'habits.json');

class Habit {
    public $id;
    public $name;
    public $frequency_days;
    public $last_done;

    function __construct($name, $frequency_days = 1, $id = null) {
        $this->id = $id ?: substr(bin2hex(random_bytes(4)), 0, 8);
        $this->name = $name;
        $this->frequency_days = $frequency_days;
        $this->last_done = null;
    }

    function nextDueDate() {
        if (!$this->last_done) return new DateTime();
        $last = new DateTime($this->last_done);
        $last->modify("+{$this->frequency_days} days");
        return $last;
    }

    function isDue() {
        return $this->nextDueDate() <= new DateTime();
    }

    function markDone() {
        $this->last_done = (new DateTime())->format('Y-m-d');
    }

    function toArray() {
        return [
            'id' => $this->id,
            'name' => $this->name,
            'frequency_days' => $this->frequency_days,
            'last_done' => $this->last_done
        ];
    }

    static function fromArray($data) {
        $h = new self($data['name'], $data['frequency_days'], $data['id']);
        $h->last_done = $data['last_done'] ?? null;
        return $h;
    }
}

class Tracker {
    private $habits = [];

    function __construct() {
        $this->load();
    }

    function load() {
        if (file_exists(DATA_FILE)) {
            $data = json_decode(file_get_contents(DATA_FILE), true);
            foreach ($data as $item) {
                $this->habits[] = Habit::fromArray($item);
            }
        }
    }

    function save() {
        $data = array_map(function($h) { return $h->toArray(); }, $this->habits);
        file_put_contents(DATA_FILE, json_encode($data, JSON_PRETTY_PRINT));
    }

    function add($name, $frequency = 1) {
        $habit = new Habit($name, $frequency);
        $this->habits[] = $habit;
        $this->save();
        echo "✅ Habit added: {$habit->name} (ID: {$habit->id}, frequency: {$frequency} days)\n";
    }

    function list() {
        if (empty($this->habits)) {
            echo "No habits.\n";
            return;
        }
        echo "\n📋 Your Habits:\n";
        foreach ($this->habits as $h) {
            $due = $h->nextDueDate();
            $today = new DateTime();
            $diff = $today->diff($due)->days;
            if ($due < $today) {
                $status = "🔔 DUE!";
            } elseif ($diff == 0) {
                $status = "⏰ due today";
            } elseif ($diff == 1) {
                $status = "due tomorrow";
            } else {
                $status = "due in $diff days";
            }
            $last = $h->last_done ?: "never";
            echo "  {$h->id}: {$h->name} (every {$h->frequency_days} day" . ($h->frequency_days>1?'s':'') . ") – last: $last, next: {$due->format('Y-m-d')} ($status)\n";
        }
    }

    function check() {
        $due = array_filter($this->habits, function($h) { return $h->isDue(); });
        if (empty($due)) {
            echo "✅ No habits due today. Well done!\n";
            return;
        }
        echo "\n🔔 Due habits:\n";
        foreach ($due as $h) {
            echo "  {$h->id}: {$h->name} (frequency: {$h->frequency_days} days)\n";
            echo "  Mark '{$h->name}' as done? (y/n): ";
            $ans = trim(fgets(STDIN));
            if (strtolower($ans) == 'y') {
                $h->markDone();
                echo "  ✅ Habit '{$h->name}' marked as done.\n";
            } else {
                echo "  ⏳ Habit '{$h->name}' skipped.\n";
            }
        }
        $this->save();
    }

    function remove($id) {
        foreach ($this->habits as $i => $h) {
            if ($h->id == $id) {
                $name = $h->name;
                array_splice($this->habits, $i, 1);
                $this->save();
                echo "✅ Habit '$name' removed.\n";
                return;
            }
        }
        echo "❌ Habit with ID $id not found.\n";
    }

    function watch($interval = 5) {
        echo "🔔 Habit Reminder – checking every $interval minute(s). Press Ctrl+C to stop.\n";
        while (true) {
            $due = array_filter($this->habits, function($h) { return $h->isDue(); });
            if (!empty($due)) {
                echo "\n🔔 Due habits at " . date('H:i') . ":\n";
                foreach ($due as $h) {
                    echo "  {$h->id}: {$h->name}\n";
                }
            } else {
                echo ".";
            }
            sleep($interval * 60);
        }
    }
}

if ($argc < 2) {
    die("Usage: php habit_reminder.php <command> [options]\n");
}
$tracker = new Tracker();
$cmd = $argv[1];

switch ($cmd) {
    case 'add':
        if ($argc < 3) die("add <name> [--frequency N]\n");
        $name = $argv[2];
        $frequency = 1;
        for ($i=3; $i<$argc; $i++) {
            if ($argv[$i] == '--frequency' && isset($argv[$i+1])) {
                $frequency = (int)$argv[++$i];
            }
        }
        $tracker->add($name, $frequency);
        break;
    case 'list':
        $tracker->list();
        break;
    case 'check':
        $tracker->check();
        break;
    case 'remove':
        if ($argc < 3) die("remove <id>\n");
        $tracker->remove($argv[2]);
        break;
    case 'watch':
        $interval = 5;
        for ($i=2; $i<$argc; $i++) {
            if ($argv[$i] == '--interval' && isset($argv[$i+1])) {
                $interval = (int)$argv[++$i];
            }
        }
        $tracker->watch($interval);
        break;
    default:
        echo "Unknown command. Use add, list, check, remove, watch.\n";
}
?>
