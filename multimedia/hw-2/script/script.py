import os
import time
import math
import subprocess
import concurrent.futures
from sklearn.linear_model import LinearRegression

from utilities import *



# setup
SELECTED_SERVER_CITY = "Los Angeles"
INSTANCES = 250
STEP_BETWEEN_LENGTHS = 1
PATH_TO_SCRIPT = os.path.join("multimedia", "hw-2", "script")


# script settings
SHOW_PLOTS = False
SAVE_TO_FILE = True
SAVE_IMAGES = True





def ping_server(server, instances, length):

    filename = f"{LOGS_PATH}{city}-RTT.txt"

    if not length % (STEP_BETWEEN_LENGTHS * 5):
        print(f" Payload length:     {length}", end="\r")

    cmd = f"ping {server} -n {instances} -l {length}"
    
    result = subprocess.run(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    
    if SAVE_TO_FILE:
        with open(f"{filename}", "a") as file:
            file.write(result.stdout)
            file.write("\n\n")


    millisecs_vector = []
    lines = result.stdout.split("\n")
    
    for line in lines:
        if "durata=" in line:
            duration = line.split("durata=")[1].split("ms")[0]
            millisecs_vector.append(int(duration))
    
    return length, millisecs_vector



def get_links_from_tracert(server: str) -> int:

    filename = f"{LOGS_PATH}{city}-links-tracert.txt"

    # get number of links from tracert
    cmd = f"tracert {server}"
    result = subprocess.run(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

    if SAVE_TO_FILE:
        with open(f"{filename}", "a") as file:
            file.write(result.stdout)

    last_link_line = result.stdout.split("\n")[-4]

    return int(last_link_line.split(" ")[1])



def get_links_from_ping(server: str) -> int:

    filename = f"{LOGS_PATH}{city}-links-ping.txt"

    for ttl in range(20, 0, -1):

        cmd = f"ping {server} -i {ttl}"
        result = subprocess.run(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

        if SAVE_TO_FILE:
            with open(f"{filename}", "a") as file:
                file.write(result.stdout)
                file.write("\n\n")

        if "TTL scaduto durante il passaggio" in result.stdout:
            return (ttl + 1)



def plot_all_data() -> None:

    if not SAVE_IMAGES and not SHOW_PLOTS:
        return

    IMGS_PATH = os.path.join(PATH_TO_SCRIPT, "imgs") + "\\"

    # creates directory if does not exist
    if SAVE_IMAGES and not os.path.exists(IMGS_PATH):
        os.makedirs(IMGS_PATH)


    
    ##### all latencies #####

    # calculate the true number of retrived data in the case of some lost packages
    exploit_lengths = []
    latencies = []

    for key, value in stats.items():
        for item in value:
            latencies.append(item)
            exploit_lengths.append(key)


    random_colors = np.random.rand(len(exploit_lengths), 3)

    plt.figure(figsize=(10, 6))
    plt.scatter(exploit_lengths, latencies, edgecolors=random_colors, facecolors="none", s=10)
    plt.xlabel("Packet size - Bytes")
    plt.ylabel("Round-Trip-Time(k) - millisecs")
    plt.title("Total gathered latencies")
    plt.grid(True)

    if SAVE_IMAGES:
        plt.savefig(f"{IMGS_PATH}total-latencies.png")



    ##### max latencies #####
    plt.figure(figsize=(10, 6))
    plt.scatter(payload_lengths, list(max_values.values()), edgecolors="magenta", facecolors="none", s=20)
    plt.xlabel("Packet size - Bytes")
    plt.ylabel("Round-Trip-Time(k) - millisecs")
    plt.title("Maximum latencies")
    plt.grid(True)

    if SAVE_IMAGES:
        plt.savefig(f"{IMGS_PATH}max-latencies.png")



    ##### avg latencies #####
    plt.figure(figsize=(10, 6))
    plt.scatter(payload_lengths, list(average_values.values()), edgecolors="lime", facecolors="none", s=20)
    plt.xlabel("Packet size - Bytes")
    plt.ylabel("Round-Trip-Time(k) - millisecs")
    plt.title("Average latencies")
    plt.grid(True)

    if SAVE_IMAGES:
        plt.savefig(f"{IMGS_PATH}avg-latencies.png")



    ##### standard deviation #####
    plt.figure(figsize=(10, 6))
    plt.scatter(payload_lengths, list(standard_deviations.values()), edgecolors="dodgerblue", facecolors="none", s=20)
    plt.xlabel("Packet size - Bytes")
    plt.ylabel("Round-Trip-Time(k) - millisecs")
    plt.title("Standard deviation")
    plt.grid(True)

    if SAVE_IMAGES:
        plt.savefig(f"{IMGS_PATH}standard-deviation.png")



    ##### min latencies and predictions #####
    
    # generate min and alpha-line
    x_line = np.linspace(min(payload_lengths), max(payload_lengths), 100)
    y_line = alpha * x_line + reg.intercept_

    plt.figure(figsize=(10, 6))
    plt.scatter(payload_lengths, list(min_values.values()), edgecolors="red", facecolors="none", s=20)
    plt.plot(x_line, y_line, color="blue")
    plt.title("Minimum latencies and fitting")
    plt.xlabel("Packet size - Bytes")
    plt.ylabel("Round-Trip-Time(k) - millisecs")
    plt.grid(True)

    if SAVE_IMAGES:
        plt.savefig(f"{IMGS_PATH}min-latencies.png")



    ### displays all plots
    plt.show()







if __name__ == "__main__":
    initial = time.time()


    # Definitions of constants
    SLEEP_TIME = 0.5
    LOGS_PATH = os.path.join(PATH_TO_SCRIPT, "logs") + "\\"
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

    # set choosen options
    server = SERVERS[SELECTED_SERVER_CITY]
    city = server.split(".")[0]
    payload_lengths = range(10, 1471, STEP_BETWEEN_LENGTHS)



    print(f"\nServer:         \033[1m\033[34m{server}\033[0m\n\n")


    # create log dir if it does not exist
    if SAVE_TO_FILE and not os.path.exists(LOGS_PATH):
        os.makedirs(LOGS_PATH)

    # delete log files in the directory
    delete_files_in_directory(LOGS_PATH, city)




    #### count number of links

    print_task(1, number_color="red")
    
    
    # using tracetr
    print(f" Number of links found with \033[1mtracert\033[0m:", end="")
    start = time.time()
   
    tracert_links = get_links_from_tracert(server)
   
    print(f" {tracert_links}")
    print(f"                { round(time.time() - start, 2) } sec")


    # using multiple ping
    start = time.time()
    print(" Number of links found with muliple \033[1mping\033[0m:", end="")

    ping_links = get_links_from_ping(server)
           
    print(f" {ping_links}")
    print(f"                { round(time.time() - start, 2) } sec")




    

    
    ### Round Trip Time
    print_task(2, number_color="red")
    start = time.time()

    stats = {}
    
    # using ThreadPoolExecutor to imporve computational capabilites
    with concurrent.futures.ThreadPoolExecutor(max_workers=100) as executor:
        
        futures = []
        for length in payload_lengths:
            future = executor.submit(ping_server, server, INSTANCES, length)
            futures.append(future)
            time.sleep(SLEEP_TIME)
        
        for future in concurrent.futures.as_completed(futures):
            length, millisecs_vector = future.result()
            stats[length] = millisecs_vector
    
    # order stats by length
    stats = dict(sorted(stats.items()))

    filename = f"{LOGS_PATH}{city}-RTT-total.txt"
    if SAVE_TO_FILE:
        save_dictionary(stats, filename)


    print(f"\n Processed {len(payload_lengths) * INSTANCES} pings")
    print(f"                { round(time.time() - start, 2) } sec")



    start = time.time()

    # compute the min, max and avg of stats
    print(" Computed Min, Max, Avg and StdDev")

    max_values = {}
    min_values = {}
    average_values = {}
    standard_deviations = {}

    for key, value in stats.items():
        max_values[key] = max(value)
        min_values[key] = min(value)
        average_values[key] = sum(value) / len(value)
        standard_deviations[key] = math.sqrt(sum((x - average_values[key]) ** 2 for x in value) / len(value))

    print(f"                { round(time.time() - start, 2) } sec")


    if SAVE_TO_FILE:
        filename = f"{LOGS_PATH}{city}-RTT-min.txt"
        save_dictionary(min_values, filename)

        filename = f"{LOGS_PATH}{city}-RTT-max.txt"
        save_dictionary(max_values, filename)
        
        filename = f"{LOGS_PATH}{city}-RTT-avg.txt"
        save_dictionary(average_values, filename)
        
        filename = f"{LOGS_PATH}{city}-RTT-std.txt"
        save_dictionary(standard_deviations, filename)




    ### alpha-coefficient and throughput
    print_task(3, number_color="red")
    start = time.time()

    # use linear regression to retrive alpha, need to transform list into np.arrays
    reg = LinearRegression().fit(
        np.array(payload_lengths).reshape(-1, 1), # transpose of payload lengths
        np.array(list(min_values.values())) 
    )

    alpha = reg.coef_[0]

    # compute throughput
    throughput_identical_link = 2 * tracert_links / alpha
    throughput_bottleneck = 2 / alpha
    
    print(f" alpha = {round(alpha, 6)}")
    print(f" Throughput with identical links = {round(throughput_identical_link, 3)}")
    print(f" Throughput in a bottleneck scenario = {round(throughput_bottleneck, 3)}")
    print(f"                { round(time.time() - start, 2) } sec")
    


    plot_all_data()


    print(f"\n\n\nTotal execution time:     { round(time.time() - initial, 2) } sec")
    print(f"                          ~ { round((time.time() - initial) / 60, ) } min\n")
