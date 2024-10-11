import pandas as pd
import sys
from scipy.stats import chisquare

expected_ratios = {
    "production_channel": {
        "902": 0.8610,
        "907": 0.0501,
        "905": 0.0313,
        "906": 0.0203,
        "904": 0.0185,
        "901": 0.0091,
        "908": 0.0038,
        "909": 0.0019,
    },
    "decay_products": {
        "5;-5": 0.571,
        "24;-24": 0.22,
        "21;21": 0.0853,
        "15;-15": 0.0626,
        "4;-4": 0.0288,
        "23;23": 0.0273,    
        "22;22": 0.00228,
        "22;23": 0.00157,
        "13;-13": 0.00022,
    }
}

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

def calculate_chi_square(observed_file, filter_type, filter_value):
    # Read the observed data
    df = pd.read_csv(observed_file)
    
    if filter_type == "production_channel":
        observed_bins = df[df["production_channel"] == int(filter_value)]
        expected_bins = expected_ratios["production_channel"]
    elif filter_type == "decay_products":
        observed_bins = df[df["decay_products"] == filter_value]
        expected_bins = expected_ratios["decay_products"]
    
    observed_counts = observed_bins["count"].values
    expected_counts = [expected_bins.get(str(filter_value), 0) * sum(observed_counts)]
    
    # Perform chi-square test
    chi2, p = chisquare(f_obs=observed_counts, f_exp=expected_counts)
    
    print(f"Chi-Square Statistic: {chi2}")
    print(f"P-value: {p}")


def main(observed_file, filter_type, filter_value):
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

    # Perform chi-square goodness of fit test
    run_chi_square(expected_bins, observed_bins)

if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("Usage: python chi_square_test.py <expected_file> <observed_file> <filter_type> <filter_value>")
        print("filter_type: 'production_channel' or 'decay_products'")
        print("filter_value: the value for filtering (e.g., 902 or '5;-5')")
        sys.exit(1)

    observed_file = sys.argv[1]
    filter_type = sys.argv[2]
    filter_value = sys.argv[3]
    main(observed_file, filter_type, filter_value)