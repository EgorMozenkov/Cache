import random

filename = "test_1000.txt"
levels = 2
algorithms = "LFU ARC"
cache_size = 1000
num_requests = 10000000
unique_keys = 100000 

with open(filename, "w") as f:
    f.write(f"{levels}\n")
    f.write(f"{algorithms}\n")
    f.write(f"{cache_size} {num_requests}\n")
    
    keys = [str(random.randint(1, unique_keys)) for _ in range(num_requests)]
    f.write(" ".join(keys) + "\n")

print(f"Файл {filename} успешно сгенерирован!")