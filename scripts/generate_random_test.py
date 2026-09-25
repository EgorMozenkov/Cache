import random

filename = "test_1000.txt"
levels = 4
algorithms = "LFU 2Q ARC LIRS"
cache_size = 1000
num_requests = 1000000
unique_keys = 5000

with open(filename, "w") as f:
    f.write(f"{levels}\n")
    f.write(f"{algorithms}\n")
    f.write(f"{cache_size} {num_requests}\n")
    
    keys = [str(random.randint(1, unique_keys)) for _ in range(num_requests)]
    f.write(" ".join(keys) + "\n")

print(f"Файл {filename} успешно сгенерирован!")