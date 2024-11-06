import tensorflow as tf
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import Dense
import pandas as pd
import numpy as np
import glob

# Load all CSV files and combine into a single dataset
def load_datasets(path_pattern):
    all_files = glob.glob(path_pattern)
    datasets = []
    for filename in all_files:
        data = pd.read_csv(filename, skiprows=2, header=0)
        datasets.append(data)
    return pd.concat(datasets, ignore_index=True)

data = load_datasets("path/to/train_*.csv")
X = data[['HiggsBoson', 'ProductionChannel', 'DecayProducts', 'InvMasses', 'pT', 'Rapidity', 'JetMultiplicity']]
y = data[['Coefficient1', 'Coefficient2', 'Coefficient3', 'Coefficient4', 'Coefficient5',
          'Coefficient6', 'Coefficient7', 'Coefficient8', 'Coefficient9']]
X = X.values
y = y.values

model = Sequential([
    Dense(64, activation='relu', input_shape=(X.shape[1],)),
    Dense(128, activation='relu'),
    Dense(64, activation='relu'),
    Dense(9)  # 9 outputs for the Wilson coefficients
])
model.compile(optimizer='adam', loss='mse', metrics=['mae'])
history = model.fit(X, y, epochs=50, batch_size=32, validation_split=0.2)

model.save("smeft_model.h5")
