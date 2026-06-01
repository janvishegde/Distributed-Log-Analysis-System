import sys, random, datetime, os

LEVELS  = ["INFO", "WARN", "ERROR"]
WEIGHTS = [0.75, 0.15, 0.10]

IPS = ["192.168.1."+str(i) for i in range(1,20)] + \
      ["10.0.0."+str(i) for i in range(1,10)] + \
      ["172.16.0.5", "203.0.113.42", "198.51.100.7"]

PATHS   = ["/api/login", "/api/data", "/health", "/metrics", "/api/users", "/admin"]
STATUS  = {"INFO":[200,201,204],"WARN":[301,400,429],"ERROR":[500,502,503,403]}
MESSAGES= {"INFO":["Request processed","Cache hit","Session started"],
           "WARN":["Slow response","Rate limit approaching","Retry attempt"],
           "ERROR":["Connection refused","Timeout exceeded","Database unreachable"]}

def gen_line(ts):
    level  = random.choices(LEVELS, weights=WEIGHTS)[0]
    ip     = random.choice(IPS)
    path   = random.choice(PATHS)
    status = random.choice(STATUS[level])
    msg    = random.choice(MESSAGES[level])
    ms     = random.randint(1, 3000)
    return f"{ts} {level} {ip} \"{path}\" {status} {ms}ms - {msg}\n"

if len(sys.argv) < 3:
    print(f"Usage: {sys.argv[0]} <num_lines> <output_file>")
    sys.exit(1)

n, path = int(sys.argv[1]), sys.argv[2]
base    = datetime.datetime(2024,1,1,0,0,0)
step    = datetime.timedelta(milliseconds=10)

print(f"Generating {n:,} log lines → {path} ...")
with open(path, "w", buffering=1<<20) as f:
    for i in range(n):
        ts = (base + step*i).strftime("%Y-%m-%dT%H:%M:%S.%f")[:-3]
        f.write(gen_line(ts))

print(f"Done — {os.path.getsize(path)/1024/1024:.1f} MB written")
