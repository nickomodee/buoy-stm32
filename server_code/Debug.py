import logging
from functools import wraps
import time

LOGGING_LEVEL = logging.DEBUG

def time_and_log(func):
    @wraps(func)
    def wrapper(*args, **kwargs):
        start_time = time.perf_counter()
        result = func(*args, **kwargs)
        end_time = time.perf_counter()
        duration = end_time - start_time
        print(f"Function '{func.__name__}' took {duration:.4f} seconds to execute.")
        return result
    return wrapper