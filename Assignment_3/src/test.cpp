// C++ include
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <sstream>

#include <Eigen/Core>
#include <Eigen/LU>
#include <Eigen/Geometry>

using namespace std;
using namespace Eigen;

void part1_4()
{
    vector<string> off_files_string{"../data/cube.off", "../data/bunny.off", "../data/bumpy_cube.off"};
    vector<MatrixXf> vertices;
    vector<MatrixXi> faces;

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
                }
                else if (element_index == 1)
                {
                    number_of_faces = stoi(element);
                }
                element_index++;
            }
        }

        // Declare the vertices and faces Matrices
        MatrixXf vertex = MatrixXf::Zero(number_of_vertices, 3);
        vertices.push_back(vertex);
        MatrixXi face = MatrixXi::Zero(number_of_faces, 3);
        faces.push_back(face);

        // Read the vertices data
        for (int row = 0; row < number_of_vertices; row++)
        {
            if (getline(off_file, line))
            {
                istringstream line_stream(line);
                string element;
                int element_index = 0;

                // Handle each element
                while (getline(line_stream, element, ' '))
                {
                    vertices[file_i](row, element_index) = stof(element);
                    element_index++;
                }
            }
        }

        // Read the faces data
        for (int row = 0; row < number_of_faces; row++)
        {
            if (getline(off_file, line))
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
                    faces[file_i](row, element_index - 1) = stoi(element);
                    element_index++;
                }
            }
        }
    }

    // Rescale and Reposition Picture
    vertices[1].col(0) = vertices[1].col(0) + 0.02 * VectorXf::Ones(vertices[1].rows());
    vertices[1].col(1) = vertices[1].col(1) - 0.1 * VectorXf::Ones(vertices[1].rows());
    vertices[1] = vertices[1] * 13;
    vertices[2] = vertices[2] / 4.37847;
}

int main(int argc, char const *argv[])
{
    /* code */
    return 0;
}
