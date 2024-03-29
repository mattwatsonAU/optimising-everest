
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <inttypes.h>

#include "matrix.h"

static uint32_t g_seed = 0;

static ssize_t g_width = 0;
static ssize_t g_height = 0;
static ssize_t g_elements = 0;

static ssize_t g_nthreads = 1;

////////////////////////////////
///     UTILITY FUNCTIONS    ///
////////////////////////////////

/**
 * Returns pseudorandom number determined by the seed
 */
uint32_t fast_rand(void) 
{

    g_seed = (214013 * g_seed + 2531011);
    return (g_seed >> 16) & 0x7FFF;
}

/**
 * Sets the seed used when generating pseudorandom numbers
 */
void set_seed(uint32_t seed) 
{

    g_seed = seed;
}

/**
 * Sets the number of threads available
 */
void set_nthreads(ssize_t count) 
{

    g_nthreads = count;
}

/**
 * Sets the dimensions of the matrix
 */
void set_dimensions(ssize_t order) 
{

    g_width = order;
    g_height = order;

    g_elements = g_width * g_height;
}

/**
 * Displays given matrix
 */
void display(const uint32_t* matrix) 
{

    for (ssize_t y = 0; y < g_height; y++) 
    {
        for (ssize_t x = 0; x < g_width; x++) 
        {
            if (x > 0) printf(" ");
            printf("%" PRIu32, matrix[y * g_width + x]);
        }

        printf("\n");
    }
}

/**
 * Displays given matrix row
 */
void display_row(const uint32_t* matrix, ssize_t row) 
{

    for (ssize_t x = 0; x < g_width; x++) 
    {
        if (x > 0) printf(" ");
        printf("%" PRIu32, matrix[row * g_width + x]);
    }

    printf("\n");
}

/**
 * Displays given matrix column
 */
void display_column(const uint32_t* matrix, ssize_t column) 
{

    for (ssize_t y = 0; y < g_height; y++) 
    {
        printf("%" PRIu32 "\n", matrix[y * g_width + column]);
    }
}

/**
 * Displays the value stored at the given element index
 */
void display_element(const uint32_t* matrix, ssize_t row, ssize_t column) 
{

    printf("%" PRIu32 "\n", matrix[row * g_width + column]);
}

////////////////////////////////
///   MATRIX INITALISATIONS  ///
////////////////////////////////

/**
 * Returns new matrix with all elements set to zero
 */
uint32_t* new_matrix(void) 
{

    return calloc(g_elements, sizeof(uint32_t));
}

/**
 * Returns new identity matrix
 */
uint32_t* identity_matrix(void) 
{

    uint32_t* matrix = new_matrix();

    for (ssize_t i = 0; i < g_width; i++) 
    {
        matrix[i * g_width + i] = 1;
    }

    return matrix;
}

/**
 * Returns new matrix with elements generated at random using given seed
 */
uint32_t* random_matrix(uint32_t seed) 
{

    uint32_t* matrix = new_matrix();

    set_seed(seed);

    for (ssize_t i = 0; i < g_elements; i++) 
    {
        matrix[i] = fast_rand();
    }

    return matrix;
}

/**
 * Returns new matrix with all elements set to given value
 */
uint32_t* uniform_matrix(uint32_t value) 
{

    uint32_t* matrix = new_matrix();

    for (ssize_t i = 0; i < g_elements; i++) 
    {
        matrix[i] = value;
    }

    return matrix;
}

/**
 * Returns new matrix with elements in sequence from given start and step
 */
uint32_t* sequence_matrix(uint32_t start, uint32_t step) 
{

    uint32_t* matrix = new_matrix();
    uint32_t current = start;

    for (ssize_t i = 0; i < g_elements; i++) 
    {
        matrix[i] = current;
        current += step;
    }

    return matrix;
}

////////////////////////////////
///     MATRIX OPERATIONS    ///
////////////////////////////////

/**
 * Returns new matrix with elements cloned from given matrix
 */
uint32_t* cloned(const uint32_t* matrix) 
{

    uint32_t* result = new_matrix();

    for (ssize_t i = 0; i < g_elements; i++) 
    {
        result[i] = matrix[i];
    }

    return result;
}

/**
 * Returns new matrix with elements ordered in reverse
 */
uint32_t* reversed(const uint32_t* matrix) 
{

    uint32_t* result = new_matrix();

    for (ssize_t i = 0; i < g_elements; i++) 
    {
        result[i] = matrix[g_elements - 1 - i];
    }

    return result;
}

/**
 * Returns new transposed matrix
 */
uint32_t* transposed(const uint32_t* matrix) 
{

    uint32_t* result = new_matrix();

    for (ssize_t y = 0; y < g_height; y++) 
    {
        for (ssize_t x = 0; x < g_width; x++) 
        {
            result[x * g_width + y] = matrix[y * g_width + x];
        }
    }

    return result;
}

/**
 * Returns new matrix with scalar added to each element
 */
uint32_t* scalar_add(const uint32_t* matrix, uint32_t scalar) 
{

    uint32_t* result = new_matrix();

    for (ssize_t c=0; c<g_elements; c++)
    {
        result[c] = matrix[c] + scalar;
    }

    return result;
}

/**
 * Returns new matrix with scalar multiplied to each element
 */
uint32_t* scalar_mul(const uint32_t* matrix, uint32_t scalar) 
{

    uint32_t* result = new_matrix();

    for (ssize_t c=0; c<g_elements; c++)
    {
        result[c] = matrix[c] * scalar;
    }

    return result;
}

/**
 * Returns new matrix with elements added at the same index
 */
uint32_t* matrix_add(const uint32_t* matrix_a, const uint32_t* matrix_b) 
{

    uint32_t* result = new_matrix();

    for (ssize_t c=0; c<g_elements; c++)
    {
        result[c] = matrix_a[c] + matrix_b[c];
    }

    return result;
}

/**
 * Returns new matrix, multiplying the two matrices together
 */
uint32_t* matrix_mul(const uint32_t* matrix_a, const uint32_t* matrix_b) 
{

    uint32_t* result = new_matrix();

    for (ssize_t c=0; c<g_elements; c++)
    {
        for (ssize_t first_row = (c/g_width) * g_width; first_row<g_elements;)
        {
            for(ssize_t second_column = (c%g_width); second_column<g_elements; second_column += g_width)
            {
                result[c] += (matrix_a[first_row++] * matrix_b[second_column]);
            }
            break;
        }
    }

    return result;
}

/**
 * Returns new matrix, powering the matrix to the exponent
 */
uint32_t* matrix_pow(const uint32_t* matrix, uint32_t exponent) 
{

    uint32_t* result = cloned(matrix);

    if(exponent==0)
    {
        result = identity_matrix();
    }
    
    else
    {
        for (int c=0; c<exponent-1 && exponent!=0; c++)
        {
            result = matrix_mul(result, matrix);
        }
    }

    

    return result;
}

////////////////////////////////
///       COMPUTATIONS       ///
////////////////////////////////

/**
 * Returns the sum of all elements
 */
uint32_t get_sum(const uint32_t* matrix) 
{
uint32_t sum_all_elements=0;

    for (ssize_t c=0; c<g_elements; c++)
    {
        sum_all_elements += matrix[c];
    }

    return sum_all_elements;
}

/**
 * Returns the trace of the matrix
 */
uint32_t get_trace(const uint32_t* matrix) 
{

    uint32_t matrix_trace=0;

    for (ssize_t c=0; c<g_width; c++)
    {
        matrix_trace += matrix[(c*g_width) + c];
    }

    return matrix_trace;
}

/**
 * Returns the smallest value in the matrix
 */
uint32_t get_minimum(const uint32_t* matrix) 
{
    uint32_t matrix_smallest_val = matrix[0];

    for (ssize_t c=0; c<g_elements; c++)
    {
        if(matrix[c] < matrix_smallest_val)
        {
            matrix_smallest_val = matrix[c];
        }
    }

    return matrix_smallest_val;
}

/**
 * Returns the largest value in the matrix
 */
uint32_t get_maximum(const uint32_t* matrix) 
{
    uint32_t matrix_largest_val =  matrix[0];

    for (ssize_t c=0; c<g_elements; c++)
    {
        if(matrix[c] > matrix_largest_val)
        {
            matrix_largest_val = matrix[c];
        }
    }

    return matrix_largest_val;
}

/**
 * Returns the frequency of the value in the matrix
 */
uint32_t get_frequency(const uint32_t* matrix, uint32_t value) 
{

    uint32_t freq_counter=0;

    for(ssize_t c=0; c<g_elements; c++)
    {
        if(matrix[c] == value)
        {
            freq_counter++;
        }
    }

    return freq_counter;
}
