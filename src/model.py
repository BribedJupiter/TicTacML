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

def sanitize_features(features):
    # Convert features from hex to decimal
    for row in range(len(features)):
        for x in range(len(features[row])):
            # Check if we're handling a base 10 digit or a base 16 digit (A-F)
            if not str(features[row][x]).isdigit(): 
                features[row][x] = int(features[row][x], 16) # Convert from hex to an integer
            else:
                features[row][x] = int(features[row][x]) # Ensure we're working with integers

# Read our CSV data
csv_filepath = sys.argv[1]
print("[PYTHON] CSV PATH: ", csv_filepath)
data = read_csv(csv_filepath)

# Prep our data arrays (a lot of this follows what we've been doing in the programming assignments)
features = data.values[:, 0:8]
classes = data.values[:, 9]

# Perform conversions so features array is valid
sanitize_features(features)

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

print("[PYTHON] READY")
sys.stdout.flush()

def request_move(request):
    req = list(request[0:8]) # Remove the label from the row data (should be -1, invalid anyway). We have 8 features
    sanitize_features(req) # Prepare features
    return model.predict([req]) 

#############
### TEST ###
#############
shutdown = False
while (not shutdown):
    for line in sys.stdin:
        line = str(line.strip())
        if "shutdown" in line:
            shutdown = True
            print("[PYTHON] Shutting down...")
            sys.stdout.flush()
            break
        elif "RQSTMV" in line:
            # The following line converts line to a str, then reduces it from the [ (+1 in order to not include the [) to the ], then 
            # delimits in by a , to create a list.
            cmd = str(line)[int(line.index("[")) + 1:int(line.index("]"))].split(",")
            print("[PYTHON] Request:", line, " --> ", cmd)

            # Now that we have the request, ask the model for a move
            print("[PYTHON] Your move is...", request_move(cmd))

            sys.stdout.flush()
        else:
            print("[PYTHON] Received: ", line)
            sys.stdout.flush()
