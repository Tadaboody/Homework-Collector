from scipy.io import loadmat
from sklearn.neighbors import KNeighborsClassifier

# tags for the data points
tags = list()
t = ['a', 'b', 'c', 'd']

for i in range(len(t)):
    for j in range(500):
        tags.append(t[i])


def find_optimal_neighbors(train_data, valid_data):

    k_neighbors = list()
    success_rates = list()
    max_k = 0
    max_rate = 0
    # optimization (?)
    valid_data_length = len(valid_data)

    # find best amount of neighbors
    for k in range(1, 150):
        success = 0
        clf = KNeighborsClassifier(n_neighbors=k)
        clf.fit(train_data, tags)
        # for every right prediction count a success
        for index, element in enumerate(valid_data):
            if t[index//100] == clf.predict([element]):
                success += 1
        # calculate success rate for this k
        success_rate = success/valid_data_length
        k_neighbors.append(k)
        success_rates.append(success_rate)
        # check if the current k is has the best success rate yet
        if max_rate <= success_rate:
            max_rate = success_rate
            max_k = k
    print("success rate of", max_k, "neighbors:", max_rate)
    return max_k, k_neighbors, success_rates


def main():
    mats = loadmat('ABCD_data.mat')
    train_data = mats['train_data']
    valid_data = mats['valid_data']
    test_data = mats['test_data']
    opt_k, k_neighbors, success_rates = find_optimal_neighbors(train_data, valid_data)

    t = ['a', 'b', 'c', 'd']
    test_successful_classifications = 0
    for index, element in enumerate(test_data):
        clf = KNeighborsClassifier(n_neighbors=opt_k)
        clf.fit(train_data, tags)
        if clf.predict([element]) == t[index//100]:
            test_successful_classifications += 1

    opt_success_rate = test_successful_classifications/len(test_data)
    print("success rate of opt k for test data:", opt_success_rate)


if __name__ == "__main__":
    main()
