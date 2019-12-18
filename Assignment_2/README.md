# Assignment 2: Rasterization

&nbsp;

## 1.1 Triangle Soup Editor

In this section, we need to implement three modes to add, edit and delete triangles.

In order to change the mode whenever we press a different key, we need to have a global variable `mode` to indicate the current state of the mode.

### 'i': Triangle Insertion Mode

When we press the 'i' key (GLFW_KEY_I), we change the value of the `mode` to 1.

In order to add triangles, I decided to add one column to the Vertex Matrix `V` and update VBO based on `V` each time when user input a mouse-click. Also, I use a integer to record the current triangle numbers and whenever a group of 3 clicks is done, this integer increase by 1.

Because we have to draw the lines in black and the triangle in red, at first glance I thought we need two fragment shaders. However, when I tried to use two fragment shaders by creating two program instances, things didn't work out and the triangles simply won't be created on the screen.  
Then after reading the example files in the extra folder, I realized that what we need is not two fragment shaders. We can simply use a uniform in the fragment shader to achieve this function, and we just have to change the uniform using `glUniform4f` to set the uniform value whenever is needed.

#### First Click

When the first click is done, we need to record the position of the first vertex into `V`, and also we need to draw a black line between the first vertex and the current position of the cursor. Therefore, we need to track the position of the cursor whenever we move the cursor, that is, we have to use `glfwSetCursorPosCallback`. And we use `glDrawArrays` and `GL_LINES` to draw the line.

Move around the cursor when the first click is done
![first click](build/img/1.1_first_click.png)

#### Second Click

When the second click is done, we need to record the position of the second vertex into `V`, and also we need to draw three lines among the first vertex, second vertex and the current position of the cursor. Therefore, we also have to use `glfwSetCursorPosCallback`. And these three lines are actually a line loop, so we use `glDrawArrays` and `GL_LINE_LOOP` to draw the line loop among these three points.

Move around the cursor when the second click is done
![second click](build/img/1.1_second_click.png)

#### Third Click

When the third click is done, we need to record the position of the third vertex into `V`, and also we need to draw a triangle based on the first vertex, second vertex and the third vertex. Therefore, we use `glDrawArrays` and `GL_TRIANGLES` to draw the triangle among these three points.

When the third click is done and the triangle is created.
![third click](build/img/1.1_third_click.png)

Moreover, because we have to represent all the triangles we have created, we need to use a loop to iterate the `V` to draw all the triangles that we have.

### 'o': Triangle Translation Mode

When we press the 'o' key (GLFW_KEY_O), we change the value of the `mode` to 2.

#### Clicked

Because we can only move a triangle at a time, we need to know which triangle we are selecting at a certain point. In order to achieve that, I set a global variable `selected_triangle` to track the selected triangle.

To determine which triangle we are selecting, we have to check whether the point of the cursor is in a certain triangle. And if our cursor is in several triangles, we choose the latest created triangle.

Here, we use the method similar to detect if a ray is in the triangle to determine whether a point is in the triangle, by solving the linear equations.

When a triangle is selected, the color of the triangle should be changed into blue. To achieve this, we use `glUniform4f`.

When the triangle is clicked (but the button is not released)
![translation click](build/img/1.1_translation_click.png)

#### Dragged

When the triangle is dragged, we have to move the position of the three vertices of the triangle according to the movement of the cursor.

Therefore, we have to record the starting state when the mouse is clicked, calculate the difference between the current position of the cursor and the starting position of the cursor. Then we need to add the difference to each of the vertices of the triangle.

When the triangle is dragged around
![translation move](build/img/1.1_translation_move.png)

#### Released

When the left mouse button is released, the current triangle is no longer selected, and we should set the color of it back to red.

When the triangle is released
![translation release](build/img/1.1_translation_release.png)

### 'p': Triangle Deletion Mode

When we press the 'p' key (GLFW_KEY_p), we change the value of the `mode` to 3.

And when we are in the Triangle Deletion Mode, when we press the left mouse button, the selected triangle is deleted. The way that I implement this is that move all the information after this triangle forward, so the information of current triangle is overridden, which appears that the triangle is deleted.

When the triangle is deleted
![delete](build/img/1.1_delete.png)

&nbsp;

## 1.2 Rotation/Scale

When we press the 'r' key (GLFW_KEY_R), we change the value of the `mode` to 4.

This is the initial state before the triangle is rotated
![initial state](build/img/1.2_rotate_init.png)

### 'h', 'j': Rotation

To achieve rotation, basically we just use a transform matrix to do so. It can be done in one matrix if we use the homogeneous coordinates, or we can do it by first subtracting the position of the barycenter, second rotating using a matrix, third adding the position of the barycenter.

Rotate Clockwise Once
![rotate clockwise once](build/img/1.2_rotate_clock_1.png)

Rotate Clockwise Twice
![rotate clockwise twice](build/img/1.2_rotate_clock_2.png)

Rotate Counter-Clockwise Once
![rotate counter-clockwise once](build/img/1.2_rotate_counter_1.png)

Rotate Counter-Clockwise Twice
![rotate counter-clockwise twice](build/img/1.2_rotate_counter_2.png)

### 'k', 'l': Scale

Same as rotation, it can be done in one matrix if we use the homogeneous coordinates, or we can do it by subtracting and adding the position of the barycenter before and after we use the scaling matrix.

Scale Up 20%
![scale up](build/img/1.2_scale_up.png)

Scale Down 20%
![scale down](build/img/1.2_scale_down.png)

&nbsp;

## 1.3 Colors

The color mode is a little bit different as the modes above. Because before we are always drawing triangles as a whole and one triangle only have one color. But in color mode we have to give every vertex a different color. To do that, I changed both the vertex shader and the fragment shader, and also add another VertexBufferObject `VBO_C` to store the color of each vertex. Therefore, we can deliver the color information from `VBO_C` to vertex shader then to fragment shader. Moreover, we also have to be compatible with the modes we made above, so I use an if statement to check if the corresponding color information of the vertex in `VBO_C` is default (0), then we use the uniform color, if not, we use the color stored in `VBO_C`.

### 'c': Vertex Color Mode

When we press the 'c' key (GLFW_KEY_C), we change the value of the `mode` to 5.

### '1' - '9': Change Vertex Color

Once we find the selected vertex, we just have to use `key_callback` to change the color of the vertex corresponding to the key we pressed, that is, change the value we stored in the `VBO_C`.

When the first selected vertex is changed to color 1
![color 1](build/img/1.3_1.png)

When the second selected vertex is changed to color 2
![color 2](build/img/1.3_2.png)

When the second selected vertex is changed to another color
![color 3](build/img/1.3_3.png)

&nbsp;

## 1.4 View Control

Because the view control should be compatible with the other modes, so we don't have to set a key to enter this mode and we don't have to change the value of `mode`.

Changing the view is not very difficult, we just have to change our vertex shader a little bit so that the rendered position is the world position transformed by `view` matrix (`view` is a uniform). Then we just have to set the uniform `view` at every loop.

Also, we have to change the transform matrix `view` every time we press a corresponding key ('+','-','w','a','s','d').

This is the initial state before changing the view
![view initial state](build/img/1.4_init.png)

### '+': Zoom 20% In

This is how it looks when we press '+' to zoom in
![zoom in](build/img/1.4_zoom_in.png)

### '-': Zoom 20% Out

This is how it looks when we press '-' to zoom out
![zoom out](build/img/1.4_zoom_out.png)

### 'a': Pan the View Right by 20%

This is how it looks when we press 'a' to pan right
![pan right](build/img/1.4_a.png)

### 'w': Pan the View Down by 20%

This is how it looks when we press 'w' to pan down
![pan down](build/img/1.4_w.png)

### 'd': Pan the View Left by 20%

This is how it looks when we press 'd' to pan left (actually twice)
![pan left](build/img/1.4_d.png)

### 's': Pan the View Up by 20%

This is how it looks when we press 's' to pan up (actually twice)
![pan up](build/img/1.4_s.png)

&nbsp;

## 1.5 Add Keyframing

Same as the view control, keyframing should be compatible with the other modes, but I have set a bool variable `is_keyframe` to check if we are in keyframing mode.

Because the video cannot be included in this file, I'll just show the animation result at the demonstration.

### 'f': Keyframing Mode

When we press the 'f' key (GLFW_KEY_F), we change the value of the `is_keyframe` to `true`.

Moreover, after we enter the keyframing mode, we also need to use the cursor to select the triangle we want to keyframing.

### 'z': Start Keyframing

When we are in the keyframing mode (that is `is_keyframe` is `true`), we click 'z' to record the starting keyframe. Specifically, we record the current state of the selected triangle into a Vertex Matrix called `KSP` (Keyframing Starting Point).

### 'x': Ending Keyframing and Start Animation

When we are in the keyframing mode (that is `is_keyframe` is `true`), and we also have recorded the starting keyframe, we click 'x' to record the ending keyframe by record current state of the selected triangle into a Vertex Matrix called `KEP` (Keyframing Ending Point).. For simplicity, just after we record the ending keyframe , we start the animation from the starting keyframe to ending keyframe automatically. We are also using the linear interpolation to draw the frames between the two keyframes.
