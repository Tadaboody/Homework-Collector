from scipy.io import loadmat
from sklearn.neighbors import NearestNeighbors
from FisherFaces import FisherFaces
import numpy as np
from sklearn.neighbors import KNeighborsClassifier


def recogTest(train_data, test_data):
    W_opt, norm = FisherFaces(train_data)
    train_data = norm.normalize(train_data)
    test_data = norm.normalize(test_data)
    face_pairs = np.array([(train_data[i], train_data[i+1])
                           for i, t in enumerate(train_data[:-1]) if i % 2 == 0])
    models = np.array([np.mean(pair, 0) for pair in face_pairs])
    proj_models = models @ W_opt.T
    proj_test = test_data @ W_opt.T
    nbrs = KNeighborsClassifier(n_neighbors=1).fit(
        proj_models, range(len(proj_models)))
    return (nbrs.score(proj_test, range(len(proj_test))))
