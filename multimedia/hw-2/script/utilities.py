import os


def print_task(n: int, task_color: str = "white", number_color: str = "white") -> None:
    
    color: dict = {
    "black": "\033[30m",
    "red": "\033[31m",
    "green": "\033[32m",
    "orange": "\033[33m",
    "blue": "\033[34m",
    "purple": "\033[35m",
    "cyan": "\033[36m",
    "white": "\033[37m"
    }

    print(f"\n{color[task_color]}Task {color[number_color]}{n}\033[0m")



def delete_files_in_directory(dir_path: str, prefix: str = "") -> None:
    """
    Function to delete files in a directory that start with a specified prefix.

    Parameters:
    dir_path (str): The path to the directory containing the files to be deleted.
    prefix (str): The prefix that the files must start with to be deleted. Default is an empty string.

    Returns:
    None
    """

    # iterate through the files in the directory
    for filename in os.listdir(dir_path):
        # check if the filename starts with the prefix
        if filename.startswith(prefix):
            file_path = os.path.join(dir_path, filename)
            
            if os.path.isfile(file_path):
                # remove the file
                os.remove(file_path)
                print(f"Deleted file: \033[33m{filename}\033[0m")



def delete_file_content(filename: str) -> None:
    """
    Function to delete the content of a file by opening it in write mode, effectively emptying the file.

    Parameters:
    filename (str): The name of the file to be emptied.

    Returns:
    None
    """
        
    with open(f"{filename}", "w"):
        pass 
