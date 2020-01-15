# An object-oriented way of performing textual regression analysis.
# In the context of this course, it was used to predict the price of a
# taco or burrito based on its description, but can be generalized to any
# case of predicting a continuous attribute based on text data.
#
# Written by Graeme Prendergast as part of a term project for SENG 474, Fall 2019

import re

import pandas as pd
import numpy as np


class TextRegression():
	"""An object for building, training, and testing text regression models."""
	def __init__(self, stopwords=None, vsize=200):
		if stopwords is not None:
			self.stopwords = stopwords
		else:
			self.stopwords = {}

		self.w = None
		self.rmse = None
		self.vsize = vsize
		self.lookup = {}
		self.vocabulary = set()


	def fit(self, text, y):
		"""Fit a textual regression model to the given data."""
		split_regex = r',|\.| |\(|\)'
		word_freqs = text.str.split(split_regex, expand=True).stack().value_counts()

		filtered_freqs = [(index.lower(), count)
			   			   for index,count in word_freqs.items()
			   			   if index.lower() not in self.stopwords]

		load = 0
		for pair in filtered_freqs:
			if load >= self.vsize:
				break

			word = pair[0]
			if word not in self.vocabulary:
				self.vocabulary.add(word)
				self.lookup[word] = load
				load += 1

		words_matrix = []
		for doc in text.str.split(split_regex):
			words = np.zeros(self.vsize)
			for word in doc:
				if word in self.vocabulary:
					words[self.lookup[word]] = 1

			words_matrix.append(words)

		x = np.array(words_matrix)
		y = y.values

		ls_result = np.linalg.lstsq(x, y, rcond=None)
		self.w = ls_result[0]
		self.rmse = np.sqrt(ls_result[1] / x.shape[0])[0]


	def predict(self, doc, vsize=200):
		"""Predict the y-value for a given text."""
		split_regex = r',|\.| |\(|\)'

		words = np.zeros(vsize)
		for word in re.split(split_regex, doc):
			if word in self.vocabulary:
				words[self.lookup[word]] = 1

		return np.dot(words, self.w)


	def print_word_weights(self):
		"""Print a sorted sequence of word-weight pairs after fitting."""
		word_weight_pairs = [(word, self.w[self.lookup[word]])
							  for word in self.vocabulary]

		word_weight_pairs.sort(key=lambda x: x[1])

		for word, weight in word_weight_pairs:
			print(f"{word}: {weight}")