import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

def load_data(csv_file):
    return pd.read_csv(csv_file)

def filter_by_production_channel(data, channel_id):
    return data[data['ProductionChannel'] == channel_id]

def plot_invariant_mass_distribution(data, decay_product_a, decay_product_b):
    mass_column = f'InvMass_{decay_product_a}_{decay_product_b}'
    
    # Check if the invariant mass column exists
    if mass_column not in data.columns:
        print(f"Column {mass_column} not found in the data.")
        return

    # Plot the histogram of the invariant mass distribution
    plt.figure(figsize=(10, 6))
    sns.histplot(data[mass_column].dropna(), bins=50, kde=True)
    plt.title(f'Invariant Mass Distribution: {decay_product_a} + {decay_product_b}')
    plt.xlabel('Invariant Mass (GeV)')
    plt.ylabel('Frequency')
    plt.grid(True)
    plt.show()

def plot_decay_ratios(data):
    decay_products_columns = [col for col in data.columns if 'InvMass' not in col and 'ProductionChannel' not in col]
    
    decay_counts = {}
    for col in decay_products_columns:
        if col in data.columns:
            decay_counts[col] = data[col].notna().sum()
    
    # Plot decay ratios as a bar chart
    plt.figure(figsize=(10, 6))
    sns.barplot(x=list(decay_counts.keys()), y=list(decay_counts.values()))
    plt.title('Decay Ratios for Different Decay Products')
    plt.xlabel('Decay Product')
    plt.ylabel('Count')
    plt.xticks(rotation=90)
    plt.grid(True)
    plt.show()

# Main function to analyze data from the CSV file
def analyze_higgs_data(csv_file, channel_id=None, decay_product_a=None, decay_product_b=None):
    data = load_data(csv_file)
    if channel_id is not None:
        data = filter_by_production_channel(data, channel_id)
    
    if decay_product_a is not None and decay_product_b is not None:
        plot_invariant_mass_distribution(data, decay_product_a, decay_product_b)
    
    plot_decay_ratios(data)

