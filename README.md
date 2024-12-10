# Roll-In!
"Roll-In!" is a 3d plateform game using OpenGL and freeglut.
![image](https://github.com/user-attachments/assets/59c07051-51e8-471e-bae3-44067ecc419e)
## How to play the game?
First of all:


```git clone https://github.com/Ouass77ck/Roll-In-```



Then, make sure to have *GLUT* and *SFML* installed.
If you want a background music you can import your own by adding a *music.wav* file to your repository
Now you can either use CodeBlocks and simply build and run the cbp file. Or you can use a linux terminal and run it with the compiler of your choice (I used GCC).

## More:
If you want you can design your own level by adding as many plateforms as you want into the `initObstacles` function:


`Obstacle platform_name = {xfloat, yfloat,zfloat, widthfloat, heightfloat, depthfloat, 2};`


`obstacles.push_back(platform1);`

### Contributors:
Ouass77ck
