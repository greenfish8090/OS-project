import matplotlib.pyplot as plt

n1_rr = []
n2_rr = []
n3_rr = []
alg_rr = []
wt_c1_rr = []
tat_c1_rr = []
wt_c2_rr = []
tat_c2_rr = []
wt_c3_rr = []
tat_c3_rr = []

n1_fcfs = []
n2_fcfs = []
n3_fcfs = []
alg_fcfs = []
wt_c1_fcfs = []
tat_c1_fcfs = []
wt_c2_fcfs = []
tat_c2_fcfs = []
wt_c3_fcfs = []
tat_c3_fcfs = []


with open("profile.txt") as file:
    line_count = 0
    for line in file:
        if line_count>0 :
            k = line.split(', ')[:-1]
            # print(k)
            if line_count%2 :
                n1_rr.append(int(k[0]))
                n2_rr.append(int(k[1]))
                n3_rr.append(int(k[2]))
                alg_rr.append(0)
                wt_c1_rr.append(float(k[4]))
                tat_c1_rr.append(float(k[5]))
                wt_c2_rr.append(float(k[6]))
                tat_c2_rr.append(float(k[7]))
                wt_c3_rr.append(float(k[9]))
                tat_c3_rr.append(float(k[9]))
            
            else:
                n1_fcfs.append(int(k[0]))
                n2_fcfs.append(int(k[1]))
                n3_fcfs.append(int(k[2]))
                alg_fcfs.append(1)
                wt_c1_fcfs.append(float(k[4]))
                tat_c1_fcfs.append(float(k[5]))
                wt_c2_fcfs.append(float(k[6]))
                tat_c2_fcfs.append(float(k[7]))
                wt_c3_fcfs.append(float(k[9]))
                tat_c3_fcfs.append(float(k[9]))
        line_count+=1

#For RR
for t in range(1,4):
    #for turnaround time

    tat_c1_plot = []
    tat_c2_plot = []
    tat_c3_plot = []

    for i in range(len(n1_rr)):
        if (t==1 and n2_rr[i] == 100000 and n3_rr[i] == 100000) or (t==2 and n1_rr[i] == 100000 and n3_rr[i] == 100000) or (t==3 and n1_rr[i] == 100000 and n2_rr[i] == 100000):
            tat_c1_plot.append(tat_c1_rr[i])
            tat_c2_plot.append(tat_c2_rr[i])
            tat_c3_plot.append(tat_c3_rr[i])

    plt.plot([10,100,1000,10000,100000,1000000], tat_c1_plot, color='r', label='c1')
    plt.plot([10,100,1000,10000,100000,1000000], tat_c2_plot, color='g', label='c2')
    plt.plot([10,100,1000,10000,100000,1000000], tat_c3_plot, color='b', label='c3')
    plt.title("Round Robin")
    plt.xlabel("n"+str(t))
    plt.ylabel("TAT")
    plt.legend()
    plt.savefig("tat_n"+str(t)+"_rr")
    plt.clf()

    #for waiting time

    wt_c1_plot = []
    wt_c2_plot = []
    wt_c3_plot = []

    for i in range(len(n1_rr)):
        if (t==1 and n2_rr[i] == 100000 and n3_rr[i] == 100000) or (t==2 and n1_rr[i] == 100000 and n3_rr[i] == 100000) or (t==3 and n1_rr[i] == 100000 and n2_rr[i] == 100000):
            wt_c1_plot.append(wt_c1_rr[i])
            wt_c2_plot.append(wt_c2_rr[i])
            wt_c3_plot.append(wt_c3_rr[i])

    plt.plot([10,100,1000,10000,100000,1000000], wt_c1_plot, color='r', label='c1')
    plt.plot([10,100,1000,10000,100000,1000000], wt_c2_plot, color='g', label='c2')
    plt.plot([10,100,1000,10000,100000,1000000], wt_c3_plot, color='b', label='c3')
    plt.title("Round Robin")
    plt.xlabel("n"+str(t))
    plt.ylabel("WT")
    plt.legend()
    plt.savefig("wt_n"+str(t)+"_rr")
    plt.clf()

#For FCFS
for t in range(1,4):
    #for turnaround time

    tat_c1_plot = []
    tat_c2_plot = []
    tat_c3_plot = []

    for i in range(len(n1_fcfs)):
        if (t==1 and n2_fcfs[i] == 100000 and n3_fcfs[i] == 100000) or (t==2 and n1_fcfs[i] == 100000 and n3_fcfs[i] == 100000) or (t==3 and n1_fcfs[i] == 100000 and n2_fcfs[i] == 100000):
            tat_c1_plot.append(tat_c1_fcfs[i])
            tat_c2_plot.append(tat_c2_fcfs[i])
            tat_c3_plot.append(tat_c3_fcfs[i])

    plt.plot([10,100,1000,10000,100000,1000000], tat_c1_plot, color='r', label='c1')
    plt.plot([10,100,1000,10000,100000,1000000], tat_c2_plot, color='g', label='c2')
    plt.plot([10,100,1000,10000,100000,1000000], tat_c3_plot, color='b', label='c3')
    plt.title("FCFS")
    plt.xlabel("n"+str(t))
    plt.ylabel("TAT")
    # Adding legend, which helps us recognize the curve according to it's color
    plt.legend()
    # To load the display window
    # plt.show()
    plt.savefig("tat_n"+str(t)+"_fcfs")
    plt.clf()

    #for waiting time

    wt_c1_plot = []
    wt_c2_plot = []
    wt_c3_plot = []

    for i in range(len(n1_fcfs)):
        if (t==1 and n2_fcfs[i] == 100000 and n3_fcfs[i] == 100000) or (t==2 and n1_fcfs[i] == 100000 and n3_fcfs[i] == 100000) or (t==3 and n1_fcfs[i] == 100000 and n2_fcfs[i] == 100000):
            wt_c1_plot.append(wt_c1_fcfs[i])
            wt_c2_plot.append(wt_c2_fcfs[i])
            wt_c3_plot.append(wt_c3_fcfs[i])

    plt.plot([10,100,1000,10000,100000,1000000], wt_c1_plot, color='r', label='c1')
    plt.plot([10,100,1000,10000,100000,1000000], wt_c2_plot, color='g', label='c2')
    plt.plot([10,100,1000,10000,100000,1000000], wt_c3_plot, color='b', label='c3')
    plt.title("FCFS")
    plt.xlabel("n"+str(t))
    plt.ylabel("WT")
    plt.legend()
    plt.savefig("wt_n"+str(t)+"_fcfs")
    plt.clf()
