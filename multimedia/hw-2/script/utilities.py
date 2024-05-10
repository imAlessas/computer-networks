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
