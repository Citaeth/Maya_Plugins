This repo contain maya node created in C++, to demonstrate my evolution with this language. 

The CacheSequencer in a plugin that I created for a Sequence Context in Layout production. 
I had in mind to switch automaticly from an usd cache to an other, depending on where we are in the timeline (when we switch from a shot to an other. 
At the creation of the node, we could add new entry for each new usd cache, defining it's start and end frame depending on the length of the shot, and the Usd Cache Start Frame, definied in the USD file itself.
We should have in output the Offset of time applied on the UsdProxyShape, and the path for the cache to used.

The CameraSwitcher one was a bit more tricky to do and for a more specific situation. 
The node was designed to work in a Stitch Camera Tool context. It's when you have differents cameras in a sequence, and want to do like a previz of the whole sequence, by retiming easly all of them in the same time and the same scene.
In the same time of the node, we want to create a new camera rig, that will be constraint by all of the cameras in the sequence. In the plugins, we have each camera, with their name, and a Start/End Frame, and the Time attribut.
The Start/End attribut is drives by a User UI, who could choose at what moment a camera drive or not the new camera rig. The goal is at the end, to have only one camera, with the plate/scene from each other cameras, like a whole view of sequence, and have a preview
of what we should change/retime in the shots scenes to match at the stitch camera.
The output of the node is the influence of each camera, between 0 and 1.
When in the UI two or more camera timeline are overlapping, or if we decide to space out between two cameras, then the plugin should create an inbetween, depending on which camera is the more closer to the current position. 
For exemple, if the camera 1 is from Frame 1 to Frame 10, camera 2 is from Frame 20 to Frame 30, and I'm currently at frame 12, then my camera rig will be impacted at 80% by camera 1 and 20% by camera 2.
This was designed in a first step for only the transform, but can work if needed for every attribut of the camera (focal length...)
We can think about improvement, right now the inbetween from two cameras are only calculated in linear. Also, we can imagine to complexify the tool to have different supporting ways for the plates, depending of if we want it to be
retimed when we stretch/extend the shots time, or if we just want to cut in it. In the end, like it's a tool who only generate a camera and a view of the scene/plate. The only goal to let the artist "play" in the scene and rework the sequence without thinking
of the pipeline/shot aspect, before going into shot scene to match to the new desired sequence. It could be great to add a way in output to get the new timing of the camera, and the "mode" choosen for the plate (cut or retime), to automoatize also the "technical part" 
and only let to the artist the joy to play with his cameras and focusing on creating the most georgous sequence ever! 
