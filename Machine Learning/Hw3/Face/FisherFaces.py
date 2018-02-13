
# coding: utf-8


from scipy.io import loadmat
import numpy as np
import scipy
mean_pic = 0


def normalize(T):
        global mean_pic
        T = T.astype(float)
        for index, sample in enumerate(T):
            sample = np.true_divide(sample, sample.std())
            sample = np.subtract(sample, sample.mean(0))
            assert round(sample.mean(0), 2) == 0  # rounding errors may occur
            assert round(sample.std(0), 2) == 1
            sample -= mean_pic
            T[index] = sample
        if mean_pic is 0:
            mean_pic = T.mean(0)
            return normalize(T)
        return T


def FisherFaces(T):
    N, d = T.shape  # number of samples,dimension of sample
    c = N//2  # number of classes (people)
    T = train_data
    T = normalize(T)
    A = T.T  # T-T
    At = A.T  # = train_data

    S_prime = At @ A

    def k_best_eig_values_vectors(M, k, B=None):
        a = scipy.linalg.eig(M, b=B)
        eig_value_vector_pairs = [(value, vector)
                                  for value, vector in zip(a[0], a[1])]
        best_eig_value_vectors = sorted(
            eig_value_vector_pairs, key=lambda x: x[0], reverse=True)[:k]
        best_eig_values = np.array([pair[0]
                                    for pair in best_eig_value_vectors])
        best_eig_vectors = np.array([pair[1]
                                     for pair in best_eig_value_vectors])
        return best_eig_values, best_eig_vectors

    best_eig_values, best_eig_vectors = k_best_eig_values_vectors(S_prime, N-c)

    V = best_eig_vectors.T
    Z = np.diag(best_eig_values)

    W_pca = A @ V @ Z

    proj_T = np.array([W_pca.T@t for t in T])
    mean_pic = proj_T.mean(0)
    face_pairs = np.array([(proj_T[i], proj_T[i+1])
                           for i, t in enumerate(proj_T[:-1]) if i % 2 == 0])
    mu = np.array([np.mean(pair, 0) for pair in face_pairs])

    def S_i(i):
        return sum(np.array([np.array([(face - mu[i])]).T @ np.array([(face - mu[i])]) for face in face_pairs[i]]))
    S_w = sum(np.array([S_i(i) for i in range(c)]))
    S_b = sum(np.array([2*np.array([(mu_i - mean_pic)]).T@
                        np.array([(mu_i - mean_pic)]) for mu_i in mu]))

    best_eig_values, best_eig_vectors = k_best_eig_values_vectors(
        S_w, c-1, S_b)

    W_mda = best_eig_vectors

    W_opt = W_mda @ W_pca.T

    return W_opt


if __name__ == '__main__':
    face_data = loadmat("FaceData.mat")
    train_data = face_data["trainData"]
    print(FisherFaces(train_data))
