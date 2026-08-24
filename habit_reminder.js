// habit_reminder.js
#!/usr/bin/env node
const fs = require('fs');
const readline = require('readline');
const { program } = require('commander');
const { v4: uuidv4 } = require('uuid');

const DATA_FILE = 'habits.json';

class Habit {
    constructor(name, frequencyDays = 1, id = null) {
        this.id = id || uuidv4().slice(0,8);
        this.name = name;
        this.frequencyDays = frequencyDays;
        this.lastDone = null;
    }

    nextDueDate() {
        if (!this.lastDone) return new Date();
        const last = new Date(this.lastDone);
        last.setDate(last.getDate() + this.frequencyDays);
        return last;
    }

    isDue() {
        const due = this.nextDueDate();
        return due <= new Date();
    }

    markDone() {
        this.lastDone = new Date().toISOString();
    }
}

class Tracker {
    constructor() {
        this.habits = [];
        this.load();
    }

    load() {
        if (fs.existsSync(DATA_FILE)) {
            try {
                const data = JSON.parse(fs.readFileSync(DATA_FILE));
                this.habits = data.map(h => {
                    const habit = new Habit(h.name, h.frequencyDays, h.id);
                    habit.lastDone = h.lastDone;
                    return habit;
                });
            } catch (e) {}
        }
    }

    save() {
        fs.writeFileSync(DATA_FILE, JSON.stringify(this.habits, null, 2));
    }

    add(name, frequency) {
        const habit = new Habit(name, frequency);
        this.habits.push(habit);
        this.save();
        console.log(`✅ Habit added: ${habit.name} (ID: ${habit.id}, frequency: ${frequency} days)`);
    }

    list() {
        if (this.habits.length === 0) {
            console.log('No habits.');
            return;
        }
        console.log('\n📋 Your Habits:');
        for (const h of this.habits) {
            const due = h.nextDueDate();
            const today = new Date();
            const diff = Math.ceil((due - today) / (1000*60*60*24));
            let status;
            if (diff < 0) status = '🔔 DUE!';
            else if (diff === 0) status = '⏰ due today';
            else if (diff === 1) status = 'due tomorrow';
            else status = `due in ${diff} days`;
            const last = h.lastDone ? h.lastDone.slice(0,10) : 'never';
            console.log(`  ${h.id}: ${h.name} (every ${h.frequencyDays} day${h.frequencyDays>1?'s':''}) – last: ${last}, next: ${due.toISOString().slice(0,10)} (${status})`);
        }
    }

    async check() {
        const due = this.habits.filter(h => h.isDue());
        if (due.length === 0) {
            console.log('✅ No habits due today. Well done!');
            return;
        }
        console.log('\n🔔 Due habits:');
        const rl = readline.createInterface({
            input: process.stdin,
            output: process.stdout
        });
        for (const h of due) {
            console.log(`  ${h.id}: ${h.name} (frequency: ${h.frequencyDays} days)`);
            const answer = await new Promise(resolve => {
                rl.question(`  Mark '${h.name}' as done? (y/n): `, resolve);
            });
            if (answer.trim().toLowerCase() === 'y') {
                h.markDone();
                console.log(`  ✅ Habit '${h.name}' marked as done.`);
            } else {
                console.log(`  ⏳ Habit '${h.name}' skipped.`);
            }
        }
        rl.close();
        this.save();
    }

    remove(id) {
        const idx = this.habits.findIndex(h => h.id === id);
        if (idx === -1) {
            console.log(`❌ Habit with ID ${id} not found.`);
            return;
        }
        const name = this.habits[idx].name;
        this.habits.splice(idx, 1);
        this.save();
        console.log(`✅ Habit '${name}' removed.`);
    }

    watch(interval = 5) {
        console.log(`🔔 Habit Reminder – checking every ${interval} minute(s). Press Ctrl+C to stop.`);
        const timer = setInterval(() => {
            const due = this.habits.filter(h => h.isDue());
            if (due.length > 0) {
                console.log(`\n🔔 Due habits at ${new Date().toTimeString().slice(0,5)}:`);
                for (const h of due) {
                    console.log(`  ${h.id}: ${h.name}`);
                }
            } else {
                process.stdout.write('.');
            }
        }, interval * 60 * 1000);
        process.on('SIGINT', () => {
            clearInterval(timer);
            console.log('\n👋 Reminder stopped.');
            process.exit(0);
        });
    }
}

program
    .command('add <name>')
    .option('--frequency <n>', 'Frequency in days', parseInt, 1)
    .action((name, options) => {
        const tracker = new Tracker();
        tracker.add(name, options.frequency);
    });

program
    .command('list')
    .action(() => {
        const tracker = new Tracker();
        tracker.list();
    });

program
    .command('check')
    .action(async () => {
        const tracker = new Tracker();
        await tracker.check();
    });

program
    .command('remove <id>')
    .action((id) => {
        const tracker = new Tracker();
        tracker.remove(id);
    });

program
    .command('watch')
    .option('--interval <n>', 'Check interval in minutes', parseInt, 5)
    .action((options) => {
        const tracker = new Tracker();
        tracker.watch(options.interval);
    });

program.parse(process.argv);
