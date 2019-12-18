# Assignment 1: Ray Tracing

## 1.1

Mimic the original part 2 function to use the geometry solution to create a simple diffuse model. The way that I deal with intersecting with multiple spheres is calculate and record each intersection point from the start and if the next intersection point is nearer to the camera, substitute the old intersection point with the new intersection point.

![Part1.1](build/part1_1.png)

## 1.2

Similar to 1.1, but we have to make Matrix C into 3 Matrices to record the RGB color, and add an additional light source. Another difference is that we need to use the specular model which requires  calculating the view vector.

![Part1.2](build/part1_2.png)

## 1.3

Similarly, I use the geometry solution. The camera is at (0,0,2) pointing at (0,0,-1).

![Part1.3 single sphere](build/part1_3_single.png)

![Part1.3 multiple spheres](build/part1_3_multiple.png)

## 1.4

It is really hard to use the geometry solution to calculate the intersection point bewteen the triangle and the ray, so I decided to use the algebra method (Cramer’s rule to calculate the intersection point). The camera settings are the same as the 1.3, and in order to show the bunny and bumpy cube more clearly I remove the spheres in the 1.3.  
Because the scale of the bunny and bumpy cube are really different, I have to rescale them in order to show them completely and clearly.

![Part1.4 Bunny and Bumpy Cube](build/part1_4.png)