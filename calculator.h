#ifndef CALCULATOR_H
#define CALCULATOR_H


int init_calculator(struct MEMORYcalculator *calculator, int max_columns) ;
void calculate_csv_file(const char *filename, int calculator_id) ;
void *calculator_thread(void *arg) ;
#endif 
