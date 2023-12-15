# Zybo-Z7-Diaries
Repo where I will store my Zybo / Z7 files and document my journey with learning about FPGAs


## To start, I wanted to get into FPGAs, but first I need a board.

Looking at the 2 leaders in the space, I have a choice between Xilinx and Altera supported boards. With that I chose Xilinx since that is what I am learning in school and am more comfortable with.

Now I have my 2 requirements:
1. beginner-friendly board
2. supported by Xilinx

In the end I chose the Zybo Z7 since it has PMODS (which allow for easier interfacing with premade connections, helpful for a beginnner who just wants less things to worry about)

## Now its time to get the software.

Looking at Digilent's documentation, PMODs are only supported till Vivado 2019.1. However, the labs at school use 2021.1, which Im assuming a student verified that it works with PMODS. Therefore, I installed Vivado 2021.1 but I will update my findings here.

NOTE: I tried to get the smallest install package so I chose a random engineering sample board during install (the installer forces you to install a device package). However, what I didnt know was that I need to install a Xilinx chip family that I will be using. In the case of the Zybo Z7, that is the Zynq 7000 SoC. Dont be like me and try to get a smaller install pacakage, only to find out this doesnt work and you have to sit through hours of install again.

For those wondering, it is about 30 GBs

Now just follow the Digilent Docs to add the board to Vivado and it should show up as a board option when creating a Vivado project

## Now its time to create projects!

I will start by following the official Zynq tutorial book and follow other guides that I find online 
