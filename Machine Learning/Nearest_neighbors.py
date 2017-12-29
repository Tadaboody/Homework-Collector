from scipy.io import loadmat
from sklearn.neighbors import NearestNeighbors
from math import sqrt
mats = loadmat('ABCD_data.mat')
test_data = mats['test_data']
train_data = mats['train_data']
valid_data = mats['valid_data']
success_rates = list()
n = len(train_data)
for k in range(20,int(sqrt(n))):
    success = 0
    clf = NearestNeighbors(n_neighbors = k).fit(train_data)
    distances,indices = clf.kneighbors(valid_data)
    for index,neighbors in enumerate(indices):
        k_i = 0
        for neighbor in neighbors:
            if neighbor//500 == index//100:
                k_i+=1
        if k_i >= k/4:
            success+=1
    success_rates.append((k,100*success/len(valid_data)))
print(max(success_rates,key=lambda x:x[1]))