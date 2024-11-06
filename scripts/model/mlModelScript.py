import tensorflow as tf
from tensorflow.keras.models import Sequential, load_model
from tensorflow.keras.layers import Dense
import pandas as pd
import glob
import os
import sys

def load_dataset(file_path):
    """Load a single CSV file as a DataFrame."""
    return pd.read_csv(file_path, skiprows=0, header=1)

def train_on_files(path_pattern, model_path="smeft_model.h5"):
    all_files = glob.glob(path_pattern)
    
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

    for filename in all_files:
        try:
            print(f"Training on {filename}")
            data = pd.read_csv(filename, skiprows=2, header=0)
            
            X = data[['HiggsBoson', 'DecayProducts', 'InvMasses', 'pT', 'Rapidity', 'JetMultiplicity']].values
            y = data[['Coefficient1', 'Coefficient2', 'Coefficient3', 'Coefficient4', 'Coefficient5',
                      'Coefficient6', 'Coefficient7', 'Coefficient8', 'Coefficient9']].values

            model.fit(X, y, epochs=50, batch_size=32, validation_split=0.2, verbose=1)
            model.save(model_path)
            print(f"Model saved after training on {filename}")
        
        except Exception as e:
            print(f"Error processing {filename}: {e}")

def main(training_dataset, model_path):
    train_on_files(training_dataset, model_path)

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python3 modelTraining.py <input_file> <model_path>")
        sys.exit(1)
    training_dataset = sys.argv[1]
    model_path = sys.argv[2]
    main(training_dataset, model_path)
