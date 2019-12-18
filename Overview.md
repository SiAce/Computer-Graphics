# Texture Mapping

This project is mainly related to texture mapping that we learned in class.

Implements 3 types of texture mapping (Bump, Normal, Parallax) and compare the differences between them.

## 1.1 Bump Mapping

Instead of encoding colors in a texture, we encode the height normals of a scence into a greyscale image to simulate the true shading.

## 1.2 Normal Mapping

Very similar to Bump Mapping, but instead of encoding normals into a greyscale image, we encode normals into a RGB image (which respectively represents the x, y, z coordinates of the normals)

## 1.3 Parallax mapping

Parrallax mapping is an approximation of displacement mapping.

In displacement mapping, instead of encoding normals into an image, we encode displacements into an image. Instead of simulating (cheating) the shading, it is used to cause an effect where the actual geometric position of points over the textured surface are displaced.

Parrallax mapping is implemented by displacing the texture coordinates at a point on the rendered polygon by a function of the view angle in tangent space
