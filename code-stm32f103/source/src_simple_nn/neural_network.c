/*
 * neural_network.c
 *
 */

#include <stdio.h>
#include <math.h>
#include "debug_trace.h"
#include "neural_network.h"


/**
 * @brief Calculates the dot product of two double arrays
 * @param[in] double The first double array
 * @param[in] double The second double array
 * @param[in] int Number of elements in the array
 * @return double The result
 */
double dot(double v[], double u[], int n)
{
    double result = 0.0;
    for (int i = 0; i < n; i++)
        result += v[i]*u[i];

    // TRACEL(TRACE_LEVEL_NN, ("dot = %f\n", result));
    return result;
}

double sigmoid(double x)
{
	double result = 1 / (1+exp(-x));
	// TRACEL(TRACE_LEVEL_NN, ("sigmoid(%f) = %f\n", x, result));
	return result;
}

double nn_predict(double * input, double * weights, int n)
{
	double result = sigmoid(dot(input, weights, n));

	// TRACEL(TRACE_LEVEL_NN, ("predict = %f\n", result));
	return(result);
}
