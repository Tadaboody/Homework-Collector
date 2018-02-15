from scipy.io import loadmat
import numpy as np
from sklearn import svm
from sklearn.metrics import accuracy_score
from sklearn.model_selection import KFold


def rbf_kernal(train_x, train_y):
    rbf_clf = svm.SVC(kernel='rbf')
    rbf_clf.fit(train_x, train_y)
    return accuracy_score(np.array(train_y), np.array(rbf_clf.predict(train_x)))


def linear_kf(train_x, train_y):
    k = 5
    kf_score = 0
    kf = KFold(n_splits=k)
    for train, test in kf.split(train_x):
        lin_clf = svm.LinearSVC()
        train_data = [train_x[a] for a in train]
        train_labels = [train_y[a] for a in train]
        test_data = [train_x[a] for a in test]
        test_labels = [train_y[a] for a in test]
        lin_clf.fit(train_data, train_labels)
        linear_class_score = accuracy_score(np.array(test_labels), np.array(lin_clf.predict(test_data)))
        kf_score += linear_class_score
    return kf_score/k


def main():
    mat = loadmat('mnist_svm.mat')
    train_x = [mat[list(mat.keys())[3]]][0]
    train_y = [a[0] for a in mat[list(mat.keys())[4]]]

    linear_score = linear_kf(train_x, train_y)
    rbf_kernal_score = rbf_kernal(train_x, train_y)

    print("linear score:", linear_score)
    print("rbf score:", rbf_kernal_score)

if __name__ == '__main__':
    main()
