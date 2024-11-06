import random
import json

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

def update_run_card(file_path, nevents, ebeam):
    with open(file_path, 'r') as file:
        lines = file.readlines()

    with open(file_path, 'w') as file:
        for line in lines:
            # Update the number of events
            if line.strip().startswith('10000 = nevents'):
                line = f"     {nevents} = nevents ! Number of unweighted events requested\n"
            # Update the center-of-mass energy (ebeam1 and ebeam2)
            elif line.strip().startswith('6500.0     = ebeam1'):
                line = f"     {ebeam} = ebeam1  ! beam 1 total energy in GeV\n"
                lines[lines.index(line) + 1] = f"     {ebeam} = ebeam2  ! beam 2 total energy in GeV\n"
                
            file.write(line)

def save_coefficients(file_path, new_coefficients):
    with open(file_path, 'w') as f:
        json.dump(new_coefficients, f)

#Generate Wilsons and update the parameter file
new_coefficients = generate_random_coefficients()
update_wilson_coefficients('/home/ubuntu/MG5_aMC_v3_6_0/SMEFT_run3/Cards/param_card.dat', new_coefficients)
update_run_card('/home/ubuntu/MG5_aMC_v3_6_0/SMEFT_run3/Cards/run_card.dat', 25000, 50000)
save_coefficients('/home/ubuntu/pythia8312/scripts/wilson_coefficients.json', new_coefficients)
