#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int width = 100;

int height = 200;

char* name = "AreaCalculator";

int is_active = 1;

int my_array[3] = {10, 20, 30};

// Function to calculate area

int calculate_area(int w, int h) {
	int area;
	area = (w * h);
	printf("Calculating for: %s\n", name);
	printf("Width: %d, Height: %d, Area: %d\n", w, h, area);
	return area;
}

int main() {
	int result;
	printf("Program: %s starting...\n", name);
	if ((is_active == 1)) {
		result = calculate_area(width, height);
		printf("Final Result: %d\n", result);
	}
	else {
		printf("Calculator is inactive.\n");
	}
	int i;
	for (i = 0; (i < 3); i = (i + 1)) {
		// B-Minor for loop, C-like
		printf("Array val: %d\n", my_array[i]);
	}
	int power_val;
	power_val = pow(2, 8);
	// 2 to the power of 8
	printf("2^8 is: %d\n", power_val);
	return 0;
}

