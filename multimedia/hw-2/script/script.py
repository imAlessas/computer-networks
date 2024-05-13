import os
import time
import math
import subprocess
import numpy as np
import concurrent.futures
from sklearn.linear_model import LinearRegression


from utilities import *



def ping_server(server, instances, length):

    print(f" Payload length:  {length}", end="\r")

    cmd = f"ping {server} -n {instances} -l {length}"
    
    result = subprocess.run(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    
    millisecs_vector = []
    lines = result.stdout.split("\n")
    
    for line in lines:
        if "durata=" in line:
            duration = line.split("durata=")[1].split("ms")[0]
            millisecs_vector.append(int(duration))
    
    return length, millisecs_vector






def get_links_from_tracert(server: str, filename: str) -> int:

    # get number of links from tracert
    cmd = f"tracert {server}"
    result = subprocess.run(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

    with open(f"{filename}", "a") as file:
        file.write(result.stdout)

    last_link_line = result.stdout.split("\n")[-4]

    return int(last_link_line.split(" ")[1])















if __name__ == "__main__":
    START = time.time()

    # setup for the script
    SELECTED_SERVER_CITY = "Los Angeles"
    INSTANCES = 100
    STEP = 100
    PAYLOAD_LENGTHS = range(10, 1471, STEP)

    # Definitions of constants
    LOGS_PATH = os.path.join("multimedia", "hw-2", "script", "logs") + "\\"
    SERVERS = {
                "Atlanta" : "atl.speedtest.clouvider.net",
                "New York City" : "nyc.speedtest.clouvider.net", 
                "London" : "lon.speedtest.clouvider.net", 
                "Los Angeles" : "la.speedtest.clouvider.net", 
                "Paris" : "paris.testdebit.info", 
                "Lillie" : "lille.testdebit.info", 
                "Lyon" : "lyon.testdebit.info", 
                "Aix-Marseille" : "aix-marseille.testdebit.info", 
                "Bordeaux" : "bordeaux.testdebit.info"
            }

    # choosen options
    server = SERVERS[SELECTED_SERVER_CITY]
    city = server.split(".")[0]



    print(f"\nServer:         \033[1m\033[34m{server}\033[0m\n\n")



    # delete files in the directory
    delete_files_in_directory(LOGS_PATH, city)



    # count number of links
    print_task(1, number_color="red")
    
    # using tracetr
    start = time.time()

    filename = f"{LOGS_PATH}{city}-links-tracert.txt"
    tracert_links = get_links_from_tracert(server, filename)
   
    print(f" Number of links found with \033[1mtracert\033[0m: {tracert_links}")
    print(f"                { round(time.time() - start, 2) } sec")


    # using multiple ping
    start = time.time()
    filename = f"{LOGS_PATH}{city}-links-ping.txt"

    ping_links = 0
    for ttl in range(20, 0, -1):

        cmd = f"ping {server} -i {ttl}"
        result = subprocess.run(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

        if "TTL scaduto durante il passaggio" in result.stdout:
            ping_links = ttl + 1
            break
           
    print(f" Number of links found with muliple \033[1mping\033[0m: {ping_links}")
    print(f"                { round(time.time() - start, 2) } sec")




    

    
    # minimum, maximum, average RTT
    print_task(2, number_color="red")
    start = time.time()

    stats = {}

    filename = f"{LOGS_PATH}{city}-RTT.txt"

    
    with concurrent.futures.ThreadPoolExecutor(max_workers=100) as executor:
        
        futures = []
        for length in PAYLOAD_LENGTHS:
            future = executor.submit(ping_server, server, INSTANCES, length)
            futures.append(future)
            time.sleep(.75)
        
        for future in concurrent.futures.as_completed(futures):
            length, millisecs_vector = future.result()
            stats[length] = millisecs_vector
    

    save_dictionary(stats, filename)

    print(f"\n Processed {len(PAYLOAD_LENGTHS) * INSTANCES} pings")
    print(f"                { round(time.time() - start, 2) } sec")



    # extract payload lengths and durations
    payload_lengths = list(stats.keys())
    durations = [item for sublist in stats.values() for item in sublist]

    # plot durations
    plot_data(payload_lengths * INSTANCES,
              durations,
              edge="random",
              x_label="L (pkt size) - bytes",
              y_label="RTT(k) - ms")




    start = time.time()

    # compute the min, max and avg of stats
    v_max = {}
    v_min = {}
    v_avg = {}
    v_std = {}

    for key, value in stats.items():
        v_max[key] = max(value)
        v_min[key] = min(value)
        v_avg[key] = sum(value) / len(value)
        v_std[key] = math.sqrt(sum((x - v_avg[key]) ** 2 for x in value) / len(value))

    print(" Computed Min, Max, Avg and StdDev")
    print(f"                { round(time.time() - start, 2) } sec")


    plot_data(payload_lengths,
        list(v_max.values()),
        edge="red",
        x_label="L (pkt size) - bytes",
        y_label="RTT(k) max - ms")
    
    plot_data(payload_lengths,
        list(v_min.values()),
        edge="yellow",
        x_label="L (pkt size) - bytes",
        y_label="RTT(k) min - ms")

    plot_data(payload_lengths,
        list(v_std.values()),
        edge="cyan",
        x_label="L (pkt size) - bytes",
        y_label="RTT(k) std - ms")

    plot_data(payload_lengths,
        list(v_avg.values()),
        edge="red",
        x_label="L (pkt size) - bytes",
        y_label="RTT(k) avg - ms")






    # alpha-coefficient and throughput
    print_task(3, number_color="red")
    start = time.time()

    min_values = np.array([[v] for v in list(v_min.values())])
    reg = LinearRegression().fit(min_values , np.array(PAYLOAD_LENGTHS))

    alpha = reg.coef_
    throughput_identical_link = 2 * tracert_links / alpha
    throughput_bottleneck = 2 / alpha
    
    print(f"alpha = {alpha}")
    print(f"Throoughput with identical links = {throughput_identical_link}")
    print(f"Throughpunt in a bottleneck scenario = {throughput_bottleneck}")
    print(f"                { round(time.time() - start, 2) } sec")
    
    



    
    plt.show()

    print(f"\n\n\nTotal execution time:     { round(time.time() - START, 2) } sec")
