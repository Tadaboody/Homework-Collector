from math import exp
from scipy.io import loadmat
from scipy.linalg import norm
import numpy as np


class PNN:
    def __init__(self, categories):
        self.data = {category: list() for category in categories}
        self.window_width = 0

    def window(self, a, b):
        return exp(-norm(a - b)**2 / (2*self.window_width**2))

    def fit(self, t_data, window_width):
        self.window_width = window_width
        for category in t_data.keys():
            self.data[category] = t_data[category]

    def vote(self, x, category):
        # average of window functions from given class
        layer_2 = [self.window(x, x_i) for x_i in self.data[category]]
        return sum(layer_2)/len(layer_2)

    def classify(self, x):
        votes = [(category, self.vote(x, category)) for category in self.data.keys()]
        return max(votes, key=lambda y: y[1])[0]


def main():
    mat = loadmat("dataAB.mat")
    a_train, b_train, a_val, b_val, a_test, b_test = load_data(mat, 'train_dataA', 'train_dataB', 'valid_dataA', 'valid_dataB', 'test_dataA', 'test_dataB')

    clf = PNN(['a', 'b'])
    clf.fit({'a': a_train, 'b': b_train}, 1)

    opt_window, win_array, success_array = find_optimal_window_size(clf, a_val, b_val)
    print(win_array)
    print(success_array)


def find_optimal_window_size(clf, a_data, b_data):
    window_size = 13
    success_rate = -1
    re_windows = list()
    re_rates = list()
    while True:
        rate = classify_data(clf, window_size, a_data, b_data)
        re_windows.append(window_size)
        re_rates.append(rate)
        if round_digits(rate, 3) == round_digits(success_rate, 3):
            return window_size, re_windows, re_rates
        if round_digits(rate, 3) < round_digits(success_rate, 3):
            window_size *= 1.5
        if round_digits(rate, 3) > round_digits(success_rate, 3):
            success_rate = rate
            window_size *= 0.5


def round_digits(number, digits):
    exponent = pow(10, digits)
    result = int(number * exponent)
    return result / exponent


def load_data(mat, *data_list):
    return [mat[data] for data in data_list]


def classify_data(clf, window_size, *data_list):
    clf.window_width = window_size
    correct_classifications = 0
    total_classifications = 0
    for i, data in enumerate(data_list):
        correct = [element for element in data if clf.classify(element) == list(clf.data.keys())[i]]
        total_classifications += len(data)
        correct_classifications += len(correct)
    success_val = 100*correct_classifications/total_classifications
    return success_val


if __name__ == "__main__":
    main()
