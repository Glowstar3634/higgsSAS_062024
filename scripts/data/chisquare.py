import pandas as pd
import sys
from scipy.stats import chisquare

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

def run_chi_square(expected, observed):
    all_bins = set(expected.keys()).union(set(observed.keys()))
    
    expected = {bin: expected.get(bin, 0) for bin in all_bins}
    observed = {bin: observed.get(bin, 0) for bin in all_bins}
    expected_values = list(expected.values())
    observed_values = list(observed.values())
    
    filtered_expected = []
    filtered_observed = []
    for e, o in zip(expected_values, observed_values):
        if e > 0:  # Only consider bins where expected value is greater than zero
            filtered_expected.append(e)
            filtered_observed.append(o)
    
    total_filtered_expected = sum(filtered_expected)
    total_filtered_observed = sum(filtered_observed)
    if total_filtered_expected != total_filtered_observed:
        scaling_factor = total_filtered_observed / total_filtered_expected
        filtered_expected = [e * scaling_factor for e in filtered_expected]
    
    if filtered_expected and filtered_observed:
        chi2, p = chisquare(f_obs=filtered_observed, f_exp=filtered_expected)
        print(f"Chi-Square Statistic: {chi2}")
        print(f"P-value: {p}")
    else:
        print("No valid bins available for chi-square test.")


def main(expected_file, observed_file, filter_type, filter_value):
    expected_data = pd.read_csv(expected_file)
    observed_data = pd.read_csv(observed_file)
    
    filter_value = str(filter_value) 

    # Apply the appropriate filter
    if filter_type == "production_channel":
        expected_filtered = filter_data_by_production_channel(expected_data, int(filter_value))
        observed_filtered = filter_data_by_production_channel(observed_data, int(filter_value))
        expected_bins = get_decay_product_bins(expected_filtered)
        observed_bins = get_decay_product_bins(observed_filtered)
        
    elif filter_type == "decay_products":
        expected_filtered = filter_data_by_decay_products(expected_data, filter_value)
        observed_filtered = filter_data_by_decay_products(observed_data, filter_value)
        expected_bins = get_production_channel_bins(expected_filtered)
        observed_bins = get_production_channel_bins(observed_filtered)

    else:
        print("Invalid filter type. Use 'production_channel' or 'decay_products'.")
        sys.exit(1)

    # Perform chi-square goodness of fit test
    run_chi_square(expected_bins, observed_bins)

if __name__ == "__main__":
    if len(sys.argv) != 5:
        print("Usage: python chi_square_test.py <expected_file> <observed_file> <filter_type> <filter_value>")
        print("filter_type: 'production_channel' or 'decay_products'")
        print("filter_value: the value for filtering (e.g., 902 or '5;-5')")
        sys.exit(1)

    expected_file = sys.argv[1]
    observed_file = sys.argv[2]
    filter_type = sys.argv[3]
    filter_value = sys.argv[4]
    main(expected_file, observed_file, filter_type, filter_value)
