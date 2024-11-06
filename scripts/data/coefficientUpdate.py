import random

def generate_random_coefficients(num_coefficients=9, min_value=-10, max_value=10):
    return {i + 1: random.uniform(min_value, max_value) for i in range(num_coefficients)}

def update_wilson_coefficients(file_path, new_coefficients):
    with open(file_path, 'r') as file:
        lines = file.readlines()

    with open(file_path, 'w') as file:
        block_found = False
        counter = 0

        for line in lines:
            if line.startswith("BLOCK SMEFT"):
                block_found = True
                file.write(line)
                continue

            if block_found and counter < 9:
                parts = line.split()
                if len(parts) > 1 and parts[0].isdigit():
                    index = int(parts[0])
                    if index in new_coefficients:
                        parts[1] = f"{new_coefficients[index]:.6e}"
                    line = " ".join(parts) + "\n"
                counter += 1
            
            file.write(line)

def save_coefficients(file_path, new_coefficients):
    with open(file_path, 'w') as f:
        json.dump(new_coefficients, f)


#Generate Wilsons and update the parameter file
new_coefficients = generate_random_coefficients()
update_wilson_coefficients('param_card.dat', new_coefficients)
save_coefficients('wilson_coefficients.json', new_coefficients)
