# gyro_functions
## About
This folder contains implementations of the gyroscope.
The simple folder contains a simple, easy to use and understand
version of gyroscope functions. The advanced folder contains a
threaded version that utilizes C++.
## How the Gyroscope Works
### Understanding the Axes.
The gyroscope works by outputting values (negative or positive)
when it senses rotation along an axis. Assuming the wombat is parallel
with the ground (as the code in the repo does), then 'left' and 'right'
rotation will be along the z axis. This is why the code uses `gyro_z()`.
If, for example, we oriented the wombat some other way, then the code
might use `gyro_x()` or `gyro_y()` to get the rotation along those axes.
The following picture helps to show where the axes are and should help
gain an understanding of which axes to use when writing your own code.

![](https://mathinsight.org/media/applet/image/large/cartesian_coordinate_axes_3d.png)

### Understanding the values
A negative value means that the gyroscope senses counterclockwise
rotation along an axis, while a positive value means that the gyroscope
senses clockwise rotation along an axis. The larger the absolute value
of the number is, the faster the rotation is.