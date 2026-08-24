# habit_reminder.rb
#!/usr/bin/env ruby
require 'json'
require 'securerandom'
require 'date'
require 'optparse'

DATA_FILE = 'habits.json'

class Habit
  attr_accessor :id, :name, :frequency_days, :last_done

  def initialize(name, frequency_days = 1, id = nil)
    @id = id || SecureRandom.hex(4)
    @name = name
    @frequency_days = frequency_days
    @last_done = nil
  end

  def next_due_date
    return Date.today if @last_done.nil?
    last = Date.iso8601(@last_done)
    last + @frequency_days
  end

  def is_due?
    next_due_date <= Date.today
  end

  def mark_done
    @last_done = Date.today.iso8601
  end

  def to_hash
    { id: @id, name: @name, frequency_days: @frequency_days, last_done: @last_done }
  end

  def self.from_hash(h)
    habit = new(h['name'], h['frequency_days'], h['id'])
    habit.last_done = h['last_done']
    habit
  end
end

class Tracker
  attr_reader :habits

  def initialize
    @habits = []
    load
  end

  def load
    if File.exist?(DATA_FILE)
      data = JSON.parse(File.read(DATA_FILE))
      @habits = data.map { |h| Habit.from_hash(h) }
    end
  end

  def save
    File.write(DATA_FILE, JSON.pretty_generate(@habits.map(&:to_hash)))
  end

  def add(name, frequency = 1)
    habit = Habit.new(name, frequency)
    @habits << habit
    save
    puts "✅ Habit added: #{habit.name} (ID: #{habit.id}, frequency: #{frequency} days)"
  end

  def list
    if @habits.empty?
      puts "No habits."
      return
    end
    puts "\n📋 Your Habits:"
    @habits.each do |h|
      due = h.next_due_date
      days_until = (due - Date.today).to_i
      status = if days_until < 0
                 "🔔 DUE!"
               elsif days_until == 0
                 "⏰ due today"
               elsif days_until == 1
                 "due tomorrow"
               else
                 "due in #{days_until} days"
               end
      last = h.last_done || "never"
      puts "  #{h.id}: #{h.name} (every #{h.frequency_days} day#{h.frequency_days>1 ? 's' : ''}) – last: #{last}, next: #{due} (#{status})"
    end
  end

  def check
    due = @habits.select(&:is_due?)
    if due.empty?
      puts "✅ No habits due today. Well done!"
      return
    end
    puts "\n🔔 Due habits:"
    due.each do |h|
      puts "  #{h.id}: #{h.name} (frequency: #{h.frequency_days} days)"
      print "  Mark '#{h.name}' as done? (y/n): "
      ans = gets.chomp.downcase
      if ans == 'y'
        h.mark_done
        puts "  ✅ Habit '#{h.name}' marked as done."
      else
        puts "  ⏳ Habit '#{h.name}' skipped."
      end
    end
    save
  end

  def remove(id)
    habit = @habits.find { |h| h.id == id }
    unless habit
      puts "❌ Habit with ID #{id} not found."
      return
    end
    name = habit.name
    @habits.delete(habit)
    save
    puts "✅ Habit '#{name}' removed."
  end

  def watch(interval = 5)
    puts "🔔 Habit Reminder – checking every #{interval} minute(s). Press Ctrl+C to stop."
    trap('INT') { puts "\n👋 Reminder stopped."; exit }
    loop do
      due = @habits.select(&:is_due?)
      if due.any?
        puts "\n🔔 Due habits at #{Time.now.strftime('%H:%M')}:"
        due.each { |h| puts "  #{h.id}: #{h.name}" }
      else
        print "."
      end
      sleep(interval * 60)
    end
  end
end

if ARGV.empty?
  puts "Usage: habit_reminder.rb <command> [options]"
  exit
end

tracker = Tracker.new
cmd = ARGV.shift

case cmd
when 'add'
  name = ARGV.shift
  frequency = 1
  if ARGV.include?('--frequency')
    idx = ARGV.index('--frequency')
    frequency = ARGV[idx+1].to_i if idx
  end
  if name.nil?
    puts "add <name> [--frequency N]"
    exit
  end
  tracker.add(name, frequency)
when 'list'
  tracker.list
when 'check'
  tracker.check
when 'remove'
  id = ARGV.shift
  if id.nil?
    puts "remove <id>"
    exit
  end
  tracker.remove(id)
when 'watch'
  interval = 5
  if ARGV.include?('--interval')
    idx = ARGV.index('--interval')
    interval = ARGV[idx+1].to_i if idx
  end
  tracker.watch(interval)
else
  puts "Unknown command. Use add, list, check, remove, watch."
end
