# pong
<p>6/6/22 CAS Reflection 2 - pong </p>

<h1> How to download and execute </h1>
<p> <b> This program requires you to be using MSVC (Visual Studio). </b> I use VC2022, but older versions may work. This project uses C++20. <br /> 
  To download, type the following command in any terminal: </p>

```
git clone https://github.com/dec-lied/pong.git
``` 

<p> to download the repository. The main code is in the pong/ directory with the includes being in the include/ directory. You can do any of the following to run the program: </p>
<ol>
  <li> Navigate to the build/ directory and run the executable corresponding to your pc architecture. </li>
  <li> Build it from the source yourself by opening pong.sln and selecting Release as well as x64 or x86 according to your pc architecture then build.
       If you decide to do this, be sure to <b> <ins> make a copy of the scoreImages folder into the same directory as the binary pong.exe </ins> </b> </li>
  <li> Download the release according respective to your architecture from the Releases tab on this page (https://github.com/dec-lied/pong/releases/tag/V1.0). </li>
</ol>

<h1> How to play </h1>
<ul>
  <li> The left paddle uses the W and S keys to move up and down respectively. </li>
  <li> The right paddle uses the up and down arrow keys to move in their respective directions. </li>
  <li> If the ball hits the top inner half of the paddle, it will be sent up. If it hits the bottom inner half of the paddle, it will be sent down. 
    You can use this to mix up your opponent and try to score </li>
  <li> After each point scored, the ball will be sent to the loser of the round in the same direction that the ball was previously traveling in after 2 seconds. </li>
  <li> The first person to score after reaching 5 points wins, and the game will reset. </li>
</ul>

<h1> Screenshots of the game </h1>

![midgame](https://user-images.githubusercontent.com/103293120/172284466-bc9f57a2-86d9-49dd-b0ff-ff622b2cd51c.jpg)
![rwins](https://user-images.githubusercontent.com/103293120/172284474-d513c8ec-1993-46cf-aa40-6bf4fcf70a5b.jpg)
![reset](https://user-images.githubusercontent.com/103293120/172284480-cbba7a13-7955-4720-9d42-2dade1912da5.jpg)
