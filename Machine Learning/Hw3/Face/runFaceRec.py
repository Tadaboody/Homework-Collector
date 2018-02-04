# function runFaceRec()
# load FaceData;
# FisherFaces(trainData);
# suc=recogTest(trainData, testData);

from scipy.io import loadmat
def runFaceRec():
    FaceData = loadmat("FaceData.mat")
    train_data = FaceData["trainData"]
    test_data = FaceData["testData"]
    suc = recogTest(train_data, test_data)