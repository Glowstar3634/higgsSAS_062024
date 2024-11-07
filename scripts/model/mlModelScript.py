import tensorflow as tf
from tensorflow.keras.models import Sequential, load_model
from tensorflow.keras.layers import Dense
import pandas as pd
import glob
import os
import sys
import re

def load_dataset(file_path):
    """Load a single CSV file as a DataFrame."""
    return pd.read_csv(file_path)

def load_wilson_coefficients(data):
    """Extract Wilson coefficients from the first row of the DataFrame."""
    # Get the entire first row (assuming the first row contains the coefficients)
    comment_row = data.iloc[0, :]  # Get the entire first row
    
    # Concatenate all columns in the first row to form a single string
    comment_string = " ".join(comment_row.astype(str).values)
    
    # Use regex to extract all Wilson coefficients
    match = re.findall(r'(\d):\s*(-?\d+\.\d+)', comment_string)
    
    # Create a list of coefficients in order
    coefficients = [float(coeff[1]) for coeff in sorted(match, key=lambda x: int(x[0]))]
    
    return coefficients

def train_on_files(training_dataset, model_path="smeft_model.h5"):
    data = load_dataset(training_dataset)
    
    # Check if model exists; if not, create a new one
    if os.path.exists(model_path):
        print("Loading existing model...")
        model = load_model(model_path)
    else:
        print("Creating a new model...")
        model = Sequential([
            Dense(64, activation='relu', input_shape=(6,)),  # 6 input features
            Dense(128, activation='relu'),
            Dense(64, activation='relu'),
            Dense(9)  # 9 outputs for the Wilson coefficients
        ])
        model.compile(optimizer='adam', loss='mse', metrics=['mae'])

    
    print(f"Training on {training_dataset}")

    wilson_coefficients = load_wilson_coefficients(data)
    data = pd.read_csv(training_dataset, skiprows=1)
    print(data.head())
    
    X = data[['HiggsBoson', 'DecayProducts', 'InvMasses', 'pT', 'Rapidity', 'JetMultiplicity']].values
    y = wilson_coefficients

    model.fit(X, y, epochs=50, batch_size=32, validation_split=0.2, verbose=1)
    model.save(model_path)
    print(f"Model saved after training on {training_dataset}")

def main(training_dataset, model_path):
    train_on_files(training_dataset, model_path)

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python3 modelTraining.py <input_file> <model_path>")
        sys.exit(1)
    training_dataset = sys.argv[1]
    model_path = sys.argv[2]
    main(training_dataset, model_path)
