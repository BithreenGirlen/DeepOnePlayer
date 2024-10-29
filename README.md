# DeepOnePlayer
A certain player intended to be used with [DeepOne](https://github.com/BithreenGirlen/DeepOne).
- Backend
  - Microsoft Media Foundation
  - Windows Imaging Component (WIC)
  - Direct2D
  - DirectWrite

## How to play
Select a folder such as the following from menu item `Folder->Open`.
<pre>
10345304
├ 101.jpg
├ 102.jpg
├ ...
├ 10345304.txt
├ adu103453_01_01.mp3
├ adu103453_01_02.mp3
└ ...
</pre>
<pre>
10201005
├ 102010_01.mp4
├ 102010_02.mp4
├ ...
├ 10201005.txt
├ adu102010_02_01.mp3
├ adu102010_02_02.mp3
└ ...
</pre>

## Mouse function

| Input | Function |
----|---- 
Mouse wheel | Scale up/down
Left button click | Step on to the next image/video.
Left button drag | Move view; valid only when scaled size is beyond display resolution.
Middle button |Reset scale to default.
Right button + mouse wheel | Play the next/previous audio file.
Right button + middle button | Hide/show window's frame. Having hidden, the window goes to the origin of the primary display.
Right button + left button | Move window; valid only when the window's frame is hidden.

## Keyboard function

| Input | Function |
----|---- 
Esc| Close the application.
| Up | Open the next folder. |
| Down | Open the previous folder. |
| C | Switch text colour between black and white. |
| T | Hide/show text. |

## Menu function
| Entry | Item | Fucntion |
----|---- |---- 
Folder| Open | Open folder-select-dialogue.
Audio| Loop | Set/reset audio loop setting.
-| Setting | Open a dialogue for audio volume and playback-rate setting.
Video| Pause | Pause video.
-| Setting | Open a dialogue for video playback-rate setting.
