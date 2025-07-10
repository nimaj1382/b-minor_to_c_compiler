width: integer = 100;
height: integer = 200;
name: string = "AreaCalculator";
is_active: boolean = true;

// B-Minor code example for a simple area calculator with array and power operation

/* multi-line comment
   This is a simple program that calculates the area of a rectangle
   and demonstrates the use of arrays and power operations.
*/

my_array: array [3] integer = {10, 20, 30};

calculate_area: function integer (w: integer, h: integer) = {
    area: integer;
    area = w * h;
    print "Calculating for: ", name, "\n";
    print "Width: ", w, ", Height: ", h, ", Area: ", area, "\n";
    return area;
}

main: function integer () = {
    result: integer;
    print "Program: ", name, " starting...\n";
    if (is_active == true) {
        result = calculate_area(width, height);
        print "Final Result: ", result, "\n";
    } else {
        print "Calculator is inactive.\n";
    }

    i: integer;
    for(i=0; i<3; i=i+1) { // B-Minor for loop, C-like
        print "Array val: ", my_array[i], "\n";
    }

    power_val: integer;
    power_val = 2 ^ 8; // 2 to the power of 8
    print "2^8 is: ", power_val, "\n";

    return 0;
}
