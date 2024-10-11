import pandas as pd
import sys
from scipy.stats import chisquare

# Load observed data and filter as needed
def filter_data_by_production_channel(data, channel):
    print(f"Filtering for Production Channel {channel} ...")
    return data[data['ProductionChannel'] == channel]

def filter_data_by_decay_products(data, decay_products):
    print(f"Filtering for Decay Products {decay_products} ...")
    return data[data['DecayProducts'].str.contains(decay_products, regex=False)]

def get_decay_product_bins(filtered_data):
    decay_pairs = filtered_data['DecayProducts'].str.split(';')
    return pd.Series([tuple(sorted([decay[i], decay[i+1]])) for decay in decay_pairs for i in range(0, len(decay)-1, 2)]).value_counts()

def get_production_channel_bins(filtered_data):
    return filtered_data['ProductionChannel'].value_counts()

# Remove bins with 0 counts in either the observed or expected datasets
def calculate_chi_square(observed_bins, expected_bins):
    total_observed = sum(observed_bins)  # Total number of observed events

    # Ensure all expected bins are in the observed bins
    expected_counts = []
    observed_counts = []

    for bin_name, expected_ratio in expected_bins.items():
        observed_count = observed_bins.get(bin_name, 0)  # If bin is missing in observed, count as 0
        expected_count = expected_ratio * total_observed  # Expected count based on the total observed

        if observed_count > 0 and expected_count > 0:  # Filter out 0 values
            observed_counts.append(observed_count)
            expected_counts.append(expected_count)

    if len(observed_counts) == 0:
        print("No valid bins for chi-square calculation.")
        return

    # Perform chi-square test
    chi2, p = chisquare(f_obs=observed_counts, f_exp=expected_counts)

    print(f"Chi-Square Statistic: {chi2}")
    print(f"P-value: {p}")

def main(observed_file, expected_file, filter_type, filter_value):
    # Load observed data
    observed_data = pd.read_csv(observed_file)
    filter_value = str(filter_value) 

    # Apply the appropriate filter
    if filter_type == "production_channel":
        observed_filtered = filter_data_by_production_channel(observed_data, int(filter_value))
        observed_bins = get_decay_product_bins(observed_filtered)
        
    elif filter_type == "decay_products":
        observed_filtered = filter_data_by_decay_products(observed_data, filter_value)
        observed_bins = get_production_channel_bins(observed_filtered)

    else:
        print("Invalid filter type. Use 'production_channel' or 'decay_products'.")
        sys.exit(1)

    # Load expected data
    expected_data = pd.read_csv(expected_file)
    
    # Assuming the expected data follows the same binning method as observed
    if filter_type == "production_channel":
        expected_bins = get_decay_product_bins(expected_data)
    elif filter_type == "decay_products":
        expected_bins = get_production_channel_bins(expected_data)

    # Perform chi-square goodness of fit test
    calculate_chi_square(observed_bins, expected_bins)

if __name__ == "__main__":
    if len(sys.argv) != 5:
        print("Usage: python chisquare.py <observed_file> <expected_file> <filter_type> <filter_value>")
        print("filter_type: 'production_channel' or 'decay_products'")
        print("filter_value: the value for filtering (e.g., 902 or '5;-5')")
        sys.exit(1)

    observed_file = sys.argv[1]
    expected_file = sys.argv[2]
    filter_type = sys.argv[3]
    filter_value = sys.argv[4]
    main(observed_file, expected_file, filter_type, filter_value)
