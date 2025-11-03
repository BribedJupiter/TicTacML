# Imports
import sys
from pandas import read_csv
from sklearn.model_selection import train_test_split
from sklearn.metrics import confusion_matrix
from sklearn.metrics import accuracy_score
from sklearn.tree import DecisionTreeClassifier
import numpy as np

#############
### TRAIN ###
#############

# Read our CSV data
csv_filepath = sys.argv[1]
print("CSV PATH: ", csv_filepath)
data = read_csv(csv_filepath)

# Prep our data arrays (a lot of this follows what we've been doing in the programming assignments)
features = data.values[:, 0:8]
classes = data.values[:, 9]

# Convert features from hex to decimal
for row in range(len(features)):
    for x in range(len(features[row])):
        # Check if we're handling a base 10 digit or a base 16 digit (A-F)
        if not str(features[row][x]).isdigit(): 
            features[row][x] = int(features[row][x], 16) # Convert from hex to an integer
        else:
            features[row][x] = int(features[row][x]) # Ensure we're working with integers

# Ensure classes are integers too
classes = classes.astype(int)

# Train and test our model using 2-fold cross-validation
model = DecisionTreeClassifier()
x_fold1, x_fold2, y_fold1, y_fold2 = train_test_split(features, classes, test_size=0.5, random_state=0)
model.fit(x_fold1, y_fold1)
prediction1 = model.predict(x_fold2)
model.fit(x_fold2, y_fold2)
prediction2 = model.predict(x_fold1)

# Join results
actual = np.concatenate([y_fold2, y_fold1])
predicted = np.concatenate([prediction1, prediction2])

# Output confusion matrix, accuracy score
print("[PYTHON] Confusion matrix:\n", confusion_matrix(actual, predicted))
print("[PYTHON] Accuracy:", accuracy_score(actual, predicted))
sys.stdout.flush()

#############
### TEST ###
#############
shutdown = False
while (not shutdown):
    for line in sys.stdin:
        line = line.strip()
        if line == "shutdown":
            shutdown = True
            print("[PYTHON] Shutting down...")
            sys.stdout.flush()
            break
        print("[PYTHON] Received: ", line)
        sys.stdout.flush()
