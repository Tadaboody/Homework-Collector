from scipy.io import loadmat
from recogTest import recogTest
from os.path import dirname, join
from recogTest import recogTest
import numpy as np

def runFaceRec():
    face_data = loadmat(join(dirname(__file__), "FaceData.mat"))
    train_data = np.array(face_data["trainData"])
    test_data = face_data['testData']
    suc = recogTest(train_data, test_data)
    return suc


def main():
    print(runFaceRec())


if __name__ == '__main__':
    main()
