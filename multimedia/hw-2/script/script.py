import os
import pprint
import numpy as np
import matplotlib.pyplot as plt


from utilities import *





def get_links_from_tracert(filename : str) -> int:

    # get number of links from tracert
    cmd = f"tracert {server} > {filename}"
    os.system(cmd)

    with open(f"{filename}", "r") as file:
        last_router_line = file.readlines()[-3]
        return int(last_router_line.split(" ")[1])






if __name__ == "__main__":

    # setup for the script
    SELECTED_SERVER_CITY = "Lyon"
    INSTANCES = 100
    PAYLOAD_LENGHTS = range(10, 1470, 75)

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


    # delete files in the directory
    delete_files_in_directory(LOGS_PATH, city)



    # count number of links
    print_task(1, number_color="red")

    filename = f"{LOGS_PATH}{city}-links.txt"
    tracert_links = get_links_from_tracert(filename)
    print(f"Number of links found with \033[1mtracert\033[0m: {tracert_links}")


    
    ping_links = 0
    for ttl in range(20, 0, -1):

        cmd = f"ping {server} -i {ttl} > {filename}"
        os.system(cmd)

        with open(f"{filename}", "r") as file:
           if "TTL scaduto durante il passaggio" in file.read():
               ping_links = ttl + 1
               break
           
    print(f"Number of links found with muliple \033[1mping\033[0m: {ping_links}")



    

    
    # minimum, maximum, average RTT
    print_task(2, number_color="red")

    stats = {}

    filename = f"{LOGS_PATH}{city}-RTT.txt"

    for lenght in PAYLOAD_LENGHTS:
        print(f"  Payload length :     {lenght}")

        millisecs_vector = []

        cmd = f"ping {server} -n {INSTANCES} > {filename}"
        os.system(cmd)
        
        with open(f"{filename}", "r") as file:
            lines = file.readlines()

            for line in lines:
                if "durata=" in line:
                    duration = line.split("durata=")[1].split("ms")[0]
                    millisecs_vector.append(int(duration))

        stats[lenght] = millisecs_vector


    pprint.pprint(stats)





    print_task(3, number_color="red")

    # Extract payload lengths and durations
    payload_lengths = list(stats.keys())
    durations = [item for sublist in stats.values() for item in sublist]

    # Create random edge colors for each point
    edge_colors = np.random.rand(len(payload_lengths) * INSTANCES, 3)

    # Create the scatter plot with random edge colors and no fill color
    plt.scatter(payload_lengths * INSTANCES, durations, edgecolors=edge_colors, facecolors='none')

    # Set the labels and title
    plt.xlabel('L (pkt size) - bits')
    plt.ylabel('RTT(k) - ms')

    # Show grid
    plt.grid(True)

    # Display the plot
    plt.show()
