# Maldev
My repo for my malware development journey.

## Compilation
Since these are basic, beginner projects, any anti-virus is going to pick up on it and shut it down, so we need to make an exclusion. To learn more about how to do this, go [Here](https://support.microsoft.com/en-us/windows/add-an-exclusion-to-windows-security-811816c0-4dfd-af4a-47e4-c301afe13b26)

Afterwards, you will need to install GCC to compile the programs. You can get this tool [Here](https://sourceforge.net/projects/mingw/files/OldFiles/)

Finally, you can go to Command Prompt, travel to the directory with code in it, and type in the following command to compile the code:
`gcc sourcefile_name.c -o outputfile.exe`

Then simply run the code with 
`outputfile.exe`

### Note
All of these use the DLL in the main directory. All projects will have to be modified slightly based on where you put the DLL.

## Disclaimer
All the code here is open-source and free to use at your discretion. Although this should be common sense, do not use the techniques I'm showing against any target without their EXPLICIT permission.

## Inspiration
This repository was inspired by Crow, a Computer Science youtuber who covers mosts of these projects in more detail. Check him out on [Youtube](https://www.youtube.com/@crr0ww) and his [Website](https://www.crow.rip/crows-nest/general/whoami)
