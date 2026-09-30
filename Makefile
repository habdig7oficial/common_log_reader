# Load file as separate streams
#make compile && ./executable ./inputs/log-streams/*_stream-*.txt


#Tiny file
#make compile && ./executable ./inputs/access_log_tiny/*_stream-*.txt
#make compile && ./executable ./inputs/access_log_tiny/access_log_tiny.txt

compile: 
	gcc main.c -o executable