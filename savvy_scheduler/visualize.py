#!/usr/bin/python3

# Classes to represent events in the log
class GthreadEvent:
    def __init__(self, timestamp):
        self.timestamp = timestamp

    def __str__(self):
        return str(timestamp)


class Start(GthreadEvent):
    def __str__(self):
        return "Start(%d)" % self.timestamp


class Stop(GthreadEvent):
    def __str__(self):
        return "Stop(%d)" % self.timestamp


class Cont(GthreadEvent):
    def __str__(self):
        return "Cont(%d)" % self.timestamp


# Opening/reading log file
lines = open("gthread.log", "r").readlines()

# setting up variables to track state
curr_proc = "000"  # process 000 refers to the main thread (not scheduled)
proc_data = dict()
min_time = 100000
max_time = -1

# Parse events
for l in lines:
    l = l.strip().split(" ")
    timestamp = int(l[-1])
    if timestamp < min_time:
        min_time = timestamp
    if timestamp > max_time:
        max_time = timestamp

    if l[0] == "Registered":
        assert l[1] not in proc_data
        proc_data[l[1]] = []
        proc_data[l[1]].append(Start(timestamp))
    elif l[0] == "Switched":
        # Context switch needs to record that it stopped the last proceess
        if l[2] == "000":
            break
        if curr_proc in proc_data:
            proc_data[curr_proc].append(Stop(timestamp))
        assert l[2] in proc_data
        proc_data[l[2]].append(Cont(timestamp))
        curr_proc = l[2]
    elif l[0] == "Ended":
        if curr_proc in proc_data:
            proc_data[curr_proc].append(Stop(timestamp))
            curr_proc = "000"
    else:
        continue

# Fill in the details and generate state for every ~10ms
running_data = ["   " for i in range(min_time, max_time)]

# Currently unused registration data
register_data = [[] for i in range(min_time, max_time)]

for proc in proc_data:
    events = proc_data[proc]
    for i in range(len(events)):
        pos = events[i].timestamp - min_time
        if isinstance(events[i], Start):
            register_data[pos].append(proc)
        elif isinstance(events[i], Cont):
            assert isinstance(events[i + 1], Stop)
            end_pos = events[i + 1].timestamp - min_time
            for i in range(pos, end_pos):
                running_data[i] = proc
        else:
            continue

# Only look at data collected every second
# There's a slight offset so it's not perfect
display_data = []
for i in range(len(running_data)):
    if i % 100 == 0:
        display_data.append(running_data[i])

# Sort threads by first run time and print
sorted_order = list(proc_data.keys())
sorted_order.sort(key=lambda k: proc_data[k][1].timestamp)
print(sorted_order)
for k in sorted_order:
    filtered = list(map(lambda l: "+" if k == l else " ", display_data))
    print(k + ": " + "".join(filtered))
