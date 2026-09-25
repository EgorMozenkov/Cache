import random

filename = "real_test.txt"
levels = 4
algorithms = "LFU 2Q ARC LIRS"
cache_size = 1000
num_requests = 1000000
unique_keys = 5000

print("Вычисление весов по закону Ципфа...")
weights = [1.0 / (i ** 1.0) for i in range(1, unique_keys + 1)]
keys_pool = list(range(1, unique_keys + 1))

print("Генерация запросов...")
sampled_keys = random.choices(keys_pool, weights=weights, k=num_requests)

with open(filename, "w") as f:
    f.write(f"{levels}\n")
    f.write(f"{algorithms}\n")
    f.write(f"{cache_size} {num_requests}\n")
    f.write(" ".join(map(str, sampled_keys)) + "\n")

print(f"Файл {filename} успешно сгенерирован!")