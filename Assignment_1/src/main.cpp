// C++ include
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <sstream>

// Image writing library
#define STB_IMAGE_WRITE_IMPLEMENTATION // Do not include this line twice in your project!
#include "stb_image_write.h"
#include "utils.h"
#include <Eigen/LU>
#include <Eigen/Geometry>

// Shortcut to avoid Eigen:: and std:: everywhere, DO NOT USE IN .h
using namespace std;
using namespace Eigen;

void part1()
{
    std::cout << "Part 1: Writing a grid png image" << std::endl;

    const std::string filename("part1.png");
    Eigen::MatrixXd M(800,800);

    // Draw a grid, each square has a side of e pixels
    const int e = 50;
    const double black = 0;
    const double white = 1;

    for (unsigned wi = 0; wi<M.cols();++wi)
        for (unsigned hi = 0; hi < M.rows(); ++hi)
            M(hi,wi) = (lround(wi / e) % 2) == (lround(hi / e) % 2) ? black : white;

    // Write it in a png image. Note that the alpha channel is reversed to make the white (color = 1) pixels transparent (alhpa = 0)
    write_matrix_to_png(M,M,M,1.0-M.array(),filename);
}

void part2()
{
    std::cout << "Part 2: Simple ray tracer, one sphere with orthographic projection" << std::endl;

    const std::string filename("part2.png");
    MatrixXd C = MatrixXd::Zero(800,800); // Store the color
    MatrixXd A = MatrixXd::Zero(800,800); // Store the alpha mask

    // The camera is orthographic, pointing in the direction -z and covering the unit square (-1,1) in x and y
    Vector3d origin(-1,1,1);
    Vector3d x_displacement(2.0/C.cols(),0,0);
    Vector3d y_displacement(0,-2.0/C.rows(),0);

    // Single light source
    const Vector3d light_position(-1,1,1);

    for (unsigned i=0;i<C.cols();i++)
    {
        for (unsigned j=0;j<C.rows();j++)
        {
            // Prepare the ray
            Vector3d ray_origin = origin + double(i)*x_displacement + double(j)*y_displacement;
            Vector3d ray_direction = RowVector3d(0,0,-1);

            // Intersect with the sphere
            // NOTE: this is a special case of a sphere centered in the origin and for orthographic rays aligned with the z axis
            Vector2d ray_on_xy(ray_origin(0),ray_origin(1));
            const double sphere_radius = 0.9;

            if (ray_on_xy.norm()<sphere_radius)
            {
                // The ray hit the sphere, compute the exact intersection point
                Vector3d ray_intersection(ray_on_xy(0),ray_on_xy(1),sqrt(sphere_radius*sphere_radius - ray_on_xy.squaredNorm()));

                // Compute normal at the intersection point
                Vector3d ray_normal = ray_intersection.normalized();

                // Simple diffuse model
                C(i,j) = (light_position-ray_intersection).normalized().transpose() * ray_normal;

                // Clamp to zero
                C(i,j) = max(C(i,j),0.);

                // Disable the alpha mask for this pixel
                A(i,j) = 1;
            }
        }
    }

    // Save to png
    write_matrix_to_png(C,C,C,A,filename);

}

void part1_1()
{
    std::cout << "Part 1_1: Rendering multiple spheres in general positions with orthographic projection" << std::endl;

    const std::string filename("part1_1.png");
    MatrixXd C = MatrixXd::Zero(800,800); // Store the color
    MatrixXd A = MatrixXd::Zero(800,800); // Store the alpha mask

    // The camera is orthographic, pointing in the direction -z and covering the unit square (-1,1) in x and y
    Vector3d origin(-1,1,1);
    Vector3d x_displacement(2.0/C.cols(),0,0);
    Vector3d y_displacement(0,-2.0/C.rows(),0);

    // Single light source
    const Vector3d light_position(-1,1,1);

    // Multiple Spheres (x,y,z,r)
    vector<Vector4d> spheres = {Vector4d(0.1,0.1,0.1,0.5), Vector4d(-0.2,0.1,0.2,0.3), Vector4d(0.3,-0.4,0.1,0.3)};

    for (unsigned i=0;i<C.cols();i++)
    {
        for (unsigned j=0;j<C.rows();j++)
        {
            // Prepare the ray
            Vector3d ray_origin = origin + double(i)*x_displacement + double(j)*y_displacement;
            Vector3d ray_direction = RowVector3d(0,0,-1);

            // Intersect with the sphere
            Vector2d ray_on_xy(ray_origin(0),ray_origin(1));

            double hit_sum = 0;
            for (int i = 0; i < spheres.size(); i++)
            {
                Vector2d sphere_origin_to_ray_xy = ray_on_xy - spheres[i].head<2>();
                double ray_to_sphere_xy = spheres[i](3) - sphere_origin_to_ray_xy.norm();
                hit_sum += max(ray_to_sphere_xy, 0.);
            }

            if (hit_sum > 0)
            {
                // The ray hit the sphere, compute the exact intersection point
                double ray_intersection_z = -1;
                int intersection_sphere_number = 0;

                for (int i = 0; i < spheres.size(); i++)
                {
                    Vector2d sphere_origin_to_ray_xy = ray_on_xy - spheres[i].head<2>();
                    double h_square = spheres[i](3)*spheres[i](3) - sphere_origin_to_ray_xy.squaredNorm();
                    if(h_square > 0)
                    {
                        if (ray_intersection_z < spheres[i](2) + sqrt(h_square))
                        {
                            ray_intersection_z = spheres[i](2) + sqrt(h_square);
                            intersection_sphere_number = i;
                        }
                    }
                }
                
                Vector3d ray_intersection(ray_on_xy(0),ray_on_xy(1), ray_intersection_z);

                // Compute normal at the intersection point
                Vector3d ray_normal = (ray_intersection - spheres[intersection_sphere_number].head<3>()).normalized();

                // Simple diffuse model
                C(i,j) = (light_position - ray_intersection).normalized().transpose() * ray_normal;

                // Clamp to zero
                C(i,j) = max(C(i,j),0.);

                // Disable the alpha mask for this pixel
                A(i,j) = 1;
            }
        }
    }

    // Save to png
    write_matrix_to_png(C,C,C,A,filename);

}

void part1_2()
{
    std::cout << "Part 1_2: Support ambient and specular lighting" << std::endl;

    const std::string filename("part1_2.png");
    MatrixXd R = MatrixXd::Zero(800,800); // Store the red color
    MatrixXd G = MatrixXd::Zero(800,800); // Store the green color
    MatrixXd B = MatrixXd::Zero(800,800); // Store the blue color
    MatrixXd A = MatrixXd::Zero(800,800); // Store the alpha mask

    // The camera is orthographic, pointing in the direction -z and covering the unit square (-1,1) in x and y
    Vector3d origin(-1,1,1);
    Vector3d x_displacement(2.0/R.cols(),0,0);
    Vector3d y_displacement(0,-2.0/R.rows(),0);

    // Two light sources
    const Vector3d light_position_1(-1,1,1);
    const Vector3d light_position_2(1,1,1);

    // Multiple Spheres (x,y,z,r), (R,G,B,M) (M=0 diffuse model, M=1 specular model)
    vector<Vector4d> spheres = {Vector4d(0.3,0.3,0.3,0.3), Vector4d(-0.5,-0.4,-0.7,0.4)};
    vector<Vector4d> spheres_color = {Vector4d(0.3,1.0,0.6,0.), Vector4d(0.3,0.1,0.9,1.)};

    for (unsigned i=0;i<R.cols();i++)
    {
        for (unsigned j=0;j<R.rows();j++)
        {
            // Prepare the ray
            Vector3d ray_origin = origin + double(i)*x_displacement + double(j)*y_displacement;
            Vector3d ray_direction = RowVector3d(0,0,-1);

            // Intersect with the sphere
            Vector2d ray_on_xy(ray_origin(0),ray_origin(1));

            double hit_sum = 0;
            for (int i = 0; i < spheres.size(); i++)
            {
                Vector2d sphere_origin_to_ray_xy = ray_on_xy - spheres[i].head<2>();
                double ray_to_sphere_xy = spheres[i](3) - sphere_origin_to_ray_xy.norm();
                hit_sum += max(ray_to_sphere_xy, 0.);
            }

            if (hit_sum > 0)
            {
                // The ray hit the sphere, compute the exact intersection point
                double ray_intersection_z = -1;
                int intersection_sphere_number = 0;

                for (int i = 0; i < spheres.size(); i++)
                {
                    Vector2d sphere_origin_to_ray_xy = ray_on_xy - spheres[i].head<2>();
                    double h_square = spheres[i](3)*spheres[i](3) - sphere_origin_to_ray_xy.squaredNorm();
                    if(h_square > 0)
                    {
                        if (ray_intersection_z < spheres[i](2) + sqrt(h_square))
                        {
                            ray_intersection_z = spheres[i](2) + sqrt(h_square);
                            intersection_sphere_number = i;
                        }
                    }
                }
                
                Vector3d ray_intersection(ray_on_xy(0),ray_on_xy(1), ray_intersection_z);

                // Compute normal at the intersection point
                Vector3d ray_normal = (ray_intersection - spheres[intersection_sphere_number].head<3>()).normalized();

                // Normalized view vector
                Vector3d view(0,0,1);

                // Compute normalized light vector
                Vector3d light_1 = (light_position_1 - ray_intersection).normalized();
                Vector3d light_2 = (light_position_2 - ray_intersection).normalized();

                // Compute normalized half angle vector
                Vector3d half_angle_1 = (view + light_1).normalized();
                Vector3d half_angle_2 = (view + light_2).normalized();

                double lightness = 0;               

                if (spheres_color[intersection_sphere_number](3) == 0) {
                    // Pure diffuse model
                    double lightness_1 = max(0.,double(light_1.transpose() * ray_normal));
                    double lightness_2 = max(0.,double(light_2.transpose() * ray_normal));
                    lightness = lightness_1 + lightness_2;
                } else if (spheres_color[intersection_sphere_number](3) == 1)
                {
                    // Specular shading
                    double lightness_1 = max(0.,double(light_1.transpose() * ray_normal)) + max(0.,pow(ray_normal.transpose() * half_angle_1, 100));
                    double lightness_2 = max(0.,double(light_2.transpose() * ray_normal)) + max(0.,pow(ray_normal.transpose() * half_angle_2, 100));
                    // Plus ambient light
                    lightness = lightness_1 + lightness_2 + 0.1;
                }

                R(i,j) = lightness * spheres_color[intersection_sphere_number](0);
                G(i,j) = lightness * spheres_color[intersection_sphere_number](1);
                B(i,j) = lightness * spheres_color[intersection_sphere_number](2);

                // Disable the alpha mask for this pixel
                A(i,j) = 1;
            }
        }
    }

    // Save to png
    write_matrix_to_png(R,G,B,A,filename);

}

void part1_3_single()
{
        std::cout << "Part 1_3_single: render 1.1 with perspective projection" << std::endl;

    const std::string filename("part1_3_single.png");
    MatrixXd C = MatrixXd::Zero(800,800); // Store the color
    MatrixXd A = MatrixXd::Zero(800,800); // Store the alpha mask

    // The camera is perspective at (0,0,2), pointing in the direction -z 
    // and covering the unit square (-1,1) in x and y at z = 1
    Vector3d origin(0,0,2);
    Vector3d direction = Vector3d(-1,1,1) - origin;
    Vector3d x_displacement(2.0/C.cols(),0,0);
    Vector3d y_displacement(0,-2.0/C.rows(),0);

    // Single light source
    const Vector3d light_position(-1,1,1);

    // Multiple Spheres (x,y,z,r)
    vector<Vector4d> spheres = {Vector4d(0.1,0.1,0.1,0.5), Vector4d(-0.2,0.1,0.2,0.3), Vector4d(0.3,-0.4,0.1,0.3)};

    for (unsigned i=0;i<C.cols();i++)
    {
        for (unsigned j=0;j<C.rows();j++)
        {
            // Prepare the ray
            Vector3d ray_origin = origin;
            Vector3d ray_direction = (direction + double(i)*x_displacement + double(j)*y_displacement).normalized();

            // Intersect with the sphere
            bool is_intersected = false;
            int intersection_sphere_number = 0;
            double nearest_intersection = 10;
            for (int index = 0; index < spheres.size(); index++)
            {
                Vector3d origin_to_sphere_center = spheres[index].head<3>() - ray_origin;
                double origin_to_perpendicular = ray_direction.dot(origin_to_sphere_center);
                double perpendicular_height = sqrt(origin_to_sphere_center.dot(origin_to_sphere_center) - origin_to_perpendicular * origin_to_perpendicular);
                if (perpendicular_height <= spheres[index](3))
                {
                    is_intersected = true;
                    double intersection_to_perpendicular = sqrt(spheres[index](3) * spheres[index](3) - perpendicular_height * perpendicular_height);
                    double origin_to_intersection_length = origin_to_perpendicular - intersection_to_perpendicular;
                    if (origin_to_intersection_length < nearest_intersection)
                    {
                        nearest_intersection = origin_to_intersection_length;
                        intersection_sphere_number = index;
                    }
                }
            }

            if (is_intersected)
            {
                // The ray hit the sphere
                Vector3d intersection_position = ray_origin + nearest_intersection * ray_direction;

                // Compute normal at the intersection point
                Vector3d ray_normal = (intersection_position - spheres[intersection_sphere_number].head<3>()).normalized();

                // Compute the light vector
                Vector3d ray_light = (light_position - intersection_position).normalized();
                
                // Simple diffuse model
                C(i,j) = ray_light.dot(ray_normal);

                // Clamp to zero
                C(i,j) = max(C(i,j),0.);

                // Disable the alpha mask for this pixel
                A(i,j) = 1;
            }
        }
    }

    // Save to png
    write_matrix_to_png(C,C,C,A,filename);

}

void part1_3_multiple()
{
    std::cout << "Part 1_3_multiple: render part1.2 with perspective projection" << std::endl;

    const std::string filename("part1_3_multiple.png");
    MatrixXd R = MatrixXd::Zero(800,800); // Store the red color
    MatrixXd G = MatrixXd::Zero(800,800); // Store the green color
    MatrixXd B = MatrixXd::Zero(800,800); // Store the blue color
    MatrixXd A = MatrixXd::Zero(800,800); // Store the alpha mask

    // The camera is perspective at (0,0,2), pointing in the direction -z 
    // and covering the unit square (-1,1) in x and y at z = 1
    Vector3d origin(0,0,2);
    Vector3d direction = Vector3d(-1,1,1) - origin;
    Vector3d x_displacement(2.0/R.cols(),0,0);
    Vector3d y_displacement(0,-2.0/R.rows(),0);

    // Two light sources
    const Vector3d light_position_1(-1,1,1);
    const Vector3d light_position_2(1,1,1);

    // Multiple Spheres (x,y,z,r), (R,G,B,M) (M=0 diffuse model, M=1 specular model)
    vector<Vector4d> spheres = {Vector4d(0.3,0.3,0.3,0.3), Vector4d(-0.5,-0.4,-0.7,0.4)};
    vector<Vector4d> spheres_color = {Vector4d(0.3,1.0,0.6,0.), Vector4d(0.3,0.1,0.9,1.)};

    for (unsigned i=0;i<R.cols();i++)
    {
        for (unsigned j=0;j<R.rows();j++)
        {
            // Prepare the ray
            Vector3d ray_origin = origin;
            Vector3d ray_direction = (direction + double(i)*x_displacement + double(j)*y_displacement).normalized();

            // Intersect with the sphere
            bool is_intersected = false;
            int intersection_sphere_number = 0;
            double nearest_intersection = 10;
            for (int index = 0; index < spheres.size(); index++)
            {
                Vector3d origin_to_sphere_center = spheres[index].head<3>() - ray_origin;
                double origin_to_perpendicular = ray_direction.dot(origin_to_sphere_center);
                double perpendicular_height = sqrt(origin_to_sphere_center.dot(origin_to_sphere_center) - origin_to_perpendicular * origin_to_perpendicular);
                if (perpendicular_height <= spheres[index](3))
                {
                    is_intersected = true;
                    double intersection_to_perpendicular = sqrt(spheres[index](3) * spheres[index](3) - perpendicular_height * perpendicular_height);
                    double origin_to_intersection_length = origin_to_perpendicular - intersection_to_perpendicular;
                    if (origin_to_intersection_length < nearest_intersection)
                    {
                        nearest_intersection = origin_to_intersection_length;
                        intersection_sphere_number = index;
                    }
                }
            }

            if (is_intersected)
            {
                // The ray hit the sphere
                Vector3d intersection_position = ray_origin + nearest_intersection * ray_direction;

                // Compute normal at the intersection point
                Vector3d ray_normal = (intersection_position - spheres[intersection_sphere_number].head<3>()).normalized();

                // Compute the light vectors
                Vector3d ray_light_1 = (light_position_1 - intersection_position).normalized();
                Vector3d ray_light_2 = (light_position_2 - intersection_position).normalized();

                // Normalized view vector
                Vector3d view = -ray_direction;

                // Compute normalized half angle vector
                Vector3d half_angle_1 = (view + ray_light_1).normalized();
                Vector3d half_angle_2 = (view + ray_light_2).normalized();

                double lightness = 0;

                if (spheres_color[intersection_sphere_number](3) == 0)
                {
                    // Pure diffuse model
                    double lightness_1 = max(0.,ray_light_1.dot(ray_normal));
                    double lightness_2 = max(0.,ray_light_2.dot(ray_normal));
                    lightness = lightness_1 + lightness_2;
                } else if (spheres_color[intersection_sphere_number](3) == 1)
                {
                    // Specular shading
                    double lightness_1 = max(0.,ray_normal.dot(ray_light_1)) + max(0.,pow(ray_normal.dot(half_angle_1), 100));
                    double lightness_2 = max(0.,ray_normal.dot(ray_light_2)) + max(0.,pow(ray_normal.dot(half_angle_2), 100));
                    // Plus ambient light
                    lightness = lightness_1 + lightness_2 + 0.1;
                }

                R(i,j) = lightness * spheres_color[intersection_sphere_number](0);
                G(i,j) = lightness * spheres_color[intersection_sphere_number](1);
                B(i,j) = lightness * spheres_color[intersection_sphere_number](2);

                // Disable the alpha mask for this pixel
                A(i,j) = 1;
            }
        }
    }

    // Save to png
    write_matrix_to_png(R,G,B,A,filename);

}

void part1_4()
{
    vector<string> off_files_string{"../data/cube.off", "../data/bunny.off", "../data/bumpy_cube.off"};
    vector<MatrixXd> vertices;
    vector<MatrixXi> faces;
    vector<Vector3d> colors{Vector3d(0.3,1.0,0.6), Vector3d(0.3,0.1,0.9)};

    // Read file to Matrix vector
    for (int file_i = 0; file_i < off_files_string.size(); file_i++)
    {
        ifstream off_file(off_files_string[file_i]);
        string line;
        int number_of_vertices = 0;
        int number_of_faces = 0;

        // Ignore the first line
        getline(off_file, line);

        // Read the numbers from the second line
        if (getline(off_file, line))
        {
            istringstream line_stream(line);
            string element;
            int element_index = 0;

            // Handle each element
            while (getline(line_stream, element, ' '))
            {
                if (element_index == 0)
                {
                    number_of_vertices = stoi(element);
                } else if (element_index == 1)
                {
                    number_of_faces = stoi(element);
                }
                element_index++;
            }
        }

        // Declare the vertices and faces Matrices
        MatrixXd vertex = MatrixXd::Zero(number_of_vertices, 3);
        vertices.push_back(vertex);
        MatrixXi face = MatrixXi::Zero(number_of_faces, 3);
        faces.push_back(face);

        // Read the vertices data
        for (int row = 0; row < number_of_vertices; row++)
        {
            if(getline(off_file, line))
            {
                istringstream line_stream(line);
                string element;
                int element_index = 0;
                
                // Handle each element
                while (getline(line_stream, element, ' '))
                {
                    vertices[file_i](row,element_index) = stod(element);
                    element_index++;
                }
            }
        }

        // Read the faces data
        for (int row = 0; row < number_of_faces; row++)
        {
            if(getline(off_file, line))
            {
                istringstream line_stream(line);
                string element;
                int element_index = 0;

                while (getline(line_stream, element, ' '))
                {
                    if (element_index == 0)
                    {
                        element_index++;
                        continue;
                    }
                    faces[file_i](row,element_index - 1) = stoi(element);
                    element_index++;
                }
            }
        }
    }

    // Rescale Picture
    vertices[0] = vertices[0] * 5;
    vertices[1] = vertices[1] / 10;

    // Shading
    const std::string filename("part1_4.png");
    MatrixXd R = MatrixXd::Zero(800,800); // Store the red color
    MatrixXd G = MatrixXd::Zero(800,800); // Store the green color
    MatrixXd B = MatrixXd::Zero(800,800); // Store the blue color
    MatrixXd A = MatrixXd::Zero(800,800); // Store the alpha mask

    // The camera is perspective at (0,0,2), pointing in the direction -z 
    // and covering the unit square (-1,1) in x and y at z = 1
    Vector3d origin(0,0,2);
    Vector3d direction = Vector3d(-1,1,1) - origin;
    Vector3d x_displacement(2.0/R.cols(),0,0);
    Vector3d y_displacement(0,-2.0/R.rows(),0);

    // Two light sources
    const vector<Vector3d> light_positions = {Vector3d(-1,1,1), Vector3d(1,1,1)};

    for (unsigned i=0;i<R.cols();i++)
    {
        for (unsigned j=0;j<R.rows();j++)
        {
            // Prepare the ray
            Vector3d ray_origin = origin;
            Vector3d ray_direction = (direction + double(i)*x_displacement + double(j)*y_displacement).normalized();
            double smallest_t = 100;

            int nearest_file_i = 0;
            int nearest_face_i = 0;
            bool is_intersected = false;

            // Use Cramer’s rule to solve t and get the nearest triangle
            for (int file_i = 0; file_i < off_files_string.size(); file_i++)
            {
                for (int face_i = 0; face_i < faces[file_i].rows(); face_i++)
                {
                    int a_i = faces[file_i](face_i,0);
                    int b_i = faces[file_i](face_i,1);
                    int c_i = faces[file_i](face_i,2);

                    Vector3d a(vertices[file_i](a_i,0), vertices[file_i](a_i,1), vertices[file_i](a_i,2));
                    Vector3d b(vertices[file_i](b_i,0), vertices[file_i](b_i,1), vertices[file_i](b_i,2));
                    Vector3d c(vertices[file_i](c_i,0), vertices[file_i](c_i,1), vertices[file_i](c_i,2));

                    Matrix3d t_numerator, beta_numerator, gamma_numerator;
                    t_numerator << a - b, a - c, a - ray_origin;

                    Matrix3d denominator;
                    denominator << a - b, a - c, ray_direction;

                    double t = t_numerator.determinant() / denominator.determinant();

                    if(t > 0 && t < smallest_t)
                    {
                        gamma_numerator << a - b, a - ray_origin, ray_direction;
                        double gamma = gamma_numerator.determinant() / denominator.determinant();
                        if (gamma >= 0 && gamma <= 1)
                        {
                            beta_numerator << a - ray_origin, a - c, ray_direction;
                            double beta = beta_numerator.determinant() / denominator.determinant();
                            if (beta >= 0 && beta <= 1 - gamma )
                            {
                                is_intersected = true;
                                smallest_t = t;
                                nearest_file_i = file_i;
                                nearest_face_i = face_i;
                            }
                        }
                    }
                }
            }

            if(is_intersected)
            {
                int a_i = faces[nearest_file_i](nearest_face_i,0);
                int b_i = faces[nearest_file_i](nearest_face_i,1);
                int c_i = faces[nearest_file_i](nearest_face_i,2);

                Vector3d a(vertices[nearest_file_i](a_i,0), vertices[nearest_file_i](a_i,1), vertices[nearest_file_i](a_i,2));
                Vector3d b(vertices[nearest_file_i](b_i,0), vertices[nearest_file_i](b_i,1), vertices[nearest_file_i](b_i,2));
                Vector3d c(vertices[nearest_file_i](c_i,0), vertices[nearest_file_i](c_i,1), vertices[nearest_file_i](c_i,2));

                Vector3d intersection_position = ray_origin + smallest_t * ray_direction;
                Vector3d b_a = b - a;
                Vector3d c_b = c - b;
                Vector3d ray_normal = (b_a.cross(c_b)).normalized();
                Vector3d view = -ray_direction;

                // Ambient light
                double lightness = 0;

                for(int light_i = 0; light_i < light_positions.size(); light_i++)
                {
                    Vector3d ray_light = (light_positions[light_i] - intersection_position).normalized();
                    Vector3d half_angle = (view + ray_light).normalized();
                    lightness += max(0.,ray_normal.dot(ray_light)) + max(0.,pow(ray_normal.dot(half_angle), 100));
                }

                R(i,j) = lightness * colors[nearest_file_i](0);
                G(i,j) = lightness * colors[nearest_file_i](1);
                B(i,j) = lightness * colors[nearest_file_i](2);

                // Disable the alpha mask for this pixel
                A(i,j) = 1;

            }
        }
    }

    // Save to png
    write_matrix_to_png(R,G,B,A,filename);

}

int main()
{
    // part1();
    // part2();
    // part1_1();
    // part1_2();
    // part1_3_single();
    // part1_3_multiple();
    part1_4();

    return 0;
}
