##############################################################################
# Makefile for ExpressionParser
#
# Author:          Kim Walisch
# Contact:         kim.walisch@gmail.com
# Created:         4 February 2011
# Last modified:   4 February 2011
#
# Project home:    http://expressionparser.googlecode.com
##############################################################################

CXX = g++

all: expr expr_correctness expr_benchmark

# expr : default program, evaluates arithmetic expressions passed as
#        command-line arguments and prints the result.
expr :
	$(CXX) main.cpp -o expr

# expr_correctness : tests ExpressionParser.h for correctness.
expr_correctness :
	$(CXX) -DEXPRESSIONPARSER_CORRECTNESS main.cpp -o expr_correctness

# expr_benchmark : tool used to benchmark ExpressionParser.
expr_benchmark :
	$(CXX) -DBENCHMARK_EXPRESSIONPARSER -DNDEBUG -O2 -fopenmp main.cpp -o expr_benchmark

.PHONY: clean
clean:
	rm expr
	rm expr_correctness
	rm expr_benchmark
