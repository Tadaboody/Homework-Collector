from scipy.io import loadmat
import numpy as np
import scipy

from os.path import join,dirname
face_data = loadmat(join(dirname(__file__),"FaceData.mat"));
train_data = np.array(face_data["trainData"]);
def FisherFaces(train_data):
    T = np.array(train_data)
    N,d = T.shape # number of samples,dimension of sample
    c = N//2 #number of classes (people)
    class Normalizer:
        def __init__(self,T):
            self.training_mean = T.mean(0)
        
        @staticmethod
        def scale(vec):
            return (vec - vec.mean()) / vec.std()

        def normalize(self,T):
            global mean_pic
            T = T.astype(float)
            for index,sample in enumerate(T):
                T[index] = self.scale(sample - self.training_mean)
            return T

    norm = Normalizer(T)
    T = norm.normalize(T)
    A = np.column_stack(T)


    S_prime = A.T @ A


    def k_best_eig_values_vectors(M,k,B=None):
        a = scipy.linalg.eigh(M,b=B, eigvals_only=True)
        return scipy.linalg.eigh(M,b=B, eigvals=(len(a) - k, len(a)-1))


    best_eig_values,best_eig_vectors = k_best_eig_values_vectors(S_prime,N-c)
    V = best_eig_vectors
    Z = np.diag(best_eig_values)
    W_pca = A @ V @ Z

    proj_T = np.array(T) @ W_pca
    proj_T = np.array([face for face in proj_T])
    face_pairs = np.array([(proj_T[i],proj_T[i+1]) for i,t in enumerate(proj_T[:-1]) if i%2 == 0])
    mu = np.array([0.5*(pair[0] + pair[1])for pair in face_pairs])
    mean_pic = sum([t for t in proj_T])/len(proj_T)

    def S_i(i):
        return sum(np.asmatrix(face -mu[i]).T @ np.asmatrix(face - mu[i]) for face in face_pairs[i])


    S_w = sum(np.array([S_i(i) for i in range(c)]))
    S_b = 2*sum(np.array([np.asmatrix((mu_i - mean_pic)).T @ np.asmatrix((mu_i - mean_pic)) for mu_i in mu]))

    best_eig_values,best_eig_vectors = k_best_eig_values_vectors(S_b,c,S_w)

    W_mda = np.column_stack(best_eig_vectors)



    W_opt = W_mda @ W_pca.T
    return W_opt,norm