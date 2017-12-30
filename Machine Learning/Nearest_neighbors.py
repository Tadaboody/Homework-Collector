from scipy.io import loadmat
from sklearn.neighbors import NearestNeighbors, KNeighborsClassifier
from math import sqrt
mats = loadmat('ABCD_data.mat')
train_data = mats['train_data']
test_data = mats['test_data']
valid_data = mats['valid_data']
k_neighbors = list()
success_rates = list()
n = len(train_data)
max_k = 0
max_rate = 0
valid_data_length = len(valid_data)
tags = list()

t = ['a', 'b', 'c', 'd']

for i in range(len(t)):
    for j in range(500):
        tags.append(t[i])

for k in range(1, 150):
    success = 0
    clf = KNeighborsClassifier(n_neighbors=k)
    clf.fit(train_data, tags)

    for index, element in enumerate(valid_data):
        if t[index//100] == clf.predict([element]):
            success += 1

    success_rate = success/valid_data_length
    k_neighbors.append(k)
    success_rates.append(success_rate)

    if success_rate == 0.9825:
        print(k)

    if max_rate < success_rate:
        max_rate = success_rate
        max_k = k

print("success rate of", max_k, "neighbors:", max_rate)
