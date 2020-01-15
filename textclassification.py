# An object-oriented implementation of a naive Bayes multinomial text classifier
# Written by Graeme Prendergast for SENG474 Fall 2019

import pandas as pd
import numpy as np

import math
import sys
import os


class MultinomialBayesClassifier:
    """A class for creating, training, and testing multinomial Bayes' classifiers"""
    def __init__(self, stopwords=None):
        if stopwords is not None:
            self.stop_words = set(stopwords)
        else:
            self.stop_words = {}


    def train(self, classes, data):
        """Train the classifier with a given data set."""
        self.V = self.extract_vocabulary(data)
        self.N = data['text'].count()

        self.word_counts = [{} for c in classes]
        self.word_probs = [{} for c in classes]
        self.prior = [0 for c in classes]

        for c in classes:
            class_data = data[data['class'] == c]
            self.prior[c] = class_data['text'].count() / self.N

            class_word_counts = {}
            for doc in class_data['text']:
                for term in doc.strip().split(' '):
                    if term not in self.stop_words:
                        if term in class_word_counts:
                            class_word_counts[term] += 1
                        else:
                            class_word_counts[term] = 1

            self.word_counts[c] = class_word_counts

            self.word_probs[c] = {
                term : self.word_counts[c][term] / len(self.word_counts[c])
                for term in self.word_counts[c]
            }


    def classify(self, classes, doc):
        """Classify a (single) given document."""
        score = {c : 0 for c in classes}
        for c in classes:
            score[c] = math.log(self.prior[c], 2)

            for term in doc.strip().split(' '):
                if term in self.V:
                    try:
                        score[c] += math.log(self.word_probs[c][term], 2)
                    except:
                        pass

        # Return the key(class) of the max value (p-score given doc)
        return 1 - max(score, key=score.get)