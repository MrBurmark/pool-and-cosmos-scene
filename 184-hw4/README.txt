readme file for homework4
scene name: pool table room
Jason Burmark 184-bj
Tianyu Shao 184-ei
link to web:http://inst.cs.berkeley.edu/~cs184-bj/
1.complete scene
    We drew a room with window, painting, a person, Pool cues, and a pool table. There are balls on the table and if we look out of the window, we can actually see some stars 
  moving.
2.hand create object
   We made the pool table by hand, including 60 vertices and 21 faces, and also the pool cue.
3.load obj file
   We downloaded a humanoid obj file from a  http://people.sc.fsu.edu/~jburkardt/data/obj/obj.html and put it in the scene.
4.placement and scaling by hand
   We did that in the .txt file and we read the txt file into the program
5.texture 
   We textured the floor, and the painting/sign
6.shiny and dull
   The balls are shiny and the walls as well as the lights are dull.
7.directional light
   We put the directional lights on the wall which make the wall and the room lighter.
8.point light
   We put point lights in the lights above the table and the sun.
9.instantiate more than once
   The balls are instantiated more than once, as is the pool cue, and the textured box.
10.shading
   The humanoid and the cues are the only things not shaded correctly as the objs had no normals (normals were calculated, but it still doesn't look right)
11.double buffering
   It's used as a default in the display
12.turn on/off texturing
   We have a variable to control this
13.fragment shader
   We extended the fragment shader to use textures and raytrace shadows for the balls from the three lights above the table
14.mouse and keyboard control
   We implemented controls using both the mouse and keyboard, movement is done by holding down a key or dragging the mouse, 
     standard fps controls with movement up and down, as well as tilting and hitting balls with the mouse buttons.
15.move around
   We were able to move around the scene with the controls explained above.
16.constantly animated
   The balls and the stars outside the window are constantly animated. Also, we can use the mouse to click on the object and they will move and 
   collide with each other.
extra credit
1.ray-tracing
   We used raytracing to implement the shadows, hit the balls, and something similar to bounce the balls off the walls.