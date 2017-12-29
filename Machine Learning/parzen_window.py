from math import exp
from scipy.io import loadmat
from scipy.linalg import norm
import numpy as np

class PNN:
    def __init__(self, catagories):
       self.data = {catagory: list() for catagory in catagories}
       self.window_width = 0

    def window(self,a, b):
        return exp(-norm(a - b)**2 /(2*self.window_width**2))

    def train(self,t_data, window_width):
        self.window_width = window_width
        for catagory in t_data.keys():
            self.data[catagory] = t_data[catagory]

    def vote(self,x, catagory):
        #average of window functions from given class
        layer_2 = [self.window(x,x_i) for x_i in self.data[catagory]]
        return sum(layer_2)/len(layer_2)

    def classify(self,x):
        votes = [(catagory, self.vote(x, catagory)) for catagory in self.data.keys()]
        return max(votes,key=lambda x:x[1])[0]

mat = loadmat("dataAB.mat")
a_train = mat["train_dataA"]
b_train = mat["train_dataB"]
clf = PNN(['a','b'])
clf.train({'a':a_train,'b':b_train},1)
a_val = mat['valid_dataA']
b_val = mat['valid_dataB']
correct_a = [a for a in a_val if clf.classify(a) == 'a']
correct_b = [b for b in b_val if clf.classify(b) == 'b']
success_val = (len(correct_a)+len(correct_b))/(len(a_val)+len(b_val))
print("success rate = " + str(success_val*100) + '%')
a_test = mat['test_dataA']
b_test = mat['test_dataB']
correct_a = [a for a in a_test if clf.classify(a) == 'a']
correct_b = [b for b in b_test if clf.classify(b) == 'b']
success_val = (len(correct_a)+len(correct_b))/(len(a_test)+len(b_test))
print("success rate = " + str(success_val*100) + '%')