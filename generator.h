#ifndef GENERATOR_H
#define GENERATOR_H
#include "header.h"
#include "globel.c"

void generate_csv_file(int generator_id);

void *file_generator(void *arg);
float get_random_value(float min_value, float max_value);


#endif 
