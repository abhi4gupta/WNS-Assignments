files
1. third.cc   //code
2. cnwdvstime.png   // graph for Congestion Window
3. qsizevstime.png   //graph for queueSize
4. lossratevstime.png  // graph for lossRate
5. recieveratevstime.png  // graph for recieveratevstime
6. thrptvstime1.plt     //data for first link throughputvstime
7. thrptvstime2.plt     //data for second link throughputvstime
8. thrptvstime1.png     //graph for first link throughputvstime
9. thrptvstime2.png     //graph for second link throughputvstime
10. report_Assignment3.pdf  // Question Answer and explained report of Assignment


Execute the script using the following command
./waf --run scratch/third
Start gnuplot by typing the following command on terminal
graph plot :-
gnuplot
reset
set term pngcairo
set output "graphname.png"
set title "title"
set xlabel "x axis label"set ylabel "y axis label"