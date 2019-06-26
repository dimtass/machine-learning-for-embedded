/*
 * neural_network.c
 *
 */

#include <stdio.h>
#include <math.h>
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
    return result;
}

double sigmoid(double x)
{
  double result = 1 / (1+exp(-x));
  return result;
}
