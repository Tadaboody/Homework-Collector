
from scipy.io import loadmat
from numpy import array, identity, linalg
from random import seed, shuffle


def prep(data):
    X = array(data,"float64")
    std =  X.std(0)
    std = array([s if not s == 0 else 1 for s in std])#Avoid zero division
    X /= std
    X -= X.mean(0)
    y = array([[x[0] for x in X]]).T
    X = array([-x for x in X])
    for x in X:
        x[0] = 1
    return X, y


def norm_train(X, y, gamma):
    return linalg.inv((X.T @ X)) @ X.T @ y


def ridge_train(X, y, gamma):
    return linalg.inv(X.T @ X + gamma*identity(X.shape[1])) @ X.T @ y


def success(size, data, regression, gamma=0):
    seed(1)  # chosen randomly,constant for reproducibility 
    data = array(data)  # copy
    shuffle(data)
    train_data = data[:size]
    test_data = data[size:]
    test_X, test_y = prep(test_data)
    train_X, train_y = prep(train_data)
    try:
        w = regression(train_X, train_y, gamma)
    except linalg.LinAlgError:  # some sizes yield matrixes that are singular
        return None
    pred_x = [data @ w for data in train_X]

    def mse(X, y):
        return sum([(data @ w - output)**2 for data, output in zip(X, y)])
    suc_train = float(mse(train_X, train_y))
    suc_test = float(mse(test_X, test_y))
    return suc_train/100, suc_test/100

def success_for_normal_regression(size,data):
    return success(size,data,norm_train)

def success_for_ridge_regression(size,data,gamma):
    return success(size,data,ridge_train,gamma=gamma)