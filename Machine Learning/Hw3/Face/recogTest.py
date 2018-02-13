# function suc=recogTest(trainData, testData)
# % trainData is a Nxd matrix of training data for N=100 images (2 per person). d is the dimension of the features, representing an image.
# % testData testData is a nxd matrix of test data for n=50 images (1 per person). d is the dimension of the features, representing an image.
# load FisherSpace.mat;

# %suc is the number of correct classifications (when the index of the test image is equal to the predicted index)
# suc=suc/size(testData,1);
# fprintf('The recognition rate is %2.2f\n',suc);
N=100
from scipy.io import loadmat
from sklearn.neighbors import NearestNeighbors
from FisherFaces import FisherFaces
def recogTest(train_data,test_data):
    fisher_space = loadmat("FisherSpace.mat")
    train_data = fisher_space["trainData"]
    test_data = fisher_space["testData"]
    W_opt = FisherFaces(train_data)
    face_pairs = np.array([(train_data[i],train_data[i+1]) for i,t in enumerate(train_data[:-1]) if i%2 == 0])
    models = np.array([np.mean(pair,0) for pair in face_pairs])
    proj_models = [W_opt.T @ model for model in proj_models]
    suc = suc/len(filter(test_data)) 
    return suc