
# coding: utf-8

# In[3]:


from scipy.io import loadmat
import numpy as np
import scipy
from sklearn.neighbors import NearestNeighbors
from os.path import join,dirname
face_data = loadmat(join(dirname(__file__),"FaceData.mat"));
train_data = face_data["trainData"];
T = train_data
N,d = T.shape # number of samples,dimension of sample
c = N//2 #number of classes (people)
def normalize(T):
    global mean_pic
    T = T.astype(float)
    for index,sample in enumerate(T):
        sample = np.true_divide(sample,sample.std())
        sample = np.subtract(sample,sample.mean(0))
        assert round(sample.mean(0),2) == 0 #rounding errors may occur
        assert round(sample.std(0),2) == 1
        sample -= mean_pic
        T[index] = sample
    if mean_pic is 0:
        mean_pic = T.mean(0)
        return normalize(T)
    return T

mean_pic = 0
T = train_data
T = normalize(T)
A = np.column_stack(T)


# In[18]:


S_prime = A.T @ A


def k_best_eig_values_vectors(M,k,B=None):
    a = scipy.linalg.eigh(M,b=B,eigvals_only=True)
    return scipy.linalg.eigh(M,b=B,eigvals=(len(a)-k,len(a)-1))


best_eig_values,best_eig_vectors = k_best_eig_values_vectors(S_prime,N-c)
V = best_eig_vectors
Z = np.diag(best_eig_values)
W_pca = A @ V @ Z

mean_pic =0
proj_T = np.array(T) @ W_pca
proj_T = np.array([face.reshape(len(face),1) for face in proj_T])
face_pairs = np.array([(proj_T[i],proj_T[i+1]) for i,t in enumerate(proj_T[:-1]) if i%2 == 0])
mu = np.array([np.mean(pair,0) for pair in face_pairs])


def S_i(i):
    cols = np.array([(face - mu[i]).reshape(len(face),1) for face in face_pairs[i]])
    return sum(col @ col.T for col in cols)


S_w = sum(np.array([S_i(i) for i in range(c)]))
S_b = sum(np.array([2*(mu_i - mean_pic)@(mu_i - mean_pic).T for mu_i in mu]))

best_eig_values,best_eig_vectors = k_best_eig_values_vectors(S_w,c-1,S_b)

W_mda = np.column_stack(best_eig_vectors)



W_opt = W_mda @ W_pca.T


train_data = face_data["trainData"]
test_data = face_data["testData"]
face_pairs = np.array([(train_data[i],train_data[i+1]) for i,t in enumerate(train_data[:-1]) if i%2 == 0])
models = np.array([np.mean(pair,0) for pair in face_pairs])
proj_models = np.array([W_opt @ model for model in models])




#classification
mean_pic = 0
normalize(test_data)
proj_test = np.array([W_opt @ test for test in test_data])




nbrs = NearestNeighbors(n_neighbors=1).fit(proj_models)
distances , indices = nbrs.kneighbors(proj_test)




correct = [index for index,img in enumerate(proj_test) if index == indices[index]]


# In[79]:


suc = len(correct)/len(proj_test)


# In[80]:


print(suc)

