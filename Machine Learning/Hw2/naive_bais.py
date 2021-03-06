import os
import math
import json


def read_text(text_path):
    texAll = list()
    libAll = list()
    Voc = set()
    cat = set()
    with open(text_path) as text:
        for line in text:
            line = line.split()
            catagory = line[0]
            data = line[1:]
            cat |= set([catagory])
            Voc |= set(data)
            texAll.append(data)
            libAll.append(catagory)
    return texAll, libAll, Voc, cat


def learn_NB_text(texAll, libAll, Voc, cat):
    P = {catagory: 0 for catagory in cat}
    Pw = {word: {catagory: 0 for catagory in cat} for word in Voc}
    for catagory in cat:
        docs = [doc for doc, label in zip(texAll, libAll) if label == catagory]
        text = list()
        P[catagory] = len(docs) / len(texAll)
        for doc in docs:
            text += doc
        n = len(text)
        for phrase in Voc:
            nk = len([word for word in text if word == phrase])
            Pw[phrase][catagory] = (nk + 1) / (n + len(Voc))
    return Pw, P


def ClassifyNB_doc(Pw, P, text, Voc, cat):
    max_label = ""
    max_value = float("-inf")
    for catagory in cat:
        value = math.log(P[catagory])
        for word in text:
            if word in Voc:
                value += math.log(Pw[word][catagory])
        if value > max_value:
            max_value = value
            max_label = catagory
    return max_label


def ClassifyNB_text(Pw, P, texAll, libAll, Voc, cat):
    classifications = [ClassifyNB_doc(
        Pw, P, text, Voc, cat) for text in texAll]
    correct_classifications = [classification for classification, label in zip(
        classifications, libAll) if label == classification]
    success_rate = len(correct_classifications) / len(classifications)
    return int(success_rate*100)


if __name__ == '__main__':
    texAll, libAll, Voc, cat = read_text(
        os.path.join("textClassif", "r8-train-stemmed.txt"))
    Pw, P = learn_NB_text(texAll, libAll, Voc, cat)
    texAll, libAll, Voc2, cat = read_text(
        os.path.join("textClassif", "r8-test-stemmed.txt"))
    success_rate = ClassifyNB_text(Pw, P, texAll, libAll, Voc, cat)
    print("success_rate = " + str(success_rate) + "%")
